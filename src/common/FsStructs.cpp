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
#include "FsStructs.h"
// bgnLba = relSector;
// endLba = relSector + partSize - 1;
void lbaToMbrChs(uint8_t* chs, uint32_t capacityMB, uint32_t lba) {
  uint32_t c;
  uint8_t h;
  uint8_t s;

  uint8_t numberOfHeads;
  uint8_t sectorsPerTrack = capacityMB <= 256 ? 32 : 63;
  if (capacityMB <= 16) {
    numberOfHeads = 2;
  } else if (capacityMB <= 32) {
    numberOfHeads = 4;
  } else if (capacityMB <= 128) {
    numberOfHeads = 8;
  } else if (capacityMB <= 504) {
    numberOfHeads = 16;
  } else if (capacityMB <= 1008) {
    numberOfHeads = 32;
  } else if (capacityMB <= 2016) {
    numberOfHeads = 64;
  } else if (capacityMB <= 4032) {
    numberOfHeads = 128;
  } else {
    numberOfHeads = 255;
  }
  c = lba / (numberOfHeads * sectorsPerTrack);
  if (c <= 1023) {
    h = (lba % (numberOfHeads * sectorsPerTrack)) / sectorsPerTrack;
    s = (lba % sectorsPerTrack) + 1;
  } else {
    c = 1023;
    h = 254;
    s = 63;
  }
  chs[0] = h;
  chs[1] = ((c >> 2) & 0XC0) | s;
  chs[2] = c;
}
