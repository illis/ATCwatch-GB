#ifndef _PINOUT_
#define _PINOUT_

#define MODEL_P22

//P22########################################################################################
#ifdef MODEL_P22
//Display
#define LCD_SCK      2
#define LCD_SDI      3
#define LCD_CS       25
#define LCD_RESET    26
#define LCD_RS       18
#define LCD_DET      9

//Flash Chip
#define SPI_SCK      2
#define SPI_MOSI     3
#define SPI_MISO     4
#define SPI_CE       5

//Touchscreen
#define TP_SDA       6
#define TP_SCL       7
#define TP_RESET     10
#define TP_INT       28

//Accl Sensor
#define BMA421_SDA   6
#define BMA421_SCL   7
#define BMA421_INT   8


//HeartRate Sensor
#define HRS3300_SDA  6
#define HRS3300_SCL  7
#define HRS3300_TEST 30

//Battery
#define CHARGE_INDICATION 12 
#define POWER_INDICATION  19
#define BATTERY_VOLTAGE   31
#define POWER_CONTROL     24

//InputOutput
#define STATUS_LED        27
#define VIBRATOR_OUT      16
#define PUSH_BUTTON_IN    15  
#define PUSH_BUTTON_OUT   13 

//Backlight
#define LCD_BACKLIGHT_LOW  14
#define LCD_BACKLIGHT_MID  22
#define LCD_BACKLIGHT_HIGH 23

#else //P22

//P8########################################################################################
#ifdef MODEL_P8
//display
#define LCD_SCK      2
#define LCD_SDI      3
#define LCD_CS       25
#define LCD_RESET    26
#define LCD_RS       18
#define LCD_DET      9

//Flash Chip
#define SPI_SCK      2
#define SPI_MOSI     3
#define SPI_MISO     4
#define SPI_CE       5

//Touchscreen
#define TP_SDA       6
#define TP_SCL       7
#define TP_RESET     13
#define TP_INT       28

//Accl Sensor
#define BMA421_SDA   6
#define BMA421_SCL   7
#define BMA421_INT  -1

//HeartRate Sensor
#define HRS3300_SDA  6
#define HRS3300_SCL  7
#define HRS3300_TEST 30

//Battery
#define CHARGE_INDICATION -1
#define POWER_INDICATION  19
#define BATTERY_VOLTAGE   31
#define POWER_CONTROL 24

//InputOutput
#define STATUS_LED        27
#define VIBRATOR_OUT      16
#define PUSH_BUTTON_IN    17
#define PUSH_BUTTON_OUT   -1

//Backlight
#define LCD_BACKLIGHT_LOW  14
#define LCD_BACKLIGHT_MID  22
#define LCD_BACKLIGHT_HIGH 23

#else //P8

//Pinetime########################################################################################
//display
#define LCD_SCK      2
#define LCD_SDI      3
#define LCD_CS       25
#define LCD_RESET    26
#define LCD_RS       18
#define LCD_DET      9

//Flash Chip
#define SPI_SCK      2
#define SPI_MOSI     3
#define SPI_MISO     4
#define SPI_CE       5

//Touchscreen
#define TP_SDA       6
#define TP_SCL       7
#define TP_RESET     10 //PinetTime
#define TP_INT       28

//Accl Sensor
#define BMA421_SDA       6
#define BMA421_SCL       7
#define BMA421_INT       8
#define SWITCH_X_Y

//HeartRate Sensor
#define HRS3300_SDA       6
#define HRS3300_SCL       7
#define HRS3300_TEST      30

//Battery
#define CHARGE_INDICATION 12
#define POWER_INDICATION  19
#define BATTERY_VOLTAGE   31
#define POWER_CONTROL 24

//InputOutput
#define STATUS_LED        27
#define VIBRATOR_OUT      16
#define PUSH_BUTTON_IN    13
#define PUSH_BUTTON_OUT   15

//Backlight
#define LCD_BACKLIGHT_LOW  14
#define LCD_BACKLIGHT_MID  22
#define LCD_BACKLIGHT_HIGH 23


#endif  //P22
#endif  //P8

#endif // _PINOUT_
