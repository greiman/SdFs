/* ExFat Library
 * Copyright (C) 2016..2017 by William Greiman
 *
 * This file is part of the ExFat Library
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
 * along with the ExFat Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef ExFatFormatter_h
#define ExFatFormatter_h
#include "ExFatConfig.h"
#include "SysCall.h"
#include "BlockDevice.h"
#include "upcase.h"
/**
 * \class ExFatFormatter
 * \brief Format an exFAT volume.
 */
class ExFatFormatter {
 public:
  /**
   * Format an exFAT volume.
   *
   * \param[in] dev Block device for volume.
   * \param[in] secBuf buffer for writing to volume.
   * \param[in] pr Print device for progress output.
   *
   * \return true for success else false.
   */
  bool format(BlockDevice* dev, uint8_t* secBuf, print_t* pr = nullptr);
 private:
  bool syncUpcase();
  bool writeUpcase(uint32_t sector);
  bool writeUpcaseByte(uint8_t b);
  bool writeUpcaseUnicode(uint16_t unicode);
  uint32_t m_upcaseSector;
  uint32_t m_upcaseChecksum;
  uint32_t m_upcaseSize;
  BlockDevice* m_dev;
  uint8_t* m_secBuf;
};
#endif  // ExFatFormatter_h
