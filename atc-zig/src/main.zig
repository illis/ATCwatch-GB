const std = @import("std");
const testing = std.testing;

const c = @cImport({
    @cInclude("string.h");
    @cInclude("stdlib.h");
    @cInclude("atcrust_c.h");
});

const StartEnd = struct {
    start: u8 = 0,
    end: u8 = 0,
};

fn to_s(s: [*c]u8, se: *const StartEnd) []u8 {
    return s[se.start..se.end];
}

const JT = enum {
    String,
    Integer,
};

const Keys = enum {
    t,
    id,
    cmd,
    incoming,
    name,
    src,
    title,
    body,
    sender,
    number,
    subject,
};

const KVIndexes = struct {
    key: ?Keys = undefined,
    val_ty: JT = undefined,
    val_se: StartEnd = StartEnd{},
};

fn find_next_kp(s: [*c]u8, start_idx: u8) KVIndexes {
    var kp: KVIndexes = KVIndexes{};
    var key_se = StartEnd{};

    key_se.start = start_idx;
    key_se.end = key_se.start + 1;
    while (s[key_se.end] != ':') {
        key_se.end += 1;
    }

    // translate to enum
    kp.key = std.meta.stringToEnum(Keys, s[key_se.start..key_se.end]);

    kp.val_ty = switch (s[key_se.end + 1] == '"') {
        true => blk: {
            kp.val_se.start = key_se.end + 2;
            break :blk JT.String;
        },
        false => blk: {
            kp.val_se.start = key_se.end + 1;
            break :blk JT.Integer;
        },
    };

    kp.val_se.end = kp.val_se.start;
    switch (kp.val_ty) {
        JT.Integer => {
            while ((s[kp.val_se.end] >= '0') and (s[kp.val_se.end] <= '9')) {
                kp.val_se.end += 1;
            }
        },
        JT.String => {
            while (s[kp.val_se.end] != '"') {
                if (s[kp.val_se.end] == '\\') {
                    kp.val_se.end += 1;
                }

                kp.val_se.end += 1;
            }
        },
    }

    return kp;
}

fn find_kps(s: [*c]u8, start_idx: u8) std.EnumMap(Keys, StartEnd) {
    var map = std.EnumMap(Keys, StartEnd){};
    var i: u8 = start_idx;

    std.debug.assert(s[i] == '{');
    i += 1;
    while (true) {
        var kp = find_next_kp(s, i);
        if (kp.key) |k| { // only append known keys
            map.put(k, kp.val_se);
        }

        i = kp.val_se.end;
        if (kp.val_ty == JT.String) i += 1; // skip over the end quote
        if (s[i] == '}') break;
        std.debug.assert(s[i] == ',');
        i += 1;
    }

    return map;
}

fn check_src(comptime s1: [*c]const u8, s2: [*c]const u8, se2: StartEnd) bool {
    comptime var s1len = std.zig.c_builtins.__builtin_strlen(s1);
    if (s1len != (se2.end - se2.start)) return false;

    var result = c.strncmp(s1, s2 + se2.start, s1len);
    return result == 0;
}

fn copy_to_buffer_se(s: [*c]const u8, se: StartEnd, buffer: [*c]u8, buffer_len: u8, i: u8) u8 {
    var se_len = se.end - se.start;
    var maxlen = @min(se_len, buffer_len - i);
    if (maxlen > 0) {
        _ = c.memcpy(buffer + i, s + se.start, maxlen);
        return maxlen;
    } else {
        return 0;
    }
    _ = c.memcpy(buffer + i, s + se.start, maxlen);
    return se_len;
}

fn copy_to_buffer(comptime s: [*c]const u8, buffer: [*c]u8, buffer_len: u8, i: u8) u8 {
    comptime var strlen = std.zig.c_builtins.__builtin_strlen(s);
    var se = StartEnd{ .start = 0, .end = strlen };
    return copy_to_buffer_se(@as([*c]const u8, s), se, buffer, buffer_len, i);
}

fn copy_to_buffer_null(buffer: [*c]u8, buffer_len: u8, i: u8) u8 {
    if (i < buffer_len) {
        buffer[i] = 0;
        return 1;
    } else {
        buffer[i - 1] = 0;
        return 0;
    }
}

const tx_callback_op = *const fn (msg: [*]const u8, len: c_uint) callconv(.C) void;
const set_time_callback_op = *const fn (epoch: c_long) callconv(.C) void;
const show_notf_callback_op = *const fn (msg: [*c]const u8) callconv(.C) void;
const wakeup_callback_op = *const fn () callconv(.C) void;

// this also needs to be duplicated in atczig.h
const BLERxData = extern struct {
    buffer_pos: u8,
    buffer: [*c]u8,
    short_msg_buffer_len: u8,
    short_msg_buffer: [*c]u8,
    notfs: *c.NotfData,
    set_time_cb: set_time_callback_op,
    wakeup_cb: wakeup_callback_op,
};

const cmd_settime_check_str = "\x10setTime(";
const cmd_notify_set_check_str = "\x10GB({t:\"notify\"";
const cmd_call_set_check_str = "\x10GB({t:\"call\"";

fn process_set_time(rxData: *BLERxData) void {
    // setTime(1692075012);E.setTimeZone(12.0);(s=>s&&(s.timezone=12.0,require('Storage').write('setting.json',s)))(require('Storage').readJSON('setting.json',1))
    var b: [15:0]u8 = undefined;
    var offset: usize = cmd_settime_check_str.len;
    for (rxData.buffer[offset..@as(usize, rxData.buffer_pos)], 0..) |epoch_char, i| {
        if (epoch_char == ')') {
            offset += i;
            break;
        }
        b[i] = epoch_char;
    }
    var epoch = c.atoi(&b);

    offset += ");E.setTimeZone(".len;
    for (rxData.buffer[offset..@as(usize, rxData.buffer_pos)], 0..) |epoch_char, i| {
        if (epoch_char == ')') {
            b[i] = 0;
            break;
        }
        b[i] = epoch_char;
    }

    // hrs * 60min * 60sec
    epoch += @intFromFloat(c.strtof(&b, null) * 60 * 60);

    rxData.set_time_cb(epoch);
}

fn process_notification(rxData: *BLERxData) void {
    var kps = find_kps(rxData.buffer, 4);

    var i: u8 = 0;
    if (check_src("K-9 Mail", rxData.buffer, kps.get(Keys.src).?)) {
        i += copy_to_buffer("e: ", rxData.short_msg_buffer, rxData.short_msg_buffer_len, i);
        i += copy_to_buffer_se(rxData.buffer, kps.get(Keys.title).?, rxData.short_msg_buffer, rxData.short_msg_buffer_len, i);
        i += copy_to_buffer_null(rxData.short_msg_buffer, rxData.short_msg_buffer_len, i);

        _ = setNotfData_se(rxData.notfs, rxData.buffer, &kps.get(Keys.src).?, &kps.get(Keys.title).?, &kps.get(Keys.body).?);
    } else {
        i += copy_to_buffer("n: ", rxData.short_msg_buffer, rxData.short_msg_buffer_len, i);
        i += copy_to_buffer_se(rxData.buffer, kps.get(Keys.src).?, rxData.short_msg_buffer, rxData.short_msg_buffer_len, i);
        i += copy_to_buffer_null(rxData.short_msg_buffer, rxData.short_msg_buffer_len, i);

        _ = setNotfData_se(rxData.notfs, rxData.buffer, &kps.get(Keys.src).?, &kps.get(Keys.title).?, &kps.get(Keys.body).?);
    }
}

fn process_call(rxData: *BLERxData) void {
    var kps = find_kps(rxData.buffer, 4);

    var i: u8 = 0;
    i += copy_to_buffer("c: ", rxData.short_msg_buffer, rxData.short_msg_buffer_len, i);
    i += copy_to_buffer_se(rxData.buffer, kps.get(Keys.name).?, rxData.short_msg_buffer, rxData.short_msg_buffer_len, i);
    i += copy_to_buffer_null(rxData.short_msg_buffer, rxData.short_msg_buffer_len, i);

    _ = setNotfData_se(rxData.notfs, rxData.buffer, &kps.get(Keys.t).?, &kps.get(Keys.name).?, &kps.get(Keys.number).?);
}

fn handle_input_msg(rxData: *BLERxData) void {
    if (c.strncmp(cmd_settime_check_str, rxData.buffer, cmd_settime_check_str.len) == 0) {
        process_set_time(rxData);
    } else if (c.strncmp(cmd_notify_set_check_str, rxData.buffer, cmd_notify_set_check_str.len) == 0) {
        process_notification(rxData);
        rxData.wakeup_cb();
    } else if (c.strncmp(cmd_call_set_check_str, rxData.buffer, cmd_call_set_check_str.len) == 0) {
        process_call(rxData);
        rxData.wakeup_cb();
    }
}

export fn handle_ble_rx(rxData: *BLERxData, input_val: [*c]u8, input_val_len: u8) void {
    _ = c.memcpy(&rxData.buffer[rxData.buffer_pos], input_val, input_val_len);
    rxData.buffer_pos += input_val_len;

    if ((rxData.buffer[rxData.buffer_pos - 1]) == '\n') {
        handle_input_msg(rxData);
        rxData.buffer_pos = 0;
    }
}

export fn process_bangle_input(s: [*c]u8, len: u8, tx_cb: tx_callback_op, set_time_cb: set_time_callback_op, show_notf_cb: show_notf_callback_op, notf_data: [*c]c.NotfData, short_buffer: [*c]u8, short_buffer_len: u8) void {
    _ = short_buffer_len;
    _ = short_buffer;
    _ = len;
    const is_gps_active_check_str_gps_active_check_str = "\x10GB({t:\"is_gps_active\"})";

    if (c.strncmp(is_gps_active_check_str_gps_active_check_str, s, is_gps_active_check_str_gps_active_check_str.len) == 0) {
        var msg = "({t:\"gps_power\",status:true";
        tx_cb(msg, msg.len);
    } else if (false) {
        set_time_cb(10 * 365 * 24 * 60 * 60); // add ten years from epoch
        var msg: [17:0]u8 = undefined;
        std.mem.copy(u8, &msg, "\x03\x01\x0azigdtestxxxxz\x00");
        //msg[2 + 3 + 1 + 5] = s[0];
        //msg[2 + 3 + 1 + 5 + 1] = s[1];

        // var si = to_string_int(s[0]);
        // msg[2 + 3 + 1 + 5 + 2] = si[0];
        // msg[2 + 3 + 1 + 5 + 3] = si[1];

        show_notf_cb(&msg);
    } else {
        _ = setNotfData(notf_data, s[0..16], s[16..32], s[32..48]);
    }
}

fn setNotfData(data: *c.NotfData, app_name: []u8, title: []u8, body: []u8) usize {
    if (data.notf_count == c.NOTF_MAX) {
        // we need to shuffle idx's
        data.notf_count = 0;
        while (data.notf_count < (c.NOTF_MAX - 1)) {
            data.notfs[data.notf_count] = data.notfs[data.notf_count + 1];
            data.notf_count += 1;
        }
    }

    // TODO; check if the -1 is necessary
    // minus one to leave room for null
    var app_name_len = @min(c.NOTF_APPNAME_LIMIT - 1, app_name.len);
    var title_len = @min(c.NOTF_TITLE_LIMIT - 1, title.len);
    var body_len = @min(c.NOTF_BODY_LIMIT - 1, body.len);

    @memcpy(data.notfs[data.notf_count].app_name[0..app_name_len], app_name);
    @memcpy(data.notfs[data.notf_count].title[0..title_len], title);
    @memcpy(data.notfs[data.notf_count].body[0..body_len], body);

    // set end char to null
    data.notfs[data.notf_count].app_name[app_name.len] = 0;
    data.notfs[data.notf_count].title[title.len] = 0;
    data.notfs[data.notf_count].body[body.len] = 0;

    data.notf_count += 1;
    return data.notf_count;
}

fn setNotfData_se(data: *c.NotfData, s: [*c]u8, app_se: *const StartEnd, title_se: *const StartEnd, body_se: *const StartEnd) usize {
    return setNotfData(data, to_s(s, app_se), to_s(s, title_se), to_s(s, body_se));
}

export fn to_string_int_c(i: u8) callconv(.C) *const u8 {
    var out: [2]u8 = undefined;
    var fba = std.heap.FixedBufferAllocator.init(&out);

    std.mem.copy(u8, &out, "00");
    _ = std.fmt.allocPrint(fba.allocator(), "{d}", .{i}) catch unreachable;

    return @ptrCast(&out);
}

fn to_string_int(i: u8) [2]u8 {
    var out: [2]u8 = undefined;
    var fba = std.heap.FixedBufferAllocator.init(&out);

    std.mem.copy(u8, &out, "00");
    _ = std.fmt.allocPrint(fba.allocator(), "{d}", .{i}) catch unreachable;

    return out;
}

test "u8 to string int" {
    try testing.expectEqualStrings("65", &to_string_int('A'));
}

test "check GB notify" {
    // UART TX: GB({t:\"notify\",id:1692075767,src:\"Gadgetbridge\",title:\"\",subject:\"Test\\"\",body:\"Test\",sender:\"Test\",tel:\"Test\"})
    var s_original = "\x10GB({t:\"notify\",id:1692075767,src:\"Gadgetbridge\",title:\"\",subject:\"Test\\\"\",body:\"Test\",sender:\"Test\",tel:\"Test\"})";
    var s: [*c]u8 = @ptrCast(@constCast(s_original));

    var idxs = find_kps(s, 4);

    try testing.expectEqualStrings("notify", s[idxs.get(Keys.t).?.start..idxs.get(Keys.t).?.end]);
    try testing.expectEqualStrings("Gadgetbridge", s[idxs.get(Keys.src).?.start..idxs.get(Keys.src).?.end]);
    try testing.expectEqualStrings("", s[idxs.get(Keys.title).?.start..idxs.get(Keys.title).?.end]);
    try testing.expectEqualStrings("Test\\\"", s[idxs.get(Keys.subject).?.start..idxs.get(Keys.subject).?.end]);
}

test "check GB notify sms" {
    // UART TX GB({t:"notify",id:1692076320,title:"",subject:"",body:"Hi! Your Skinny balance is\n$8.93 credit\n1946 NZ and AU Rollover Mins\nUnlimited Skinny Mins\nUnlimited NZ and AU Texts\n13902.76 MB Rollover Data\nFor your account info, text INFO to 2424.",sender:"2424",tel:"2424"})
    var s_original = "\x10GB({t:\"notify\",id:1692076320,title:\"\",subject:\"\",body:\"Hi! \n How are you.\",sender:\"2424\",tel:\"2424\"})";
    var s: [*c]u8 = @ptrCast(@constCast(s_original));

    var idxs = find_kps(s, 4);

    try testing.expect(idxs.contains(Keys.t));
    try testing.expectEqualStrings("notify", s[idxs.get(Keys.t).?.start..idxs.get(Keys.t).?.end]);
    try testing.expectEqualStrings("1692076320", s[idxs.get(Keys.id).?.start..idxs.get(Keys.id).?.end]);
    try testing.expectEqualStrings("", s[idxs.get(Keys.title).?.start..idxs.get(Keys.title).?.end]);
    try testing.expectEqualStrings("2424", s[idxs.get(Keys.sender).?.start..idxs.get(Keys.sender).?.end]);

    try testing.expect(!idxs.contains(Keys.name));
}

test "check GB call" {
    var s_original = "\x10GB({t:\"call\",cmd:\"incoming\",name:\"Testname\",number:\"Testnum\"})";
    var s: [*c]u8 = @ptrCast(@constCast(s_original));

    var idxs = find_kps(s, 4);

    try testing.expectEqualStrings("call", s[idxs.get(Keys.t).?.start..idxs.get(Keys.t).?.end]);
    try testing.expectEqualStrings("incoming", s[idxs.get(Keys.cmd).?.start..idxs.get(Keys.cmd).?.end]);
    try testing.expectEqualStrings("Testname", s[idxs.get(Keys.name).?.start..idxs.get(Keys.name).?.end]);
    try testing.expectEqualStrings("Testnum", s[idxs.get(Keys.number).?.start..idxs.get(Keys.number).?.end]);
}

test "check find next kp w/ string" {
    var s_original = "t:\"c\\\"all\"";
    var s: [*c]u8 = @ptrCast(@constCast(s_original));

    var idxs = find_next_kp(s, 0);
    try testing.expectEqual(Keys.t, idxs.key.?);
    try testing.expectEqualStrings("c\\\"all", s[idxs.val_se.start..idxs.val_se.end]);
}

test "check find next kp w/ int" {
    var s_original = "t:1234";
    var s: [*c]u8 = @ptrCast(@constCast(s_original));

    var idxs = find_next_kp(s, 0);
    try testing.expectEqual(Keys.t, idxs.key.?);
    try testing.expectEqualStrings("1234", s[idxs.val_se.start..idxs.val_se.end]);
}

test "check find kps" {
    var s_original = "\x10GB({t:\"call\",cmd:\"incoming\"})";
    var s: [*c]u8 = @ptrCast(@constCast(s_original));

    var idxs = find_kps(s, 4);

    try testing.expect(idxs.contains(Keys.t));
    try testing.expectEqualStrings("call", s[idxs.get(Keys.t).?.start..idxs.get(Keys.t).?.end]);
    try testing.expect(idxs.contains(Keys.cmd));
    try testing.expectEqualStrings("incoming", s[idxs.get(Keys.cmd).?.start..idxs.get(Keys.cmd).?.end]);

    try testing.expect(!idxs.contains(Keys.name));
}

test "copy to buffer test" {
    var s_original = "\x10GB({t:\"call\",cmd:\"incoming\"})";
    var s: [*c]u8 = @ptrCast(@constCast(s_original));
    var kps = find_kps(s, 4);

    const b_len: u8 = 30;
    var b: [b_len]u8 = undefined;
    var i: u8 = 0;
    i += copy_to_buffer("n: ", &b, b_len, i);
    i += copy_to_buffer_se(s, kps.get(Keys.cmd) orelse undefined, &b, b_len, i);
    i += copy_to_buffer_null(&b, b_len, i);

    try testing.expectEqualStrings("n: incoming\x00", b[0..i]);
}
