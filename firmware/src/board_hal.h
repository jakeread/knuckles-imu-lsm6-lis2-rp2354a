#ifndef BOARD_HAL_H_
#define BOARD_HAL_H_ 

// stub, cribbed from prior xiao version 

#define PIN_LED_USER 25 
#define PIN_UART_RX1 21 
#define PIN_UART_TX1 20

#define PIN_LSM_SPI_CS     5 // GPIO5, XIAO D3  | 1, 5, 17, 21 
#define PIN_LSM_SPI_MOSI   3 // GPIO3, XIAO D10 | 3, 7, 19, 23 
#define PIN_LSM_SPI_MISO   4 // GPIO4, XIAO D9  | 0, 4, 16, 20  
#define PIN_LSM_SPI_CLK    2 // GPIO2, XIAO D8  | 2, 6, 18, 22

// https://docs.sparkfun.com/SparkFun_6DoF_LSM6DSV16X/hardware_overview/#gpio
// https://www.raspberrypi.com/documentation/microcontrollers/pico-series.html 

#define USE_PWM_RES_BITS 10 

#endif
