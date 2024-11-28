#include "WS_QMI8658.h"

WS_QMI8658 imu{};  // default I2C-pins on Waveshare ESP32S3 Matrix

void setup() {
  Serial.begin(115200);
  while (!Serial) {}  // ESP32S3, ESP32C3, ...

  Serial.println("Starting...");
  Serial.printf("Running on HAL: %s\n", HAL_NAME);
  
#if defined(ARDUINO_FQBN)
    Serial.printf("Board: %s\n", ARDUINO_FQBN);
#endif

  Serial.printf("WS_QMI8658 IMU Test %s", ARDUINO_FQBN);
  Serial.println();

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
