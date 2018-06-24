/**
 * Copyright (c) 2011..2018 Bill Greiman
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
#ifndef SdFs_h
#define SdFs_h
/**
 * \file
 * \brief main SdFs include file.
 */
#include "SysCall.h"
#include "SdCard/SdCard.h"
#include "ExFatLib/ExFatLib.h"
#include "FatLib/FatLib.h"
#include "iostream/ArduinoStream.h"
#include "iostream/fstream.h"
#include "FsVolume.h"
#include "FsFile.h"
//------------------------------------------------------------------------------
/** SdFs version YYYYMMDD */
#define SD_FS_DATE 20180624
//==============================================================================
/**
 * \class SdBase
 * \brief base SD file system template class.
 */
template <class Vol>
class SdBase : public Vol {
 public:
  //----------------------------------------------------------------------------
  /** Initialize SD card and file system.
   *
   * \param[in] csPin SD card chip select pin.
   * \return true for success else false.
   */
  bool begin(uint8_t csPin = SS) {
#ifdef BUILTIN_SDCARD
    if (csPin == BUILTIN_SDCARD) {
      return begin(SdioConfig(FIFO_SDIO));
    }
#endif  // BUILTIN_SDCARD
    return begin(SdSpiConfig(csPin, SHARED_SPI, SPI_FULL_SPEED));
  }
  //----------------------------------------------------------------------------
  /** Initialize SD card and file system.
   *
   * \param[in] csPin SD card chip select pin.
   * \param[in] spiSettings SPI speed, mode, and bit order.
   * \return true for success else false.
   */
  bool begin(uint8_t csPin, SPISettings spiSettings) {
    return begin(SdSpiConfig(csPin, SHARED_SPI, spiSettings));
  }
  //----------------------------------------------------------------------------
  /** Initialize SD card and file system for SPI mode.
   *
   * \param[in] spiConfig SPI configuration.
   * \return true for success else false.
   */
  bool begin(SdSpiConfig spiConfig) {
    return cardBegin(spiConfig) && Vol::begin(m_card);
  }
  //---------------------------------------------------------------------------
  /** Initialize SD card and file system for SDIO mode.
   *
   * \param[in] sdioConfig SDIO configuration.
   * \return true for success else false.
   */
  bool begin(SdioConfig sdioConfig) {
    return cardBegin(sdioConfig) && Vol::begin(m_card);
  }
  //----------------------------------------------------------------------------
  /** \return Pointer to SD card object. */
  SdCard* card() {return m_card;}
  //----------------------------------------------------------------------------
  /** Initialize SD card in SPI mode.
   *
   * \param[in] spiConfig SPI configuration.
   * \return true for success else false.
   */
  bool cardBegin(SdSpiConfig spiConfig) {
    m_card = m_cardFactory.newCard(spiConfig);
    return m_card && !m_card->errorCode();
  }
  //----------------------------------------------------------------------------
  /** Initialize SD card in SDIO mode.
   *
   * \param[in] sdioConfig SDIO configuration.
   * \return true for success else false.
   */
  bool cardBegin(SdioConfig sdioConfig) {
    m_card = m_cardFactory.newCard(sdioConfig);
    return m_card && !m_card->errorCode();
  }


  //----------------------------------------------------------------------------
  /** \return SD card error code. */
  uint8_t sdErrorCode() {
    if (m_card) {
      return m_card->errorCode();
    }
    return SD_CARD_ERROR_INVALID_CARD_CONFIG;
  }
  //----------------------------------------------------------------------------
  /** \return SD card error data. */
  uint8_t sdErrorData() {return m_card ? m_card->errorData() : 0;}
  //----------------------------------------------------------------------------
  /** %Print error info and halt.
   *
   * \param[in] pr Print destination.
   */
  void errorHalt(Print* pr) {
    if (sdErrorCode()) {
      pr->print(F("SdError: 0X"));
      pr->print(sdErrorCode(), HEX);
      pr->print(F(",0X"));
      pr->println(sdErrorData(), HEX);
    } else if (!Vol::fatType()) {
      pr->println(F("Check SD format."));
    }
    SysCall::halt();
  }
  //----------------------------------------------------------------------------
  /** %Print error info to Serial and halt. */
  void errorHalt() {errorHalt(&Serial);}
  //----------------------------------------------------------------------------
  /** %Print error info and halt.
   *
   * \param[in] pr Print destination.
   * \param[in] msg Message to print.
   */
  void errorHalt(Print* pr, const char* msg) {
    pr->print(F("error: "));
    pr->println(msg);
    errorHalt(pr);
  }
  //----------------------------------------------------------------------------
  /** %Print msg and halt.
   *
   * \param[in] pr Print destination.
   * \param[in] msg Message to print.
   */
  void errorHalt(Print* pr, const __FlashStringHelper* msg) {
    pr->print(F("error: "));
    pr->println(msg);
    errorHalt(pr);
  }
  //----------------------------------------------------------------------------
  /** %Print msg to Serial and halt.
   *
   * \param[in] msg Message to print.
   */
  void errorHalt(const __FlashStringHelper* msg) {
    errorHalt(&Serial, msg);
  }
  //----------------------------------------------------------------------------
  /** %Print error info and halt.
   *
   * \param[in] msg Message to print.
   */
  void errorHalt(const char* msg) {errorHalt(&Serial, msg);}
  //----------------------------------------------------------------------------
  /** %Print error info and halt.
   *
   * \param[in] pr Print destination.
   */
  void initErrorHalt(Print* pr) {
    pr->println(F("begin() failed"));
    if (sdErrorCode()) {
      pr->println(F("Do not reformat the SD."));
      if (sdErrorCode() == SD_CARD_ERROR_CMD0) {
        pr->println(F("No card, wrong chip select pin, or wiring error?"));
      }
    }
    errorHalt(pr);
  }
  //----------------------------------------------------------------------------
  /** %Print error info and halt. */
  void initErrorHalt() {initErrorHalt(&Serial);}
  //----------------------------------------------------------------------------
  /** %Print volume FAT/exFAT type.
   *
   * \param[in] pr Print destination.
   */
  void printFatType(Print* pr) {
    if (Vol::fatType() == FAT_TYPE_EXFAT) {
      pr->print(F("exFAT"));
    } else {
      pr->print(F("FAT"));
      pr->print(Vol::fatType());
    }
  }
  //----------------------------------------------------------------------------
  /** %Print SD errorCode and errorData.
   *
   * \param[in] pr Print destination.
   */
  void printSdErrorCode(Print* pr) {
    if (sdErrorCode()) {
      pr->print(F("SdError: 0X"));
      pr->print(sdErrorCode(), HEX);
      pr->print(F(",0X"));
      pr->println(sdErrorData(), HEX);
    }
  }
  //----------------------------------------------------------------------------
  /** %Print error info and return.
   *
   * \param[in] pr Print destination.
   */
  void printSdError(print_t* pr) {
    if (sdErrorCode()) {
      if (sdErrorCode() == SD_CARD_ERROR_CMD0) {
        pr->println(F("No card, wrong chip select pin, or wiring error?"));
      }
      pr->print(F("SD error: "));
      printSdErrorSymbol(pr, sdErrorCode());
      pr->print(F(" = 0x"));
      pr->print(sdErrorCode(), HEX);
      pr->print(F(",0x"));
      pr->println(sdErrorData(), HEX);
    } else if (!Vol::cwv()) {
      pr->println(F("Check SD format."));
    }
  }
  //----------------------------------------------------------------------------
  /** Initialize file system after call to cardBegin.
   *
   * \return true for success else false.
   */
  bool volumeBegin() {
     return Vol::begin(m_card);
  }
  //----------------------------------------------------------------------------
 private:
  SdCard* m_card;
  SdCardFactory m_cardFactory;
};
//------------------------------------------------------------------------------
/**
 * \class SdFat
 * \brief SD file system class for FAT volumes.
 */
class SdFat : public SdBase<FatVolume> {
 public:
  /** Format a SD card FAT32/FAT16.
   *
   * \param[in] pr Optional Print information.
   * \return true for success else false.
   */
  bool format(print_t* pr = nullptr) {
    FatFormatter fmt;
    uint8_t* cache = reinterpret_cast<uint8_t*>(cacheClear());
    if (!cache) {
      return false;
    }
    return fmt.format(card(), cache, pr);
  }
};
//------------------------------------------------------------------------------
/**
 * \class SdExFat
 * \brief SD file system class for exFAT volumes.
 */
class SdExFat : public SdBase<ExFatVolume> {
 public:
  /** Format a SD card exFAT.
   *
   * \param[in] pr Optional Print information.
   * \return true for success else false.
   */
  bool format(print_t* pr = nullptr) {
    ExFatFormatter fmt;
    uint8_t* cache = reinterpret_cast<uint8_t*>(cacheClear());
    if (!cache) {
      return false;
    }
    return fmt.format(card(), cache, pr);
  }
};
//------------------------------------------------------------------------------
/**
 * \class SdFs
 * \brief SD file system class for FAT16, FAT32, and exFAT volumes.
 */
class SdFs : public SdBase<FsVolume> {
};
#endif  // SdFs_h


