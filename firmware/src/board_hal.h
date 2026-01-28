#ifndef BOARD_HAL_H_
#define BOARD_HAL_H_ 

// stub, cribbed from prior xiao version 

#define PIN_LED_USER_WS2812 12

#define PIN_LED_R           16
#define PIN_LED_G           18
#define PIN_LED_B           17

// LSM6DSV16X
#define PIN_LSM_SPI_CS      13 // F1: SPI1
#define PIN_LSM_SPI_MOSI    15 // F1: SPI1
#define PIN_LSM_SPI_MISO    12 // F1: SPI1
#define PIN_LSM_SPI_CLK     14 // F1: SPI1
#define PIN_LSM_INT1        11
#define PIN_LSM_INT2        10

// LIS2MDLTR
#define PIN_MAG_SPI_CS      21 // F1: SPI0
#define PIN_MAG_SPI_MOSI    23 // F1: SPI0
#define PIN_MAG_SPI_MISO    20 // F1: SPI0
#define PIN_MAG_SPI_CLK     22 // F1: SPI0

// https://docs.sparkfun.com/SparkFun_6DoF_LSM6DSV16X/hardware_overview/#gpio
// https://www.raspberrypi.com/documentation/microcontrollers/pico-series.html 

#define USE_PWM_RES_BITS 10 

#endif
