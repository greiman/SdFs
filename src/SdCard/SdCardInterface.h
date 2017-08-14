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
#ifndef SdCardInterface_h
#define SdCardInterface_h
#include "BlockDeviceInterface.h"
#include "SdCardInfo.h"
/**
 * \class SdCardInterface
 * \brief Abstract interface for an SD card.
 */
class SdCardInterface : public BlockDeviceInterface {
 public:
   /** Erase a range of sectors.
   *
   * \param[in] firstSector The address of the first sector in the range.
   * \param[in] lastSector The address of the last sector in the range.
   *
   * \return true for success else false.
   */
  virtual bool erase(uint32_t firstSector, uint32_t lastSector) = 0;
  /** \return error code. */
  virtual uint8_t errorCode() const = 0;
  /** \return error data. */
  virtual uint32_t errorData() const = 0;
  /** \return true if card is busy. */
  virtual bool isBusy() = 0;
  /**
   * Read a card's CID register.
   *
   * \param[out] cid pointer to area for returned data.
   *
   * \return true for success or false for failure.
   */
  virtual bool readCID(cid_t* cid) = 0;
   /**
   * Read a card's CSD register.
   *
   * \param[out] csd pointer to area for returned data.
   *
   * \return true for success or false for failure.
   */
  virtual bool readCSD(csd_t* csd) = 0;
  /** Read OCR register.
   *
   * \param[out] ocr Value of OCR register.
   * \return true for success else false.
   */
  virtual bool readOCR(uint32_t* ocr) = 0;
  /**
   * Determine the size of an SD flash memory card.
   *
   * \return The number of 512 byte data sectors in the card
   *         or zero if an error occurs.
   */
  virtual uint32_t sectorCount() = 0;
  /** \return card status. */
  virtual uint32_t status() {return 0XFFFFFFFF;}
  /** Return the card type: SD V1, SD V2 or SDHC/SDXC
   * \return 0 - SD V1, 1 - SD V2, or 3 - SDHC/SDXC.
   */
  virtual uint8_t type() const = 0;
  /** Write one data sector in a multiple sector write sequence.
   * \param[in] src Pointer to the location of the data to be written.
   * \return true for success else false.
   */

  virtual bool writeData(const uint8_t* src) = 0;
  /** Start a write multiple sectors sequence.
   *
   * \param[in] sector Address of first sector in sequence.
   *
   * \return true for success else false.
   */
  virtual bool writeStart(uint32_t sector) = 0;
  /** End a write multiple sectors sequence.
   * \return true for success else false.
   */
  virtual bool writeStop() = 0;
};
#endif  // SdCardInterface_h
