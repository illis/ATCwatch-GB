from invoke import task
import os
import fileinput
import re

@task
def init(c):
    c.run("arduino-cli core update-index")
    c.run("arduino-cli core install nRF52D6Fitness:nRF5")

    arduino_data_dir = os.environ['ARDUINO_DIRECTORIES_DATA']
    platform_txt_uri = arduino_data_dir + "/packages/nRF52D6Fitness/hardware/nRF5/0.7.5/platform.txt"
    platform_txt_old_uri = platform_txt_uri + ".bck"
    libraries_dir = "libraries"

    print("--- downloading extra libs ---")
    with c.cd(libraries_dir):
        if not os.path.isdir(os.path.join(libraries_dir, 'lv_arduino')):
            c.run('git clone -b 2.1.5 https://github.com/lvgl/lv_arduino')
            print("--- patching lv_arduino ---")
            c.run('git -C lv_arduino apply ../lv_arduino-patch-0001.patch')

        if not os.path.isdir(os.path.join(libraries_dir, 'HRS3300-Arduino-Library')):
            c.run('git clone https://github.com/atc1441/HRS3300-Arduino-Library')

    print("--- patching platform.txt ---")
    if os.path.isfile(platform_txt_old_uri):
        c.run('cp %s %s' % (platform_txt_old_uri, platform_txt_uri))
    else:
        c.run('cp %s %s' % (platform_txt_uri, platform_txt_old_uri))

    with fileinput.FileInput(platform_txt_uri, inplace=True) as file:
        libraries_dir = "ATCwatch\/libraries"
        extra_ld_flags = '-Wl,--allow-multiple-definition' # see: https://github.com/rust-lang/compiler-builtins/issues/353
        for l in ['atc-rust', 'atc-zig', 'HRS3300-Arduino-Library']:
            extra_ld_flags += ' -L ' + os.path.join(c.cwd, '%(base_dir)s/%(name)s/src/cortex-m4' % {'base_dir': 'ATCwatch/libraries', 'name': l})
        regobjs = [
            (re.compile(r'(compiler.path=).*'), r'\1'),
            (re.compile(r'(recipe.c.combine.pattern=.*)'), r'\1 -W -lheart -latcrust -latczig'),
            (re.compile(r'(compiler.ldflags=.*)'), r"\1 %(ld_flags)s" % {'ld_flags': extra_ld_flags}),
        ]

        for line in file:
            found = False
            for ro in regobjs:
                match = ro[0].match(line)
                if match:
                    found = True
                    print(ro[0].sub(ro[1], line), end='')
                    break
            if not found: print(line, end='')

    print("--- init complete ---")


@task
def bear(c):
    sysroot = c.run('arm-none-eabi-g++ --print-sysroot').stdout.rstrip()
    extra_include_paths = [
        os.path.join(c.cwd, 'libraries/atc-rust/src/'),
        os.path.join(c.cwd, 'libraries/atc-zig/src/'),
        os.path.join(c.cwd, 'libraries/lv_arduino/'),
        os.path.join(c.cwd, 'libraries/lv_arduino/src/'),
        os.path.join(c.cwd, 'libraries/HRS3300-Arduino-Library/src/'),
        os.path.join(c.cwd, '.arduino/data/packages/nRF52D6Fitness/hardware/nRF5/0.7.5/libraries/BLEPeripheral/src/'),
        os.path.join(c.cwd, '.arduino/data/packages/nRF52D6Fitness/hardware/nRF5/0.7.5/libraries/Time/'),
        os.path.join(c.cwd, '.arduino/data/packages/nRF52D6Fitness/hardware/nRF5/0.7.5/cores/nRF5/'),
        os.path.join(c.cwd, '.arduino/data/packages/nRF52D6Fitness/hardware/nRF5/0.7.5/cores/nRF5/SDK/components/drivers_nrf/delay'),
        os.path.join(c.cwd, '.arduino/data/packages/nRF52D6Fitness/hardware/nRF5/0.7.5/cores/nRF5/SDK/components/device/'),
        os.path.join(c.cwd, '.arduino/data/packages/nRF52D6Fitness/hardware/nRF5/0.7.5/cores/nRF5/SDK/components/toolchain/CMSIS/Include/'),
        os.path.join(c.cwd, '.arduino/data/packages/nRF52D6Fitness/hardware/nRF5/0.7.5/cores/nRF5/SDK/components/toolchain/'),
        os.path.join(c.cwd, '.arduino/data/packages/nRF52D6Fitness/hardware/nRF5/0.7.5/variants/DSD6/'),
        os.path.join(sysroot, 'include/'),
        os.path.join(sysroot, 'include/c++/12.2.1/'),
        os.path.join(sysroot, 'include/c++/12.2.1/arm-none-eabi/thumb/v7e-m+fp/hard/bits/'),
        # os.path.join(c.cwd, '.arduino/data/packages/nRF52D6Fitness/tools/gcc-arm-none-eabi/5_2-2015q4/arm-none-eabi/include/c++/5.2.1/arm-none-eabi/armv7e-m/fpu/bits/'),
    ]

    # ripped from output of arduino-cil -v compile ...
    compile_cmd = 'arm-none-eabi-g++ -mcpu=cortex-m4 -mthumb -c -g -Os -w -mfloat-abi=hard -mfpu=fpv4-sp-d16 -DCONFIG_NFCT_PINS_AS_GPIOS -DUSE_LFRC -DNRF52 -DS132 -DNRF51_S132 -std=gnu++11 -ffunction-sections -fdata-sections -fno-threadsafe-statics -nostdlib --param max-inline-insns-single=500 -fno-rtti -fno-exceptions -MMD -DF_CPU=16000000 -DARDUINO=10607 -DARDUINO_DSD6 -DARDUINO_ARCH_NRF5 -DNRF52 -DNRF5'

    for p in extra_include_paths:
        compile_cmd += " -I" + p

    compile_cmd += ' .arduino/data/packages/nRF52D6Fitness/hardware/nRF5/0.7.5/variants/DSD6/variant.cpp -o .cache/bear_sample.cpp.o'
    c.run('bear -- ' + compile_cmd)


@task(iterable=['project'])
def build(c, project=[], clean=False):
    if not project:
        project=['rust', 'zig', 'arduino']

    if 'rust' in project:
        with c.cd('./atc-rust'):
            c.run('cargo build --release --target thumbv7em-none-eabihf')

    if 'zig' in project:
        with c.cd('./atc-zig'):
            c.run('zig build')

    if 'arduino' in project:
        output_file_path = "./ATCwatch/build/nRF52D6Fitness.nRF5.dsd6Watch/ATCwatch.ino.zip"
        size_old = 0
        if os.path.isfile(output_file_path):
            size_old = os.stat(output_file_path).st_size

        c.run('arduino-cli compile %(clean)s -e --fqbn nRF52D6Fitness:nRF5:dsd6Watch:softdevice=onlySoftDevice ATCwatch' % { "clean": "--clean" if clean else "" })

        if os.path.isfile(output_file_path):
            size_new = os.stat(output_file_path).st_size
            print("%(path)s\nsize: %(size_new)i +/-: %(size_diff)i " % { 'path': output_file_path, 'size_new': size_new, 'size_diff': size_new - size_old})
            print("")

    print("--- build complete ---")
