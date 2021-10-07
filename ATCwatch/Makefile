BOARD = nRF52D6Fitness:nRF5:dsd6Watch:softdevice=onlySoftDevice
BIN_DIR = /tmp/arduino_build

compile:
	arduino --pref build.path=$(BIN_DIR) --preserve-temp-files --board $(BOARD) \
			--verify --verbose *ino
