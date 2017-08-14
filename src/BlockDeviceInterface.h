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
#ifndef BlockDeviceInterface_h
#define BlockDeviceInterface_h
/**
 * \file
 * \brief BlockDeviceInterface include file.
 */
#include <stdint.h>
#include <stddef.h>
/**
 * \class BlockDeviceInterface
 * \brief BlockDeviceInterface class.
 */
class BlockDeviceInterface {
 public:
  virtual ~BlockDeviceInterface() {}
  /**
   * Read a 512 byte sector.
   *
   * \param[in] sector Logical sector to be read.
   * \param[out] dst Pointer to the location that will receive the data.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  virtual bool readSector(uint32_t sector, uint8_t* dst) = 0;

  /**
   * Read multiple 512 byte sectors.
   *
   * \param[in] sector Logical sector to be read.
   * \param[in] ns Number of sectors to be read.
   * \param[out] dst Pointer to the location that will receive the data.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  virtual bool readSectors(uint32_t sector, uint8_t* dst, size_t ns) = 0;

  /** \return device size in sectors. */
  virtual uint32_t sectorCount() = 0;

  /** End multi-sector transfer and go to idle state.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  virtual bool syncDevice() = 0;

  /**
   * Writes a 512 byte sector.
   *
   * \param[in] sector Logical sector to be written.
   * \param[in] src Pointer to the location of the data to be written.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  virtual bool writeSector(uint32_t sector, const uint8_t* src) = 0;

  /**
   * Write multiple 512 byte sectors.
   *
   * \param[in] sector Logical sector to be written.
   * \param[in] ns Number of sectors to be written.
   * \param[in] src Pointer to the location of the data to be written.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  virtual bool writeSectors(uint32_t sector, const uint8_t* src, size_t ns) = 0;
};
#endif  // BlockDeviceInterface_h
