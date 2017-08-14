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
#include "FsVolume.h"
#include "FsFile.h"
FsVolume* FsVolume::m_cwv = nullptr;
//------------------------------------------------------------------------------
bool FsVolume::begin(BlockDevice* blockDev) {
  m_blockDev = blockDev;
  m_fVol = nullptr;
  m_xVol = new (m_volMem) ExFatVolume;
  if (m_xVol && m_xVol->begin(m_blockDev)) {
    goto done;
  }
  m_xVol = nullptr;
  m_fVol = new (m_volMem) FatVolume;
  if (m_fVol && m_fVol->begin(m_blockDev)) {
    goto done;
  }
  m_cwv = nullptr;
  m_fVol = nullptr;
  return false;

 done:
  m_cwv = this;
  return true;
}
//------------------------------------------------------------------------------
void FsVolume::ls(print_t* pr, const char* path, uint8_t flags) {
  FsFile dir;
  dir.open(this, path, O_READ);
  dir.ls(pr, flags);
}
//------------------------------------------------------------------------------
FsFile FsVolume::open(const char *path, uint8_t oflag) {
  FsFile tmpFile;
  tmpFile.open(this, path, oflag);
  return tmpFile;
}
#if ENABLE_ARDUINO_FEATURES
//------------------------------------------------------------------------------
FsFile FsVolume::open(const String &path, uint8_t oflag) {
  return open(path.c_str(), oflag );
}
#endif  // ENABLE_ARDUINO_FEATURES
