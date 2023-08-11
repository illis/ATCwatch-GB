const std = @import("std");
const testing = std.testing;

const c = @cImport({
    @cInclude("string.h");
    @cInclude("stdlib.h");
    @cInclude("atcrust_c.h");
});
//const c = @cImport({
//    @cInclude("TimeLib.h");
//});

export fn add(a: i32, b: i32) i32 {
    return a + b;
}

fn find_char_idx(ch: u8, s: [*c]u8, start_idx: u8) u8 {
    var idx = start_idx;
    while (true) {
        if (s[idx] == '\\') {
            idx += 1;
        } else if (s[idx] == ch) {
            break;
        }

        idx += 1;
    }

    return idx;
}

const StartEnd = struct {
    start: u8,
    end: u8,
};

fn to_s(s: [*c]u8, se: *StartEnd) []u8 {
    return s[se.start..se.end];
}

const GBCallIndexes = struct {
    t: StartEnd,
    cmd: StartEnd,
    name: StartEnd,
    number: StartEnd,
};

const FindWorker = struct {
    next_start_idx: u8,
};

const JT = enum {
    String,
    Integer,
};

fn find_next_index(w: *FindWorker, s: [*c]u8, label: [:0]const u8, ty: JT, se: *StartEnd) void {
    se.start = @intCast(w.next_start_idx + label.len + 2); // + 2 for pre ',' and trailing ':'

    switch (ty) {
        JT.String => {
            se.start += 1; // +1 for '"'
            se.end = find_char_idx('\"', s, se.start);
            w.next_start_idx = se.end + 1; // + 1 for trailing '"'
        },
        JT.Integer => {
            se.end = find_char_idx(',', s, se.start);
            w.next_start_idx = se.end;
        },
    }
}

const GBNotifyIndexes = struct {
    t: StartEnd,
    id: StartEnd,
    src: StartEnd,
    title: StartEnd,
    subject: StartEnd,
    body: StartEnd,
    sender: StartEnd,
};

fn findGBNotifyIndexes(s: [*c]u8) GBNotifyIndexes {
    var idxs: GBNotifyIndexes = undefined;
    var w = FindWorker{
        .next_start_idx = 4,
    };

    find_next_index(&w, s, "t", JT.String, &idxs.t);
    find_next_index(&w, s, "id", JT.Integer, &idxs.id);
    find_next_index(&w, s, "src", JT.String, &idxs.src);
    find_next_index(&w, s, "title", JT.String, &idxs.title);
    find_next_index(&w, s, "subject", JT.String, &idxs.subject);
    find_next_index(&w, s, "body", JT.String, &idxs.body);
    find_next_index(&w, s, "sender", JT.String, &idxs.sender);

    return idxs;
}

fn findGBCallIndexes(s: [*c]u8) GBCallIndexes {
    var idxs: GBCallIndexes = undefined;
    var w = FindWorker{
        .next_start_idx = 4,
    };

    find_next_index(&w, s, "t", JT.String, &idxs.t);
    find_next_index(&w, s, "cmd", JT.String, &idxs.cmd);
    find_next_index(&w, s, "name", JT.String, &idxs.name);
    find_next_index(&w, s, "number", JT.String, &idxs.number);

    return idxs;
}

const set_time_callback_op = *const fn (epoch: c_long) callconv(.C) void;
const show_notf_callback_op = *const fn (msg: [*]const u8) callconv(.C) void;

// export fn process_bangle_input(s: [*c]u8, len: u8, set_time_cb: set_time_callback_op, show_notf_cb: show_notf_callback_op) void {
export fn process_bangle_input(s: [*c]u8, len: u8, set_time_cb: set_time_callback_op, show_notf_cb: show_notf_callback_op, notf_data: [*c]c.NotfData) void {
    const settime_check_str = "\x10setTime(";
    const notify_set_check_str = "\x10GB({t:\"notify";
    const call_set_check_str = "\x10GB({t:\"call";

    if (c.strncmp(settime_check_str, s, settime_check_str.len) == 0) {
        // setTime(1692075012);E.setTimeZone(12.0);(s=>s&&(s.timezone=12.0,require('Storage').write('setting.json',s)))(require('Storage').readJSON('setting.json',1))
        var b: [15:0]u8 = undefined;
        var offset: usize = settime_check_str.len;
        for (s[offset..@as(usize, len)], 0..) |epoch_char, i| {
            if (epoch_char == ')') {
                offset += i;
                break;
            }
            b[i] = epoch_char;
        }
        var epoch = c.atoi(&b);

        offset += ");E.setTimeZone(".len;
        for (s[offset..@as(usize, len)], 0..) |epoch_char, i| {
            if (epoch_char == ')') {
                b[i] = 0;
                break;
            }
            b[i] = epoch_char;
        }

        // hrs * 60min * 60sec
        epoch += @intFromFloat(c.strtof(&b, null) * 60 * 60);

        set_time_cb(epoch);

        if (false) {
            var msg: [17:0]u8 = undefined;
            std.mem.copy(u8, &msg, "\x03\x01\x0azigdtestxxxxz\x00");
            //msg[2 + 3 + 1 + 5] = s[0];
            //msg[2 + 3 + 1 + 5 + 1] = s[1];

            // var si = to_string_int(s[0]);
            // msg[2 + 3 + 1 + 5 + 2] = si[0];
            // msg[2 + 3 + 1 + 5 + 3] = si[1];

            var si = to_string_int(@intCast(notf_data.*.notf_count));
            // msg[2 + 3 + 1 + 5] = @intCast(notf_data.*.notf_count);
            msg[2 + 3 + 1 + 5 + 0] = si[0];
            msg[2 + 3 + 1 + 5 + 1] = si[1];
            msg[2 + 3 + 1 + 5 + 2] = notf_data.*.notfs[0].title[0];
            // msg[2 + 3 + 1 + 5 + 2] = si[0];
            // msg[2 + 3 + 1 + 5 + 3] = si[1];

            show_notf_cb(&msg);
        }
    } else if (c.strncmp(notify_set_check_str, s, notify_set_check_str.len) == 0) {
        var idxs = findGBNotifyIndexes(s);
        _ = setNotfData_se(notf_data, s, &idxs.sender, &idxs.subject, &idxs.body);
    } else if (c.strncmp(call_set_check_str, s, notify_set_check_str.len) == 0) {
        var idxs = findGBCallIndexes(s);
        _ = setNotfData_se(notf_data, s, &idxs.t, &idxs.t, &idxs.t);
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
    }
    // var buff: [4096:0]u8 = undefined;

    // var i: usize = 0;
    // while (i < 4096) {
    //     buff[i + 3] = s[i];
    //     if (s[i] == 0) {
    //         break;
    //     }
    // }

    // not send this - seems to break something
    // show_notf_cb(&buff);
}

fn copy_notf_field(f: []u8, f_len: u8, n: []u8, n_len: u8) void {
    var max = f_len;
    if (n_len < max) max = n_len;

    c.memcpy(f, n, max);
}

fn setNotfData(data: *c.NotfData, app_name: []u8, title: []u8, body: []u8) usize {
    if (data.*.notf_count == c.NOTF_MAX) {
        // we need to shuffle idx's
        data.*.notf_count = 0;
        while (data.*.notf_count < (c.NOTF_MAX - 1)) {
            data.*.notfs[data.*.notf_count] = data.*.notfs[data.*.notf_count + 1];
            data.*.notf_count += 1;
        }
    }

    // TODO; check if the -1 is necessary
    // minus one to leave room for null
    var app_name_len = @min(c.NOTF_APPNAME_LIMIT - 1, app_name.len);
    var title_len = @min(c.NOTF_TITLE_LIMIT - 1, title.len);
    var body_len = @min(c.NOTF_BODY_LIMIT - 1, body.len);

    @memcpy(data.*.notfs[data.notf_count].app_name[0..app_name_len], app_name);
    @memcpy(data.*.notfs[data.notf_count].title[0..title_len], title);
    @memcpy(data.*.notfs[data.notf_count].body[0..body_len], body);

    // set end char to null
    data.*.notfs[data.notf_count].app_name[app_name.len] = 0;
    data.*.notfs[data.notf_count].title[title.len] = 0;
    data.*.notfs[data.notf_count].body[body.len] = 0;

    data.notf_count += 1;
    return data.notf_count;
}

fn setNotfData_se(data: *c.NotfData, s: [*c]u8, app_se: *StartEnd, title_se: *StartEnd, body_se: *StartEnd) usize {
    return setNotfData(data, to_s(s, app_se), to_s(s, title_se), to_s(s, body_se));
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

    var idxs = findGBNotifyIndexes(s);

    try testing.expectEqualStrings("Gadgetbridge", s[idxs.src.start..idxs.src.end]);
    try testing.expectEqualStrings("", s[idxs.title.start..idxs.title.end]);
    try testing.expectEqualStrings("Test\\\"", s[idxs.subject.start..idxs.subject.end]);
}

test "check GB call" {
    var s_original = "\x10GB({t:\"call\",cmd:\"incoming\",name:\"Testname\",number:\"Testnum\"})";
    var s: [*c]u8 = @ptrCast(@constCast(s_original));

    var idxs = findGBCallIndexes(s);

    try testing.expectEqualStrings("call", s[idxs.t.start..idxs.t.end]);
    try testing.expectEqualStrings("incoming", s[idxs.cmd.start..idxs.cmd.end]);
    try testing.expectEqualStrings("Testname", s[idxs.name.start..idxs.name.end]);
    try testing.expectEqualStrings("Testnum", s[idxs.number.start..idxs.number.end]);
}

test "basic add functionality" {
    try testing.expect(add(3, 7) == 10);
}
