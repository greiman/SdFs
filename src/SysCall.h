/**
 * Copyright (c) 20011..2017 Bill Greiman
 * This file is part of the SdFs library for SD memory cards.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
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
