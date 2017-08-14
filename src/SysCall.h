/* SdFs Library
 * Copyright (C) 2016..2017 by William Greiman
 *
 * This file is part of the SdFs Library
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the SdFs Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
/**
 * \file
 * \brief SysCall class
 */
#ifndef SysCall_h
#define SysCall_h
#include <stdint.h>
#include <stddef.h>
#include "FsConfig.h"

#if __cplusplus < 201103
#warning nullptr defined
/** Define nullptr if not C++11 */
#define nullptr NULL
//  #error C++11 Support required
#endif

#if ENABLE_ARDUINO_FEATURES
#if defined(ARDUINO)
#include <Arduino.h>
#include <SPI.h>
typedef Print print_t;

#elif defined(PLATFORM_ID)  // Only defined if a Particle device

#include "application.h"
#else  // defined(ARDUINO)
#error "Unknown system"
#endif  // defined(ARDUINO)
//-----------------------------------------------------------------------------
#ifdef ESP8266
// undefine F macro if ESP8266.
#undef F
#endif  // ESP8266
//-----------------------------------------------------------------------------
#ifndef F
/** Define macro for strings stored in flash. */
#define F(str) (str)
#endif  // F
//-----------------------------------------------------------------------------
/** \return the time in milliseconds. */
inline uint16_t curTimeMS() {
  return millis();
}
//-----------------------------------------------------------------------------
/** \return the time in milliseconds. */
inline uint32_t curMs() {
  return millis();
}
//-----------------------------------------------------------------------------
/**
 * \class SysCall
 * \brief SysCall - Class to wrap system calls.
 */
class SysCall {
 public:
  /** Halt execution of this thread. */
  static void halt() {
    while (1) {
      yield();
    }
  }
  /** Yield to other threads. */
  static void yield();
};

#if defined(ESP8266)
inline void SysCall::yield() {
  // Avoid ESP8266 bug
  delay(0);
}
#elif defined(ARDUINO)
inline void SysCall::yield() {
  // Use the external Arduino yield() function.
  ::yield();
}
#elif defined(PLATFORM_ID)  // Only defined if a Particle device
inline void SysCall::yield() {
  Particle.process();
}
#else  // ESP8266
inline void SysCall::yield() {}
#endif  // ESP8266
//-----------------------------------------------------------------------------
#else  // ENABLE_ARDUINO_FEATURES

#ifndef F
/** Define macro for strings stored in flash. */
#define F(str) (str)
#endif  // F
#ifndef HIGH
#define HIGH 1
#endif  // HIGH
#ifndef LOW
#define LOW 0
#endif  // LOW
void digitalWrite(uint8_t, int);

class  print_t {
 public:
  virtual size_t write(char c) = 0;
  virtual size_t write(const char* s) = 0;
  size_t print(const char* s) {return write(s);}
};
//-----------------------------------------------------------------------------
/** \return the time in milliseconds. */
inline uint32_t curMs() {
  return 0;
}
class SysCall {
 public:
  /** Halt execution of this thread. */
  static void halt() {
    while (1) {
      yield();
    }
  }
  /** Yield to other threads. */
  static void yield() {}
};
//-----------------------------------------------------------------------------
#ifndef MSBFIRST
#define MSBFIRST 1
#endif  // MSBFIRST
#ifndef SPI_MODE0
#define SPI_MODE0 0
#endif  // MSBFIRST
struct SPISettings {
  uint32_t clockHz;
  SPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) {
    clockHz = clock;
    (void)bitOrder;
    (void) dataMode;
  }
  SPISettings() {
    clockHz = 4000000;
  }
};
#endif  // IS_ARDUINO
#endif  // SysCall_h
