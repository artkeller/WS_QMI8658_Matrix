#pragma once

#ifndef ESP32_HAL_IDENTIFIER_H
#define ESP32_HAL_IDENTIFIER_H

// Definiere nützliche Konstanten
#define HAL_NAME "ESP32"
#define HAL_HAS_STL true
#define HAL_SUPPORTS_PREFERENCES true  // ESP32 Non-Volatile Storage (NVS)

// Wichtige Header für die ESP32-Architektur
// DoubleResetDetector (DRD) Konfiguration
#define ESP_DRD_USE_LITTLEFS false
#define ESP_DRD_USE_SPIFFS false
#define ESP_DRD_USE_EEPROM true

#define DRD_ADDRESS 0

#include <ESP_DoubleResetDetector.h>
#define DoubleResetDetector DoubleResetDetector // Class mapping

#endif // ESP32_HAL_IDENTIFIER_H
