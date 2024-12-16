#include "WS_QMI8658.h"

WS_QMI8658 imu{};         // default I2C-pins on Waveshare ESP32S3 Matrix
//WS_QMI8658 imu(11,12);  // myOwn I2C-pins 

void setup() {
  Serial.begin(115200);
  while (!Serial) {}  // ESP32S3, ESP32C3, ...

  Serial.println("Starting...");
  Serial.printf("Running on HAL: %s\n", HAL_NAME);
  
#if defined(ARDUINO_FQBN)
    Serial.printf("Board: %s\n", ARDUINO_FQBN);
#endif

  if (imu.begin() == WS_QMI8658_ERR) {
    Serial.println("IMU not initialized - STOP");
    while (1) {}
  };
}

void loop() {
  static Side previousSide = UNKNOWN;
  Side currentSide = imu.update(100);
  if (currentSide != UNKNOWN && previousSide != currentSide) {
    previousSide = currentSide;
    Serial.printf("Aktuelle Seite: %s\n", imu.sideToString(currentSide));
  }
}
