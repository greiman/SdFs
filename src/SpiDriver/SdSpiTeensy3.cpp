/* Arduino SdSpiDriver Library
 * Copyright (C) 2013..2017 by William Greiman
 *
 * This file is part of the Arduino SdSpiDriver Library
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
 * along with the Arduino SdSpiDriver Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include "SdSpiDriver.h"
#if defined(__arm__) && defined(CORE_TEENSY)
#define USE_BLOCK_TRANSFER 1
//------------------------------------------------------------------------------
void SdSpiAltDriver::activate() {
  m_spi->beginTransaction(m_spiSettings);
}
//------------------------------------------------------------------------------
void SdSpiAltDriver::begin(SdSpiConfig spiConfig) {
  m_csPin = spiConfig.csPin;
#ifdef SDCARD_SPI
  if (m_csPin == SDCARD_SS_PIN) {
    m_spi = &SDCARD_SPI;
    m_spi->setMISO(SDCARD_MISO_PIN);
    m_spi->setMOSI(SDCARD_MOSI_PIN);
    m_spi->setSCK(SDCARD_SCK_PIN);
  } else {
    m_spi = &SPI;
  }
#else  //  SDCARD_SPI
  m_spi = &SPI;
#endif  //  SDCARD_SPI
  pinMode(m_csPin, OUTPUT);
  digitalWrite(m_csPin, HIGH);
  m_spi->begin();
}
//------------------------------------------------------------------------------
void SdSpiAltDriver::deactivate() {
  m_spi->endTransaction();
}
//------------------------------------------------------------------------------
/** Receive a byte.
 *
 * \return The byte.
 */
uint8_t SdSpiAltDriver::receive() {
  return m_spi->transfer(0XFF);
}
/** Receive multiple bytes.
 *
 * \param[out] buf Buffer to receive the data.
 * \param[in] n Number of bytes to receive.
 *
 * \return Zero for no error or nonzero error code.
 */
uint8_t SdSpiAltDriver::receive(uint8_t* buf, size_t n) {
#if USE_BLOCK_TRANSFER
  memset(buf, 0XFF, n);
  m_spi->transfer(buf, n);
#else  // USE_BLOCK_TRANSFER
  for (size_t i = 0; i < n; i++) {
    buf[i] = m_spi->transfer(0XFF);
  }
#endif  // USE_BLOCK_TRANSFER
  return 0;
}
/** Send a byte.
 *
 * \param[in] b Byte to send
 */
void SdSpiAltDriver::send(uint8_t b) {
  m_spi->transfer(b);
}
/** Send multiple bytes.
 *
 * \param[in] buf Buffer for data to be sent.
 * \param[in] n Number of bytes to send.
 */
void SdSpiAltDriver::send(const uint8_t* buf , size_t n) {
#if USE_BLOCK_TRANSFER
  uint32_t tmp[128];
  if (0 < n && n <= 512) {
    memcpy(tmp, buf, n);
    m_spi->transfer(tmp, n);
    return;
  }
#endif  // USE_BLOCK_TRANSFER
  for (size_t i = 0; i < n; i++) {
    m_spi->transfer(buf[i]);
  }
}
#endif  // defined(__arm__) && defined(CORE_TEENSY)
