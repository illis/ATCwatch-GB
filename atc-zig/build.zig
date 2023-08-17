const std = @import("std");

// Although this function looks imperative, note that its job is to
// declaratively construct a build graph that will be executed by an external
// runner.
pub fn build(b: *std.Build) void {
    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});

    // Standard optimization options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall. Here we do not
    // set a preferred release mode, allowing the user to decide how to optimize.
    const optimize = b.standardOptimizeOption(.{});

    const lib = b.addStaticLibrary(.{
        .name = "atczig",
        // In this case the main source file is merely a path, however, in more
        // complicated build scripts, this could be a generated file.
        .root_source_file = .{ .path = "src/main.zig" },
        .target = .{
            .cpu_arch = std.Target.Cpu.Arch.arm,
            .cpu_model = std.zig.CrossTarget.CpuModel{ .explicit = &std.Target.arm.cpu.cortex_m4 },
            .cpu_features_add = std.Target.arm.cpu.cortex_m4.features,
            .os_tag = std.Target.Os.Tag.freestanding,
            .abi = std.Target.Abi.gnueabihf,
        },
        .optimize = std.builtin.OptimizeMode.ReleaseSmall,
    });

    lib.addSystemIncludePath(.{ .cwd_relative = "../.arduino/data/packages/nRF52D6Fitness/tools/gcc-arm-none-eabi/5_2-2015q4/arm-none-eabi/include/" });
    lib.addIncludePath(.{ .cwd_relative = "../libraries/atc-rust/src/" });

    // This declares intent for the library to be installed into the standard
    // location when the user invokes the "install" step (the default step when
    // running `zig build`).
    b.installArtifact(lib);

    // Creates a step for unit testing. This only builds the test executable
    // but does not run it.
    const main_tests = b.addTest(.{
        .root_source_file = .{ .path = "src/main.zig" },
        .target = target,
        .optimize = optimize,
    });

    main_tests.linkLibC();
    main_tests.addIncludePath(.{ .cwd_relative = "../libraries/atc-rust/src/" });

    const run_main_tests = b.addRunArtifact(main_tests);

    // This creates a build step. It will be visible in the `zig build --help` menu,
    // and can be selected like this: `zig build test`
    // This will evaluate the `test` step rather than the default, which is "install".
    const test_step = b.step("test", "Run library tests");
    test_step.dependOn(&run_main_tests.step);
}
