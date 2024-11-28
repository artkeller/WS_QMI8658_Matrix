#pragma once

#ifndef GENERIC_HAL_IDENTIFICATION_H
#define GENERIC_HAL_IDENTIFICATION_H

// Automatische Architekturzuordnung
#if defined(ARDUINO_ARCH_ESP32)
    #include "ESP32_HAL_Identifier.h"
#elif defined(ARDUINO_ARCH_AVR)
    #include "AVR_HAL_Identifier.h"
#elif defined(ARDUINO_ARCH_RP2040)
    #include "RP2040_HAL_Identifier.h"
#elif defined(ARDUINO_ARCH_STM32)
    #include "STM32_HAL_Identifier.h"
#else
    #error "Unsupported architecture. Please add a HAL for this platform."
#endif

#if HAL_SUPPORTS_PREFERENCES
#include <Preferences.h> 
#endif

#if HAL_HAS_STL
#include <map>  
#endif

#endif // GENERIC_HAL_IDENTIFICATION_H
