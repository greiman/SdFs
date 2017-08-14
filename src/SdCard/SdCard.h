/* Arduino SdCard Library
 * Copyright (C) 2011..2017 by William Greiman
 *
 * This file is part of the Arduino SdCard Library
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
 * along with the Arduino SdCard Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef SdCard_h
#define SdCard_h
#include "SdioCard.h"
#include "SdSpiCard.h"
#if HAS_SDIO_CLASS
typedef SdCardInterface SdCard;
#else  // HAS_SDIO_CLASS
typedef SdSpiCard SdCard;
#endif  // HAS_SDIO_CLASS
/** Determine card configuration type.
 *
 * \param[in] cfg Card configuration.
 * \return true if SPI.
 */
inline bool isSpi(SdSpiConfig cfg) {(void)cfg; return true;}
/** Determine card configuration type.
 *
 * \param[in] cfg Card configuration.
 * \return true if SPI.
 */
inline bool isSpi(SdioConfig cfg) {(void)cfg; return false;}
/**
 * \class SdCardFactory
 * \brief Setup a SPI card or SDIO card.
 */
class SdCardFactory {
 public:
  /** Initialize SPI card.
   *
   * \param[in] config SPI configuration.
   * \return generic card pointer.
   */
  SdCard* newCard(SdSpiConfig config) {
    m_spiCard.begin(&m_spi, config);
    return &m_spiCard;
  }
  /** Initialize SDIO card.
   *
   * \param[in] config SDIO configuration.
   * \return generic card pointer or nullptr if SDIO is not supported.
   */
  SdCard* newCard(SdioConfig config) {
#if HAS_SDIO_CLASS
    m_sdioCard.begin(config);
    return &m_sdioCard;
#else  // HAS_SDIO_CLASS
    (void)config;
    return nullptr;
#endif  // HAS_SDIO_CLASS
  }

 private:
#if HAS_SDIO_CLASS
  SdioCard m_sdioCard;
#endif  // HAS_SDIO_CLASS
  SdSpiCard m_spiCard;
  SdSpiDriver m_spi;
};
#endif  // SdCard_h
