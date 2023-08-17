from invoke import task
import os

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

    c.run("sed -i 's/\(compiler.path=\).*/\\1/' " + platform_txt_uri)
    c.run("sed -i 's/\(compiler.ldflags=.*\)/\\1 -L \{runtime\.tools\.gcc-arm-none-eabi-5_2-2015q4\.path\}\/lib -L " + libraries_dir + "\/atc-rust\/src\/cortex-m4 -L " + libraries_dir + "\/atc-zig\/src\/cortex-m4 -L " + libraries_dir + "\/HRS3300-Arduino-Library\/src\/cortex-m4 /' " + platform_txt_uri)
    c.run("sed -i 's/\(recipe.c.combine.pattern=.*\)/\\1 -W -lheart -latcrust -latczig /' " + platform_txt_uri)
    print("--- init complete ---")

@task
def build(c):
    with c.cd('./atc-rust'):
        c.run('cargo build --release --target thumbv7em-none-eabihf')

    with c.cd('./atc-zig'):
        c.run('zig build')

    c.run("arduino-cli compile --clean -e --fqbn nRF52D6Fitness:nRF5:dsd6Watch:softdevice=onlySoftDevice ATCwatch")

    c.run("ls -al ./ATCwatch/build/nRF52D6Fitness.nRF5.dsd6Watch/ATCwatch.ino.zip")
    print("--- build complete ---")
