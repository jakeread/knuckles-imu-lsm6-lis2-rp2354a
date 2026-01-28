#include "board_hal.h"
#include "SparkFun_LSM6DSV16X.h"
#include <SPI.h>

// TODO
// notes in `maxl-dev/log/2025-11_accelerometer` 

// -------------------------------------------------------- LSM

SparkFun_LSM6DSV16X_SPI lsm;

// -------------------------------------------------------- core 0 for co-mmunication 

void setup() {
  return;

  // normie serial stuff, 
  Serial.begin(); 
  delay(1000);

  // setup the arduino-style SPI interface... 
  SPI.setRX(PIN_LSM_SPI_MISO);
  SPI.setTX(PIN_LSM_SPI_MOSI);
  SPI.setSCK(PIN_LSM_SPI_CLK);
  SPI.setCS(PIN_LSM_SPI_CS);
  pinMode(PIN_LSM_SPI_CS, OUTPUT);
  digitalWrite(PIN_LSM_SPI_CS, HIGH);
  SPI.begin();

  // lsm has this implicitly... ffs, so just
  if(!lsm.begin(PIN_LSM_SPI_CS)){
    Serial.println("lsm.begin fails...");
    return;
  }

  lsm.deviceReset();
  while(!lsm.getDeviceReset()){
    Serial.println("lsm.getDeviceReset() ...");
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
}

sfe_lsm_data_t data_accel;
sfe_lsm_data_t data_gyro;

void loop() {
  return;
  digitalWrite(PIN_LED_R, !digitalRead(PIN_LED_R));

  lsm.getAccel(&data_accel);
  lsm.getGyro(&data_gyro);

  Serial.println("\tAX: " + String(data_accel.xData, 3) + "\tAY: " + String(data_accel.yData, 3) + "\tAZ: " + String(data_accel.zData, 3));
  Serial.println("\tGX: " + String(data_gyro.xData, 3) + "\tGY: " + String(data_gyro.yData, 3) + "\tGZ: " + String(data_gyro.zData, 3));

  // 
  delay(100);
}

/// 

void setup1(){
  pinMode(PIN_LED_R, OUTPUT); digitalWrite(PIN_LED_R, HIGH);
  pinMode(PIN_LED_G, OUTPUT); digitalWrite(PIN_LED_G, HIGH);
  pinMode(PIN_LED_B, OUTPUT); digitalWrite(PIN_LED_B, HIGH);
  analogWriteResolution(USE_PWM_RES_BITS);
}

void loop1(){
  // le blink 
  // digitalWrite(PIN_LED_R, !digitalRead(PIN_LED_R));
  digitalWrite(PIN_LED_G, !digitalRead(PIN_LED_G));
  // digitalWrite(PIN_LED_B, !digitalRead(PIN_LED_B));
  delay(100);
}