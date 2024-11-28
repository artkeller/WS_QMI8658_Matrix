#include "WS_QMI8658.h"

// Standard-Konstruktor (verwendet vordefinierte Pins)
WS_QMI8658::WS_QMI8658()
  : drd(nullptr), refAccel{ 0, 0, -1 }, previousSide(UNKNOWN),
    previousAccelData{ 0, 0, 0 }, sdaPin(I2C_SDA), sclPin(I2C_SCL) {}

// Konstruktor mit benutzerdefinierten Pins
WS_QMI8658::WS_QMI8658(int sdaPin, int sclPin)
  : drd(nullptr), refAccel{ 0, 0, -1 }, previousSide(UNKNOWN),
    previousAccelData{ 0, 0, 0 }, sdaPin(sdaPin), sclPin(sclPin) {}

WS_QMI8658::~WS_QMI8658() {
  delete drd;
}

WS_QMI8658_RETURN_CODE WS_QMI8658::begin() {
  // Initialisiere DoubleResetDetector
  log_d("Initialize DRD");
  drd = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);
  if (!drd) {
    log_e("Error in memory allocation for DoubleResetDetector!");
    return WS_QMI8658_ERR;
  } else {
    log_d("DRD initialized, timeout:%d, address:%d", DRD_TIMEOUT, DRD_ADDRESS);
  }
  // Initialisiere Sensor
  log_d("Initialize QMI8658 sensor");
  if (!sensor.begin(Wire, QMI8658_L_SLAVE_ADDRESS, sdaPin, sclPin)) {
    log_e("Failed to initialize QMI8658 sensor!");
    return WS_QMI8658_ERR;
  }

  // Konfiguriere Sensor
  sensor.configAccelerometer(SensorQMI8658::ACC_RANGE_4G, SensorQMI8658::ACC_ODR_1000Hz, SensorQMI8658::LPF_MODE_0);
  //sensor.configGyroscope(SensorQMI8658::GYR_RANGE_64DPS, SensorQMI8658::GYR_ODR_896_8Hz, SensorQMI8658::LPF_MODE_3);

  sensor.enableAccelerometer();
  //sensor.enableGyroscope();

  sensor.dumpCtrlRegister();  // printf register configuration information

  //log_d("QMI8658 %lu %.2f ℃", sensor.getTimestamp(), sensor.getTemperature_C());
  log_d("QMI8658 sensor initialized");

  // NVS (Preferences) initialisieren
  preferences.begin("IMUcal", false);  // "IMUcal" ist der Namespace

  // Prüfe, ob ein Doppel-Reset zum Zurücksetzen der Kalibrierungsdaten nötig ist
  if (detectDoubleReset()) {
    resetCalibrationData();
    Serial.println("Kalibrierungsdaten gelöscht.");
  }

  // Prüfen, ob Kalibrierungsdaten bereits vorhanden sind
  if (preferences.isKey("refAccelX")) {
    // Lade Kalibrierungsdaten
    loadCalibrationData();
    Serial.println("Kalibrierungsdaten geladen:");
    Serial.printf("X: %f, Y: %f, Z: %f\n", refAccel.x, refAccel.y, refAccel.z);
  } else {
    Serial.println("Keine Kalibrierungsdaten vorhanden. Kalibriere das Board.");
    calibrateBoard();
  }
  return WS_QMI8658_OK;
}

bool WS_QMI8658::detectDoubleReset() {
  return drd->detectDoubleReset();
}

void WS_QMI8658::calibrateBoard() {
  Serial.println("Lege das Board flach hin und warte...");
  delay(3000);

  // Kalibrierungsdaten erfassen
  sensor.getAccelerometer(refAccel.x, refAccel.y, refAccel.z);

  Serial.printf("Kalibrierte Werte: X=%f, Y=%f, Z=%f\n", refAccel.x, refAccel.y, refAccel.z);
  saveCalibrationData();
}

void WS_QMI8658::loadCalibrationData() {
#if HAL_SUPPORTS_PREFERENCES
  refAccel.x = preferences.getFloat("refAccelX", 0);
  refAccel.y = preferences.getFloat("refAccelY", 0);
  refAccel.z = preferences.getFloat("refAccelZ", -1);
#else
  // Kalibrierwerte aus EEPROM laden
  for (size_t i = 0; i < 3; i++) {
    calibrationData.calibrationValues[i] = preferences.get<float>(EEPROM_CALIB_ADDRESS + i * sizeof(float), 0);
  }
  // Synchronisiere die `refAccel` Struktur mit den geladenen Werten
  refAccel.x = calibrationData.refAccel.x;
  refAccel.y = calibrationData.refAccel.y;
  refAccel.z = calibrationData.refAccel.z;
#endif
}

void WS_QMI8658::saveCalibrationData() {
#if HAL_SUPPORTS_PREFERENCES
  preferences.putFloat("refAccelX", refAccel.x);
  preferences.putFloat("refAccelY", refAccel.y);
  preferences.putFloat("refAccelZ", refAccel.z);
#else
  // Speichere die Kalibrierwerte als Array
  for (size_t i = 0; i < sizeof(calibrationData.calibrationValues) / sizeof(float); i++) {
    preferences.put(EEPROM_CALIB_ADDRESS + i * sizeof(float), calibrationData.calibrationValues[i]);
  }

#endif
}

void WS_QMI8658::resetCalibrationData() {
#if HAL_SUPPORTS_PREFERENCES
  preferences.clear();
#else
  preferences.clear(EEPROM_CALIB_ADDRESS, sizeof(DEFAULT_CALIB_VALUES));
#endif
}

Side WS_QMI8658::getDownwardFacingSide(const IMUdata& accelData, float hysteresisThreshold) {
  float calibratedX = accelData.x - refAccel.x;
  float calibratedY = accelData.y - refAccel.y;
  float calibratedZ = accelData.z;

  Side currentSide;
  if (abs(calibratedX) > abs(calibratedY) && abs(calibratedX) > abs(calibratedZ)) {
    currentSide = calibratedX > 0 ? FRONT : BACK;
  } else if (abs(calibratedY) > abs(calibratedX) && abs(calibratedY) > abs(calibratedZ)) {
    currentSide = calibratedY > 0 ? LEFT : RIGHT;
  } else {
    currentSide = UNKNOWN;
  }

  if (previousSide != UNKNOWN && currentSide != UNKNOWN) {
    if (abs(calibratedX - previousAccelData.x) < hysteresisThreshold
        && abs(calibratedY - previousAccelData.y) < hysteresisThreshold
        && abs(calibratedZ - previousAccelData.z) < hysteresisThreshold) {
      return previousSide;
    }
  }

  previousSide = currentSide;
  previousAccelData = { calibratedX, calibratedY, calibratedZ };
  return currentSide;
}

Side WS_QMI8658::update(int maxTimerMillis) {
  drd->loop();

  // Führe eine Kalibrierung durch, wenn nötig
  if (!preferences.isKey("refAccelX")) {
    calibrateBoard();
  }

  if (timerMillis >= maxTimerMillis) {
    timerMillis = 0;

    IMUdata Accel;
    //IMUdata Gyro;

    sensor.getAccelerometer(Accel.x, Accel.y, Accel.z);
    //sensor.getGyroscope(Gyro.x, Gyro.y, Gyro.z);

    return getDownwardFacingSide(Accel);
  }
  return UNKNOWN;
}

// Umwandlung von Enum-Werten in Strings
const char* WS_QMI8658::sideToString(Side side) {
#if HAL_HAS_STL
  if (sideToStringMap.find(side) != sideToStringMap.end()) {
    return sideToStringMap[side];
  }
#else
  // Suche den passenden String
  for (const auto& entry : sideToStringMap) {
    if (entry.side == side) {
      return entry.name;
    }
  }
#endif
  return "Invalid Side";  // should not happen
}
