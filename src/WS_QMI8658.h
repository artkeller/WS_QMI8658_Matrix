#pragma once

// author: artkeller@gmx.de 
// created: 20241128
// copyright: 2024 Thomas Walloschke
// licence: CC 1.0

#ifndef WS_QMI8658_H
#define WS_QMI8658_H

#ifndef WS_QMI8658_VERSION
  #define WS_QMI8658_VERSION            "Waveshare QMI8658 v0.1.3"

  #define WS_QMI8658_VERSION_MAJOR      0
  #define WS_QMI8658_VERSION_MINOR      1
  #define WS_QMI8658_VERSION_PATCH      3

  #define WS_QMI8658_VERSION_INT        0001003

#endif

#include <Arduino.h>
#include <elapsedMillis.h>
#include <Wire.h>
#include <SensorLib.h>        // SensorLib: https://github.com/lewisxhe/SensorLib/blob/master/src/SensorLib.h
#include <SensorQMI8658.hpp>  // SensorLib driver for QMI8658: https://github.com/lewisxhe/SensorLib/blob/master/src/SensorQMI8658.hpp
                              // Equv BMI160: https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bmi160-ds000.pdf

// DoubleResetDetector (DRD) Konfiguration
#define DOUBLERESETDETECTOR_DEBUG true
#define DRD_TIMEOUT 10
#include "Generic_HAL_Identification.h"  // Architekturzuordnung und DoubleResetDetector

enum WS_QMI8658_RETURN_CODE {
  WS_QMI8658_OK,
  WS_QMI8658_ERR
};

// ESP32S3 Matrix
#define I2C_SDA 11
#define I2C_SCL 12

enum Side {
  FRONT = 1,
  BACK = 3,
  LEFT = 2,
  RIGHT = 0,
  UNKNOWN = 9
};

class WS_QMI8658 {
public:
  // Konstruktor für Standard-I2C-Pins
  WS_QMI8658();

  // Konstruktor für benutzerdefinierte I2C-Pins
  WS_QMI8658(int sdaPin, int sclPin);

  ~WS_QMI8658();

  WS_QMI8658_RETURN_CODE begin();
  bool detectDoubleReset();
  void calibrateBoard();
  Side getDownwardFacingSide(const IMUdata& accelData, float hysteresisThreshold = 0.1);
  Side update(int maxTimerMillis = 100);
  const char* sideToString(Side side);

private:
  void loadCalibrationData();
  void saveCalibrationData();
  void resetCalibrationData();

  Preferences preferences;
  DoubleResetDetector* drd;
  SensorQMI8658 sensor;
  IMUdata refAccel;
  elapsedMillis timerMillis;

  Side previousSide;
  IMUdata previousAccelData;

#if not HAL_SUPPORTS_PREFERENCES
  // Union für die Kalibrierung, um auf die selben Speicheradressen zuzugreifen
  union CalibrationData {
    IMUdata refAccel;           // Kalibrierwerte als IMUdata X,Y,Z
    float calibrationValues[3];  // Array für Kalibrierwerte  0,1,2
  };

  // Instanz der Union, nur innerhalb der Klasse zugänglich
  CalibrationData calibrationData;
#endif

  int sdaPin;  // Benutzerdefinierte SDA-Pin-Nummer
  int sclPin;  // Benutzerdefinierte SCL-Pin-Nummer

// Map zur Umwandlung von Enum in String
#if HAL_HAS_STL
  std::map<Side, const char*> sideToStringMap = {
#else
  static const struct {
    Side side;
    const char* name;
  } sideToStringMap[] = {
#endif
    { FRONT, "FRONT" },
    { BACK, "BACK" },
    { LEFT, "LEFT" },
    { RIGHT, "RIGHT" },
    { UNKNOWN, "UNKNOWN" }
  };
};

#endif  // WS_QMI8658_H
