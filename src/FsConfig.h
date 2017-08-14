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
 * \brief configuration definitions
 */
#ifndef FsConfig_h
#define FsConfig_h
#include <stdint.h>
#ifdef __AVR__
#include <avr/io.h>
#endif  // __AVR__
/** For Debug - must be one */
#define ENABLE_ARDUINO_FEATURES 1
#include "Arduino.h"
//------------------------------------------------------------------------------
/**
 * Set USE_LONG_FILE_NAMES nonzero to use long file names (LFN) in FAT16/FAT32.
 * exFAT always uses long file names.
 *
 * Long File Name are limited to a maximum length of 255 characters.
 *
 * This implementation allows 7-bit characters in the range
 * 0X20 to 0X7E except the following characters are not allowed:
 *
 *  < (less than)
 *  > (greater than)
 *  : (colon)
 *  " (double quote)
 *  / (forward slash)
 *  \ (backslash)
 *  | (vertical bar or pipe)
 *  ? (question mark)
 *  * (asterisk)
 *
 */
#define USE_LONG_FILE_NAMES 1
//------------------------------------------------------------------------------
/**
 * Set ENABLE_DEDICATED_SPI to enable dedicated use of the SPI bus.
 * Selecting dedicated SPI in SdSpiConfig() will produce better
 * performance by using very large multi-block transfers to and
 * from the SD card.
 *
 * Enabling dedicated SPI will cost some extra flash and RAM.
 */
#ifdef __AVR_ATmega328P__
#define ENABLE_DEDICATED_SPI 0
#else  // __AVR_ATmega328P__
#define ENABLE_DEDICATED_SPI 1
#endif  // __AVR_ATmega328P__
//-----------------------------------------------------------------------------
/**
 * If the symbol USE_STANDARD_SPI_LIBRARY is nonzero, the standard
 * Arduino SPI.h library will be used. If USE_STANDARD_SPI_LIBRARY
 * is zero, an optimized custom SPI driver is used if it exists.
 */
#define USE_STANDARD_SPI_LIBRARY 0
//------------------------------------------------------------------------------
/**
 * Set MAINTAIN_FREE_CLUSTER_COUNT nonzero to keep the count of free clusters
 * updated.  This will increase the speed of the freeClusterCount() call
 * after the first call.  Extra flash will be required.
 */
#define MAINTAIN_FREE_CLUSTER_COUNT 0
//------------------------------------------------------------------------------
/**
 * To enable SD card CRC checking for SPI, set USE_SD_CRC nonzero.
 *
 * Set USE_SD_CRC to 1 to use a smaller CRC-CCITT function.  This function
 * is slower for AVR but may be fast for ARM and other processors.
 *
 * Set USE_SD_CRC to 2 to used a larger table driven CRC-CCITT function.  This
 * function is faster for AVR but may be slower for ARM and other processors.
 */
#define USE_SD_CRC 0
//------------------------------------------------------------------------------
/**
 * Handle Watchdog Timer for WiFi modules.
 *
 * Yield will be called before accessing the SPI bus if it has been more
 * than WDT_YIELD_TIME_MICROS microseconds since the last yield call by SdFat.
 */
#if defined(PLATFORM_ID) || defined(ESP8266)
// If Particle device or ESP8266 call yield.
#define WDT_YIELD_TIME_MICROS 100000
#else
#define WDT_YIELD_TIME_MICROS 0
#endif
//------------------------------------------------------------------------------
/**
 * Set FAT12_SUPPORT nonzero to enable use if FAT12 volumes.
 * FAT12 has not been well tested and requires additional flash.
 */
#define FAT12_SUPPORT 0
//------------------------------------------------------------------------------
/**
 * Set DESTRUCTOR_CLOSES_FILE nonzero to close a file in its destructor.
 *
 * Causes use of lots of heap in ARM.
 */
#define DESTRUCTOR_CLOSES_FILE 0
//------------------------------------------------------------------------------
/**
 * Call flush for endl if ENDL_CALLS_FLUSH is nonzero
 *
 * The standard for iostreams is to call flush.  This is very costly for
 * SdFat.  Each call to flush causes 2048 bytes of I/O to the SD.
 *
 * SdFat has a single 512 byte buffer for SD I/O so it must write the current
 * data sector to the SD, read the directory sector from the SD, update the
 * directory entry, write the directory sector to the SD and read the data
 * sector back into the buffer.
 *
 * The SD flash memory controller is not designed for this many rewrites
 * so performance may be reduced by more than a factor of 100.
 *
 * If ENDL_CALLS_FLUSH is zero, you must call flush and/or close to force
 * all data to be written to the SD.
 */
#define ENDL_CALLS_FLUSH 0
//------------------------------------------------------------------------------
/**
 * Set USE_SIMPLE_LITTLE_ENDIAN nonzero for little endian processors
 * with no memory alignment restrictions.
 */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ && !defined(__SAMD21G18A__)\
  && !defined(__MKL26Z64__) && !defined(ESP8266)
#define USE_SIMPLE_LITTLE_ENDIAN 1
#else  // __BYTE_ORDER_
#define USE_SIMPLE_LITTLE_ENDIAN 0
#endif  // __BYTE_ORDER_
//------------------------------------------------------------------------------
/**
 * Set USE_SEPARATE_FAT_CACHE nonzero to use a second 512 byte cache
 * for FAT table entries.  This improves performance for large writes
 * that are not a multiple of 512 bytes.
 */
#ifdef __arm__
#define USE_SEPARATE_FAT_CACHE 1
#else  // __arm__
#define USE_SEPARATE_FAT_CACHE 0
#endif  // __arm__
//------------------------------------------------------------------------------
/**
 * Set USE_MULTI_SECTOR_IO nonzero to use multi-sector SD read/write.
 *
 * Don't use mult-sector read/write on small AVR boards.
 */
#if defined(RAMEND) && RAMEND < 3000
#define USE_MULTI_SECTOR_IO 0
#else  // RAMEND
#define USE_MULTI_SECTOR_IO 1
#endif  // RAMEND
//-----------------------------------------------------------------------------
/** Enable SDIO driver if available. */
#if defined(__MK64FX512__) || defined(__MK66FX1M0__)
#ifndef SDCARD_SPI
#define SDCARD_SPI      SPI1
#define SDCARD_MISO_PIN 59
#define SDCARD_MOSI_PIN 61
#define SDCARD_SCK_PIN  60
#define SDCARD_SS_PIN   62
#endif  // SDCARD_SPI
#define HAS_SDIO_CLASS 1
#else  // HAS_SDIO_CLASS
#define HAS_SDIO_CLASS 0
#endif  // HAS_SDIO_CLASS
//------------------------------------------------------------------------------
/**
 * Determine the default SPI configuration.
 */
#if defined(__STM32F1__) || (defined(__arm__) && defined(CORE_TEENSY))
// has multiple SPI ports
#define SD_HAS_CUSTOM_SPI 2
#elif defined(__AVR__) || defined(__SAM3X8E__)\
  || defined(__SAM3X8H__) || defined(ESP8266)
#define SD_HAS_CUSTOM_SPI 1
#else  // SD_HAS_CUSTOM_SPI
// Use standard SPI library.
#define SD_HAS_CUSTOM_SPI 0
#endif  // SD_HAS_CUSTOM_SPI
//------------------------------------------------------------------------------
/**
 * Check if API to select HW SPI port is needed.
 */
#if SD_HAS_CUSTOM_SPI < 2
#define IMPLEMENT_SPI_PORT_SELECTION 0
#else  // SD_HAS_CUSTOM_SPI < 2
#define IMPLEMENT_SPI_PORT_SELECTION 1
#endif  // SD_HAS_CUSTOM_SPI < 2
#endif  // FsConfig_h
