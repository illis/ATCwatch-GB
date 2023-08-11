extern crate cbindgen;

use std::env;
use std::path::PathBuf;

fn main() {
    let dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").expect("err getting dir"));

    // cpp version, needed for arduino code to reference
    let mut config_cxx = cbindgen::Config::from_file("./cbindgen.toml").unwrap();
    config_cxx.language = cbindgen::Language::Cxx;
    cbindgen::generate_with_config(&dir, config_cxx)
        .expect("err during generate")
        .write_to_file(dir.join("./arduino_lib/src/atcrust.h"));

    // c version, needed for zig code to reference
    let mut config_c = cbindgen::Config::from_file("./cbindgen.toml").unwrap();
    config_c.language = cbindgen::Language::C;
    cbindgen::generate_with_config(&dir, config_c)
        .expect("err during generate")
        .write_to_file(dir.join("./arduino_lib/src/atcrust_c.h"));
}
