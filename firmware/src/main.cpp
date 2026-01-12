#include "board_hal.h"
#include "SparkFun_LSM6DSV16X.h"
#include <SPI.h>

// TODO
// notes in `maxl-dev/log/2025-11_accelerometer` 

// -------------------------------------------------------- LSM

SparkFun_LSM6DSV16X_SPI lsm;

// l-o-l 
volatile uint8_t  data_reading = 0;
uint64_t          data_stamps[2] = {0, 0};
sfe_lsm_data_t    data_accel[2];
sfe_lsm_data_t    data_gyro[2];

#define ERRCODE_ALL_GOOD 0 
#define ERRCODE_WAIT 1 
#define ERRCODE_NO_INIT 2
#define ERRCODE_RESETTING 3

uint8_t err_code = ERRCODE_WAIT;

uint8_t get_errcode(void){
  return err_code;
}

// BUILD_RPC(get_errcode, "", "code");

auto get_data(void){
  // read, flip, sendy 
  auto tup = std::make_tuple(
    data_stamps[data_reading], 
    data_accel[data_reading].xData, data_accel[data_reading].yData, data_accel[data_reading].zData, 
    data_gyro[data_reading].xData,  data_gyro[data_reading].yData,  data_gyro[data_reading].zData
  );
  data_reading = data_reading ? 0 : 1;
  return tup; 
}

// BUILD_RPC(get_data, "", "stamp, ax, ay, az, rx, ry, rz");

// -------------------------------------------------------- Timer ?
// code for RP2040, probably roll something new... / just poll AFAP 

/*
#define ALARM_DT_NUM 1
#define ALARM_DT_IRQ TIMER1_IRQ_1
#define ALARM_DT_PER_US 1000

// let's get our tight loop, don't let this run from FLASH
// https://www.raspberrypi.com/documentation/pico-sdk/ 
// runtime.html#group_pico_platform_1gad9ab05c9a8f0ab455a5e11773d610787
void __time_critical_func(alarm_handler)(void){
  // clear handly 
  hw_clear_bits(&timer_hw->intr, 1u << ALARM_DT_NUM);
  timer_hw->alarm[ALARM_DT_NUM] = (uint32_t) (timer_hw->timerawl + ALARM_DT_PER_US);

}

void alarm_begin(void){
  // setup the hardware timer 
  hw_set_bits(&timer_hw->inte, 1u << ALARM_DT_NUM);
  irq_set_exclusive_handler(ALARM_DT_IRQ, alarm_handler);
  irq_set_enabled(ALARM_DT_IRQ, true);
  timer_hw->alarm[ALARM_DT_NUM] = (uint32_t) (timer_hw->timerawl + ALARM_DT_PER_US);
}
*/

// -------------------------------------------------------- core 0 for co-mmunication 

void setup() {
  // only has one GPIO LED @ user, use that as 'green' for sync-blink, 
  // UART1 TX/RX are bottom pads / unused, dummies here... 
  pinMode(PIN_LED_USER, OUTPUT); digitalWrite(PIN_LED_USER, HIGH);
  pinMode(PIN_UART_RX1, OUTPUT); digitalWrite(PIN_UART_RX1, HIGH);
  pinMode(PIN_UART_TX1, OUTPUT); digitalWrite(PIN_UART_TX1, HIGH);
  // 
  analogWriteResolution(USE_PWM_RES_BITS);
}

void loop() {
  // 
}

// -------------------------------------------------------- core 1 for pollin' 
// https://github.com/sparkfun/SparkFun_LSM6DSV16X_Arduino_Library/blob/main/examples/example3_spi/example3_spi.ino 

void setup1(){
  // setup the arduino-style SPI interface... 
  SPI.setRX(PIN_LSM_SPI_MISO);
  SPI.setTX(PIN_LSM_SPI_MOSI);
  SPI.setSCK(PIN_LSM_SPI_CLK);
  SPI.setCS(PIN_LSM_SPI_CS);
  pinMode(PIN_LSM_SPI_CS, OUTPUT);
  digitalWrite(PIN_LSM_SPI_CS, HIGH);
  // TODO: how to set SPI rate, if needed ? 
  SPI.begin();

  // lsm has this implicitly... ffs, so just
  if(!lsm.begin(PIN_LSM_SPI_CS)){
    err_code = ERRCODE_NO_INIT;
    return;
  }

  lsm.deviceReset();
  while(!lsm.getDeviceReset()){
    err_code = ERRCODE_RESETTING;
    delay(1);
  }

  // don't update accel, gyro until read 
  lsm.enableBlockDataUpdate();

  // rates, ranges
  lsm.setAccelDataRate(LSM6DSV16X_ODR_AT_480Hz);
  lsm.setAccelFullScale(LSM6DSV16X_8g);
  lsm.setGyroDataRate(LSM6DSV16X_ODR_AT_480Hz);
  lsm.setGyroFullScale(LSM6DSV16X_250dps);

  // filter ?
  lsm.enableFilterSettling();
  lsm.enableAccelLP2Filter();
  lsm.setAccelLP2Bandwidth(LSM6DSV16X_XL_STRONG);
  lsm.enableGyroLP1Filter();
  lsm.setGyroLP1Bandwidth(LSM6DSV16X_GY_ULTRA_LIGHT);

  // should be ready then ? 
  err_code = ERRCODE_ALL_GOOD;
}

void loop1(){
  // let's just rip it as fast as we can ? 
  // kind of dogshit race-condition-avoidance, probably works when rates OK ... 
  uint8_t data_writing = data_reading ? 0 : 1;
  data_stamps[data_writing] = micros(); 
  lsm.getAccel(&(data_accel[data_writing]));
  lsm.getGyro(&(data_gyro[data_writing]));
  //
  delay(100);
}