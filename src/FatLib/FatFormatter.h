/* FatLib Library
 * Copyright (C) 2012..2017 by William Greiman
 *
 * This file is part of the FatLib Library
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
 * along with the FatLib Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef FatFormatter_h
#define FatFormatter_h
#include "FatFile.h"
#include "SysCall.h"
#include "BlockDevice.h"
#include "../common/FsStructs.h"
/**
 * \class FatFormatter
 * \brief Format a FAT volume.
 */
class FatFormatter {
 public:
  /**
   * Format a FAT volume.
   *
   * \param[in] dev Block device for volume.
   * \param[in] secBuffer buffer for writing to volume.
   * \param[in] pr Print device for progress output.
   *
   * \return true for success else false.
   */
  bool format(BlockDevice* dev, uint8_t* secBuffer, print_t* pr = nullptr);

 private:
  bool initFatDir(uint8_t fatType, uint32_t sectorCount);
  void initPbs();
  bool makeFat16();
  bool makeFat32();
  bool writeMbr();
  uint32_t m_capacityMB;
  uint32_t m_dataStart;
  uint32_t m_fatSize;
  uint32_t m_fatStart;
  uint32_t m_relativeSectors;
  uint32_t m_sectorCount;
  uint32_t m_totalSectors;
  BlockDevice* m_dev;
  print_t*m_pr;
  uint8_t* m_secBuf;
  uint16_t m_reservedSectorCount;
  uint8_t m_partType;
  uint8_t m_sectorsPerCluster;
};
#endif  // FatFormatter_h
