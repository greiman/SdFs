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
#if defined(__STM32F1__)
#include "SdSpiDriver.h"
#define USE_STM32F1_DMAC 1
//------------------------------------------------------------------------------
#if BOARD_NR_SPI > 1
static SPIClass m_SPI2(2);
#endif  // BOARD_NR_SPI > 1
#if BOARD_NR_SPI > 2
static SPIClass m_SPI3(3);
#endif  // BOARD_NR_SPI > 2
//
static SPIClass* pSpi[] =
#if BOARD_NR_SPI == 1
  {&SPI};
#elif BOARD_NR_SPI == 2
  {&SPI, &m_SPI2};
#elif BOARD_NR_SPI == 3
  {&SPI, &m_SPI2, &m_SPI3};
#else  // BOARD_NR_SPI
#error "BOARD_NR_SPI too large"
#endif  // BOARD_NR_SPI
//------------------------------------------------------------------------------
/** Set SPI options for access to SD/SDHC cards.
 *
 * \param[in] divisor SCK clock divider relative to the APB1 or APB2 clock.
 */
void SdSpiAltDriver::activate() {
  m_spi->beginTransaction(m_spiSettings);
}
//------------------------------------------------------------------------------
/** Initialize the SPI bus.
 *
 * \param[in] chipSelectPin SD card chip select pin.
 */
void SdSpiAltDriver::begin(SdSpiConfig spiConfig) {
  if (spiConfig.spiPort > 0 && spiConfig.spiPort <= BOARD_NR_SPI) {
    m_spi = pSpi[spiConfig.spiPort - 1];
  } else {
    m_spi = pSpi[0];
  }
  m_csPin = spiConfig.csPin;
  pinMode(m_csPin, OUTPUT);
  digitalWrite(m_csPin, HIGH);
  m_spi->begin();
}
//------------------------------------------------------------------------------
/**
 * End SPI transaction.
 */
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
//------------------------------------------------------------------------------
/** Receive multiple bytes.
 *
 * \param[out] buf Buffer to receive the data.
 * \param[in] n Number of bytes to receive.
 *
 * \return Zero for no error or nonzero error code.
 */
uint8_t SdSpiAltDriver::receive(uint8_t* buf, size_t n) {
  int rtn = 0;
#if USE_STM32F1_DMAC
  rtn = m_spi->dmaTransfer(0, const_cast<uint8*>(buf), n);
#else  // USE_STM32F1_DMAC
  //  m_spi->read(buf, n); fails ?? use byte transfer
  for (size_t i = 0; i < n; i++) {
    buf[i] = m_spi->transfer(0XFF);
  }
#endif  // USE_STM32F1_DMAC
  return rtn;
}
//------------------------------------------------------------------------------
/** Send a byte.
 *
 * \param[in] b Byte to send
 */
void SdSpiAltDriver::send(uint8_t b) {
  m_spi->transfer(b);
}
//------------------------------------------------------------------------------
/** Send multiple bytes.
 *
 * \param[in] buf Buffer for data to be sent.
 * \param[in] n Number of bytes to send.
 */
void SdSpiAltDriver::send(const uint8_t* buf , size_t n) {
#if USE_STM32F1_DMAC
  m_spi->dmaSend(const_cast<uint8*>(buf), n);
#else  // #if USE_STM32F1_DMAC
  m_spi->write(buf, n);
#endif  // USE_STM32F1_DMAC
}
#endif  // defined(__STM32F1__)
