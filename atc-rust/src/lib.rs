#![no_std]
// #![feature(core_intrinsics)]
// 
 #[cfg(not(test))]
// use core::{intrinsics, panic::PanicInfo};
use core::{panic::PanicInfo};

#[cfg(not(test))]
#[panic_handler]
#[no_mangle]
pub fn panic(_info: &PanicInfo) -> ! {
    // intrinsics::abort()
    loop {}
}
// #[cfg(not(test))]
// extern crate panic_halt;
// // use panic_halt as _;

use core::cmp::min;
use core::slice::from_raw_parts;

// Taken from android app: app/src/main/java/nodomain/freeyourgadget/gadgetbridge/service/devices/banglejs/BangleJSDeviceSupport.java
// Guessed from how long a notification could potentially be
// TODO: check this is ok
pub const ANDROID_MAX_MSG_LENGTH: usize = 1024;
pub const NOTF_MAX: usize = 5;
pub const NOTF_APPNAME_LIMIT: usize = 64;
pub const NOTF_TITLE_LIMIT: usize = 64;
pub const NOTF_BODY_LIMIT: usize = 256;

#[derive(Clone, Copy)]
#[repr(C)]
pub struct Notf {
    app_name: [u8; NOTF_APPNAME_LIMIT],
    title: [u8; NOTF_TITLE_LIMIT],
    body: [u8; NOTF_BODY_LIMIT],
}

impl Default for Notf {
    fn default() -> Self {
        Notf {
            app_name: [0; 64],
            title: [0; 64],
            body: [0; 256],
        }
    }
}

#[derive(Clone, Copy, Default)]
#[repr(C)]
pub struct NotfData {
    notfs: [Notf; NOTF_MAX],
    notf_count: usize,
}

fn set_limited_chars<'a>(src: &[u8], dst: &'a mut [u8], dst_len: usize) -> &'a [u8] {
    let mut limit = min(dst_len - 1, src.len());
    dst[limit] = b'\0';
    while limit > 0 {
        limit -= 1;
        dst[limit] = src[limit];
    }

    dst
}

#[no_mangle]
pub extern "C" fn read_notf_string<'a>(notf: &'a mut Notf, s: *const u8) -> &'a Notf {
    unsafe {
        let app_name_length = s.add(0).read() as u8 as usize;
        let title_length = s.add(1).read() as u8 as usize;
        let body_length = s.add(2).read() as u8 as usize;

        let mut i = 3;

        let name = from_raw_parts(s.add(i), app_name_length);
        set_limited_chars(name, &mut notf.app_name, NOTF_APPNAME_LIMIT);
        i += app_name_length;

        let title = from_raw_parts(s.add(i), title_length);
        set_limited_chars(title, &mut notf.title, NOTF_TITLE_LIMIT);
        i += title_length;

        let body = from_raw_parts(s.add(i), body_length);
        set_limited_chars(body, &mut notf.body, NOTF_BODY_LIMIT);
    }

    notf
}

#[no_mangle]
pub extern "C" fn read_notification(
    notfs: &mut [Notf; NOTF_MAX],
    notf_count: &mut usize,
    new_notf_string: *const u8,
) -> usize {
    if *notf_count == NOTF_MAX {
        // we need to shuffle idx's
        *notf_count = 0;
        while *notf_count < (NOTF_MAX - 1) {
            notfs[*notf_count] = notfs[*notf_count + 1];
            *notf_count += 1;
        }
    }

    read_notf_string(&mut notfs[*notf_count as usize], new_notf_string);
    *notf_count += 1;
    *notf_count
}

#[no_mangle]
pub extern "C" fn read_notification_using_struct(
    data: &mut NotfData,
    new_notf_string: *const u8,
) -> usize {
    return read_notification(&mut data.notfs, &mut data.notf_count, new_notf_string);
}

#[cfg(test)]
#[macro_use]
extern crate std;
#[cfg(test)]
mod tests {
    use crate::*;
    use std::vec::Vec;

    fn gen_test_notf_string_with_attribs(attribs: [&[u8]; 3]) -> Vec<u8> {
        let mut stringsizes = vec![];
        for a in attribs {
            stringsizes.push(a.len() as u8);
        }

        for a in attribs {
            for b in a {
                stringsizes.push(*b);
            }
        }

        stringsizes
    }

    fn gen_test_notf_string() -> Vec<u8> {
        let attribs: [&[u8]; 3] = ["com.fsck.k9", "title", "body"].map(|s| s.as_bytes());
        gen_test_notf_string_with_attribs(attribs)
    }

    fn gen_test_notf_string_u8_with_attribs<'a>(
        dst: &'a mut [u8],
        attribs: [&[u8]; 3],
    ) -> &'a [u8] {
        let mut i = 0;
        for c in gen_test_notf_string_with_attribs(attribs) {
            dst[i] = c;
            i = i + 1;
        }

        dst
    }

    fn gen_test_notf_string_u8<'a>(dst: &'a mut [u8]) -> &'a [u8] {
        let mut i = 0;
        for c in gen_test_notf_string() {
            dst[i] = c;
            i = i + 1;
        }

        dst
    }

    fn gen_notf_with_attribs(attribs: [&[u8]; 3]) -> Notf {
        let mut buff = [0; 512];
        let notf_string = gen_test_notf_string_u8_with_attribs(&mut buff, attribs);
        let mut notf = Notf::default();

        read_notf_string(&mut notf, notf_string.as_ptr());
        notf
    }

    #[test]
    fn it_gens_a_test_notifcation() {
        let mut buff = [0; 512];
        let notf_string = gen_test_notf_string_u8(&mut buff);

        assert_eq!(notf_string[0], 11);
        assert_eq!(notf_string[1], 5);
        assert_eq!(notf_string[2], 4);

        assert_eq!(notf_string[23], '\0' as u8); // ensure string is nul terminated
    }

    #[test]
    fn it_limits_a_char_array() {
        let char_arr = b"12345";

        let mut dst: [u8; 6] = [1; 6];
        assert_eq!(set_limited_chars(char_arr, &mut dst, 6), b"12345\0");

        let mut dst: [u8; 9] = [1; 9];
        assert_eq!(
            set_limited_chars(char_arr, &mut dst, 9),
            b"12345\0\x01\x01\x01"
        );

        let mut dst: [u8; 4] = [1; 4];
        assert_eq!(set_limited_chars(char_arr, &mut dst, 4), b"123\0");

        //overload last, we dont care if we dont clear the whole buffer
        let char_arr = b"1";
        assert_eq!(set_limited_chars(char_arr, &mut dst, 4), b"1\03\0");
    }

    #[test]
    fn it_generates_a_empty_notf() {
        let notf = Notf::default();
        assert_eq!(notf.app_name, [0; 64]);
        assert_eq!(notf.title, [0; 64]);
        assert_eq!(notf.body, [0; 256]);
    }

    fn partial_cmp_u8(a: &[u8], b: &[u8]) -> bool {
        let (smaller, _bigger) = if a.len() > b.len() { (a, b) } else { (b, a) };

        for i in 0..smaller.len() {
            if a[i] == 0 && b[i] == 0 {
                return true;
            } else if a[i] != b[i] {
                return false;
            }
        }

        true
    }

    #[test]
    fn it_can_partial_eq_u8_str() {
        let a = b"123\0";

        assert_eq!(partial_cmp_u8(a, b"123\0"), true);
        assert_eq!(partial_cmp_u8(a, b"1234\0"), false);
        assert_eq!(partial_cmp_u8(a, b"12\0"), false);
        assert_eq!(partial_cmp_u8(a, b"12\0\0"), false);

        let b = b"1\03\0";
        assert_eq!(partial_cmp_u8(b, b"1\0\0\0"), true);
    }

    #[test]
    fn it_parses_a_notification_string() {
        let mut buff = [0; 512];
        let notf_string = gen_test_notf_string_u8(&mut buff);
        let mut notf = Notf::default();
        let notf = read_notf_string(&mut notf, notf_string.as_ptr());

        assert!(partial_cmp_u8(&notf.app_name, b"com.fsck.k9\0"));
        assert!(partial_cmp_u8(&notf.title, b"title\0"));
        assert!(partial_cmp_u8(&notf.body, b"body\0"));
    }

    #[test]
    fn it_adds_a_notification() {
        let mut notf_count: usize = 0;
        let mut notfs: [Notf; NOTF_MAX] = Default::default();

        let mut buff = [0; 512];
        let notf_string =
            gen_test_notf_string_u8_with_attribs(&mut buff, [b"app0\0", b"title0\0", b"body0\0"]);

        assert_eq!(notf_count, 0);
        let _i = read_notification(&mut notfs, &mut notf_count, notf_string.as_ptr());
        assert_eq!(notf_count, 1);
        assert!(partial_cmp_u8(&notfs[0].app_name, b"app0\0"));
        assert!(partial_cmp_u8(&notfs[0].title, b"title0\0"));
        assert!(partial_cmp_u8(&notfs[0].body, b"body0\0"));

        let notf_string =
            gen_test_notf_string_u8_with_attribs(&mut buff, [b"app1\0", b"title1\0", b"body1\0"]);
        let _i = read_notification(&mut notfs, &mut notf_count, notf_string.as_ptr());
        assert_eq!(notf_count, 2);
        assert!(partial_cmp_u8(&notfs[0].app_name, b"app0\0"));
        assert!(partial_cmp_u8(&notfs[0].title, b"title0\0"));
        assert!(partial_cmp_u8(&notfs[0].body, b"body0\0"));
        assert!(partial_cmp_u8(&notfs[1].app_name, b"app1\0"));
        assert!(partial_cmp_u8(&notfs[1].title, b"title1\0"));
        assert!(partial_cmp_u8(&notfs[1].body, b"body1\0"));

        // fill the rest of the array
        while notf_count < NOTF_MAX {
            let pre_count = notf_count;
            let notf_string = gen_test_notf_string_u8_with_attribs(
                &mut buff,
                [b"app1\0", b"title1\0", b"body1\0"],
            );
            let _i = read_notification(&mut notfs, &mut notf_count, notf_string.as_ptr());
            assert_eq!(notf_count, pre_count + 1);
            assert!(partial_cmp_u8(&notfs[pre_count].app_name, b"app1\0"));
            assert!(partial_cmp_u8(&notfs[pre_count].title, b"title1\0"));
            assert!(partial_cmp_u8(&notfs[pre_count].body, b"body1\0"));
        }

        assert_eq!(notf_count, NOTF_MAX);

        // add another, should delete the oldest index (0) and shuffle them.
        let notf_string =
            gen_test_notf_string_u8_with_attribs(&mut buff, [b"app6\0", b"title6\0", b"body6\0"]);
        let _i = read_notification(&mut notfs, &mut notf_count, notf_string.as_ptr());
        assert_eq!(notf_count, NOTF_MAX);
        assert!(partial_cmp_u8(&notfs[0].app_name, b"app1\0"));
        assert!(partial_cmp_u8(&notfs[0].title, b"title1\0"));
        assert!(partial_cmp_u8(&notfs[0].body, b"body1\0"));
        assert!(partial_cmp_u8(&notfs[NOTF_MAX - 1].app_name, b"app6\0"));
        assert!(partial_cmp_u8(&notfs[NOTF_MAX - 1].title, b"title6\0"));
        assert!(partial_cmp_u8(&notfs[NOTF_MAX - 1].body, b"body6\0"));
    }
}
