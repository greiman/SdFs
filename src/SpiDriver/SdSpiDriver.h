/**
 * Copyright (c) 20011-2017 Bill Greiman
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
 * \brief SpiDriver classes
 */
#ifndef SdSpiDriver_h
#define SdSpiDriver_h
#include "SysCall.h"
//------------------------------------------------------------------------------
/** SPISettings for SCK frequency in Hz. */
#define SD_SCK_HZ(maxSpeed) SPISettings(maxSpeed, MSBFIRST, SPI_MODE0)
/** SPISettings for SCK frequency in MHz. */
#define SD_SCK_MHZ(maxMhz) SPISettings(1000000UL*maxMhz, MSBFIRST, SPI_MODE0)
// SPI divisor constants
/** Set SCK to max rate of F_CPU/2. */
#define SPI_FULL_SPEED SD_SCK_MHZ(50)
/** Set SCK rate to F_CPU/3 for Due */
#define SPI_DIV3_SPEED SD_SCK_HZ(F_CPU/3)
/** Set SCK rate to F_CPU/4. */
#define SPI_HALF_SPEED SD_SCK_HZ(F_CPU/4)
/** Set SCK rate to F_CPU/6 for Due */
#define SPI_DIV6_SPEED SD_SCK_HZ(F_CPU/6)
/** Set SCK rate to F_CPU/8. */
#define SPI_QUARTER_SPEED SD_SCK_HZ(F_CPU/8)
/** Set SCK rate to F_CPU/16. */
#define SPI_EIGHTH_SPEED SD_SCK_HZ(F_CPU/16)
/** Set SCK rate to F_CPU/32. */
#define SPI_SIXTEENTH_SPEED SD_SCK_HZ(F_CPU/32)
//------------------------------------------------------------------------------
/** The SD is the only device on the SPI bus. */
#define DEDICATED_SPI 0X80
/** SPI bus is share with other devices. */
#define SHARED_SPI 0
/**
 * \class SdSpiConfig
 * \brief SPI card configuration.
 */
class SdSpiConfig {
 public:
   /** SdSpiConfig constructor.
   *
   * \param[in] cs Chip select pin.
   * \param[in] opt Options.
   * \param[in] settings SPISettings.
   * \param[in] port The SPI port to use.
   */
  SdSpiConfig(uint8_t cs, uint8_t opt, SPISettings settings, uint8_t port) :
    spiSettings(settings), csPin(cs), options(opt), spiPort(port) {}

  /** SdSpiConfig constructor.
   *
   * \param[in] cs Chip select pin.
   * \param[in] opt Options.
   * \param[in] settings SPISettings.
   */
  SdSpiConfig(uint8_t cs, uint8_t opt, SPISettings settings) :
    spiSettings(settings), csPin(cs), options(opt), spiPort(-1) {}
  /** SdSpiConfig constructor.
   *
   * \param[in] cs Chip select pin.
   * \param[in] opt Options.
   */
  SdSpiConfig(uint8_t cs, uint8_t opt) :
    spiSettings(SPI_FULL_SPEED), csPin(cs), options(opt), spiPort(-1)  {}
  /** SdSpiConfig constructor.
   *
   * \param[in] cs Chip select pin.
   */
  explicit SdSpiConfig(uint8_t cs) :
    spiSettings(SPI_FULL_SPEED), csPin(cs), options(SHARED_SPI), spiPort(-1) {}

  /** SPISettings */
  const SPISettings spiSettings;
  /** Chip select pin. */
  const uint8_t csPin;
  /** Options */
  const uint8_t options;
  /** SPI port */
  const int8_t spiPort;
};
//------------------------------------------------------------------------------
/**
 * \class SdSpiLibDriver
 * \brief SdSpiLibDriver - use standard SPI library.
 */
class SdSpiLibDriver {
 public:
  /** Activate SPI hardware. */
  void activate() {
    m_spi->beginTransaction(m_spiSettings);
  }
  /** Initialize the SPI bus.
   *
   * \param[in] spiConfig SD card configuration.
   */
  void begin(SdSpiConfig spiConfig) {
    m_csPin = spiConfig.csPin;
#if defined(SDCARD_SPI) && defined(SDCARD_SS_PIN)
    if (m_csPin == SDCARD_SS_PIN) {
      m_spi = &SDCARD_SPI;
    } else {
      m_spi = &SPI;
    }
#else  // defined(SDCARD_SPI) && defined(SDCARD_SS_PIN)
    m_spi = &SPI;
#endif  // defined(SDCARD_SPI) && defined(SDCARD_SS_PIN)
    digitalWrite(m_csPin, HIGH);
    pinMode(m_csPin, OUTPUT);
    m_spi->begin();
  }
  /** Deactivate SPI hardware. */
  void deactivate() {
    m_spi->endTransaction();
  }
  /** Receive a byte.
   *
   * \return The byte.
   */
  uint8_t receive() {
    return m_spi->transfer( 0XFF);
  }
  /** Receive multiple bytes.
  *
  * \param[out] buf Buffer to receive the data.
  * \param[in] n Number of bytes to receive.
  *
  * \return Zero for no error or nonzero error code.
  */
  uint8_t receive(uint8_t* buf, size_t n) {
    for (size_t i = 0; i < n; i++) {
      buf[i] = m_spi->transfer(0XFF);
    }

    return 0;
  }
  /** Send a byte.
   *
   * \param[in] data Byte to send
   */
  void send(uint8_t data) {
    m_spi->transfer(data);
  }
  /** Send multiple bytes.
   *
   * \param[in] buf Buffer for data to be sent.
   * \param[in] n Number of bytes to send.
   */
  void send(const uint8_t* buf, size_t n) {
    for (size_t i = 0; i < n; i++) {
      m_spi->transfer(buf[i]);
    }
  }
  /** Set CS low. */
  void select() {
    digitalWrite(m_csPin, LOW);
  }
  /** Save SPISettings.
   *
   * \param[in] spiSettings SPI speed, mode, and byte order.
   */
  void setSpiSettings(SPISettings spiSettings) {
    m_spiSettings = spiSettings;
  }
  /** Set CS high. */
  void unselect() {
    digitalWrite(m_csPin, HIGH);
  }

 private:
  SPIClass* m_spi;
  SPISettings m_spiSettings;
  uint8_t m_csPin;
};
//------------------------------------------------------------------------------
/**
 * \class SdSpiAltDriver
 * \brief Optimized SPI class for access to SD and SDHC flash memory cards.
 */
class SdSpiAltDriver {
 public:
#if IMPLEMENT_SPI_PORT_SELECTION
  SdSpiAltDriver() : m_spi(nullptr) {}
#endif  // IMPLEMENT_SPI_PORT_SELECTION
  /** Activate SPI hardware. */
  void activate();
  /** deactivate SPI driver. */
  void end();
  /** Deactivate SPI hardware. */
  void deactivate();
  /** Initialize the SPI bus.
   *
   * \param[in] spiConfig SD card configuration.
   */
  void begin(SdSpiConfig spiConfig);
  /** Receive a byte.
   *
   * \return The byte.
   */
  uint8_t receive();
  /** Receive multiple bytes.
  *
  * \param[out] buf Buffer to receive the data.
  * \param[in] n Number of bytes to receive.
  *
  * \return Zero for no error or nonzero error code.
  */
  uint8_t receive(uint8_t* buf, size_t n);
  /** Send a byte.
   *
   * \param[in] data Byte to send
   */
  void send(uint8_t data);
  /** Send multiple bytes.
   *
   * \param[in] buf Buffer for data to be sent.
   * \param[in] n Number of bytes to send.
   */
  void send(const uint8_t* buf, size_t n);
  /** Set CS low. */
  void select() {
     digitalWrite(m_csPin, LOW);
  }
  /** Save SPISettings.
   *
   * \param[in] spiSettings SPI speed, mode, and byte order.
   */
  void setSpiSettings(SPISettings spiSettings) {
    m_spiSettings = spiSettings;
  }
  /** Set CS high. */
  void unselect() {
    digitalWrite(m_csPin, HIGH);
  }

 private:
#if IMPLEMENT_SPI_PORT_SELECTION || defined(DOXYGEN)
  SPIClass *m_spi;
#endif  // IMPLEMENT_SPI_PORT_SELECTION
  SPISettings m_spiSettings;
  uint8_t m_csPin;
};
//-----------------------------------------------------------------------------
// Choose SPI driver for SdFat and SdFatEX classes.
#if USE_STANDARD_SPI_LIBRARY || !SD_HAS_CUSTOM_SPI
/** SdFat uses Arduino library SPI. */
typedef SdSpiLibDriver SdSpiDriver;
#else  // USE_STANDARD_SPI_LIBRARY || !SD_HAS_CUSTOM_SPI
/** SdFat uses custom fast SPI. */
typedef SdSpiAltDriver SdSpiDriver;
#endif  // USE_STANDARD_SPI_LIBRARY || !SD_HAS_CUSTOM_SPI
//=============================================================================
// Use of in-line for AVR to save flash.
#if  defined(__AVR__) && ENABLE_ARDUINO_FEATURES
#define nop asm volatile ("nop\n\t")
//------------------------------------------------------------------------------
inline void SdSpiAltDriver::begin(SdSpiConfig spiConfig) {
  m_csPin = spiConfig.csPin;
  pinMode(m_csPin, OUTPUT);
  digitalWrite(m_csPin, HIGH);
  SPI.begin();
}
//------------------------------------------------------------------------------
inline void SdSpiAltDriver::activate() {
  SPI.beginTransaction(m_spiSettings);
}
//------------------------------------------------------------------------------
inline void SdSpiAltDriver::deactivate() {
  SPI.endTransaction();
}
//------------------------------------------------------------------------------
inline uint8_t SdSpiAltDriver::receive() {
  SPDR = 0XFF;
  while (!(SPSR & (1 << SPIF))) {}
  return SPDR;
}
//------------------------------------------------------------------------------
inline uint8_t SdSpiAltDriver::receive(uint8_t* buf, size_t n) {
  if (n == 0) {
    return 0;
  }
  uint8_t* pr = buf;
  SPDR = 0XFF;
  while (--n > 0) {
    while (!(SPSR & _BV(SPIF))) {}
    uint8_t in = SPDR;
    SPDR = 0XFF;
    *pr++ = in;
    // nops to optimize loop for 16MHz CPU 8 MHz SPI
    nop;
    nop;
  }
  while (!(SPSR & _BV(SPIF))) {}
  *pr = SPDR;
  return 0;
}
//------------------------------------------------------------------------------
inline void SdSpiAltDriver::send(uint8_t data) {
  SPDR = data;
  while (!(SPSR & (1 << SPIF))) {}
}
//------------------------------------------------------------------------------
inline void SdSpiAltDriver::send(const uint8_t* buf , size_t n) {
  if (n == 0) {
    return;
  }
  SPDR = *buf++;
  while (--n > 0) {
    uint8_t b = *buf++;
    while (!(SPSR & (1 << SPIF))) {}
    SPDR = b;
    // nops to optimize loop for 16MHz CPU 8 MHz SPI
    nop;
    nop;
  }
  while (!(SPSR & (1 << SPIF))) {}
  return;
}
#endif  // __AVR__
#endif  // SdSpiDriver_h
