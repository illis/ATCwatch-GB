from invoke import task
import os

@task
def init(c):
    c.run("arduino-cli core update-index")
    c.run("arduino-cli core install nRF52D6Fitness:nRF5")

    arduino_data_dir = os.environ['ARDUINO_DIRECTORIES_DATA']
    platform_txt_uri = arduino_data_dir + "/packages/nRF52D6Fitness/hardware/nRF5/0.7.5/platform.txt"
    libraries_dir = "\.\/ATCwatch\/libraries"

    c.run("sed -i 's/\(compiler.path=\).*/\\1/' " + platform_txt_uri)
    c.run("sed -i 's/\(compiler.ldflags=.*\)/\\1 -L \{runtime\.tools\.gcc-arm-none-eabi-5_2-2015q4\.path\}\/lib -L " + libraries_dir + "\/atc-rust\/src\/cortex-m4 -L " + libraries_dir + "\/atc-zig\/src\/cortex-m4 -L " + libraries_dir + "\/HRS3300-Arduino-Library-master\/src\/cortex-m4 /' " + platform_txt_uri)
    c.run("sed -i 's/\(recipe.c.combine.pattern=.*\)/\\1 -lheart -latcrust -latczig/' " + platform_txt_uri)

@task
def build(c):
    with c.cd('./atc-rust'):
        c.run('cargo build --release --target thumbv7em-none-eabihf')

    with c.cd('./atc-zig'):
        c.run('zig build')

    c.run("arduino-cli compile --clean -e --fqbn nRF52D6Fitness:nRF5:dsd6Watch:softdevice=onlySoftDevice ATCwatch")

    print("--- compile complete ---")
    c.run("ls -al ./ATCwatch/build/nRF52D6Fitness.nRF5.dsd6Watch/ATCwatch.ino.zip")
