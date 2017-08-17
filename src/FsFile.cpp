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
//-----------------------------------------------------------------------------
FsFile::FsFile(const FsFile& from) {
  close();
  if (from.m_fFile) {
    m_fFile = new (m_fileMem) FatFile;
    *m_fFile = *from.m_fFile;
  } else if (from.m_xFile) {
    m_xFile = new (m_fileMem) ExFatFile;
    *m_xFile = *from.m_xFile;
  }
}
//-----------------------------------------------------------------------------
FsFile& FsFile::operator=(const FsFile& from) {
  if (this == &from) return *this;
  close();
  if (from.m_fFile) {
    m_fFile = new (m_fileMem) FatFile;
    *m_fFile = *from.m_fFile;
  } else if (from.m_xFile) {
    m_xFile = new (m_fileMem) ExFatFile;
    *m_xFile = *from.m_xFile;
  }
  return *this;
}
//-----------------------------------------------------------------------------
bool FsFile::close() {
  if (m_fFile && m_fFile->close()) {
    m_fFile = nullptr;
    return true;
  }
  if (m_xFile && m_xFile->close()) {
    m_xFile = nullptr;
    return true;
  }
  return false;
}
//-----------------------------------------------------------------------------
bool FsFile::mkdir(FsFile* dir, const char* path, bool pFlag) {
  close();
  if (dir->m_fFile) {
    m_fFile = new (m_fileMem) FatFile;
    if (m_fFile->mkdir(dir->m_fFile, path, pFlag)) {
      return true;
    }
    m_fFile = nullptr;
  } else if (dir->m_xFile) {
    m_xFile = new (m_fileMem) ExFatFile;
    if (m_xFile->mkdir(dir->m_xFile, path, pFlag)) {
      return true;
    }
    m_xFile = nullptr;
  }
  return false;
}
//-----------------------------------------------------------------------------
bool FsFile::open(FsVolume* vol, const char* path, uint8_t oflag) {
  if (!vol) {
    return false;
  }
  close();
  if (vol->m_fVol) {
    m_fFile = new (m_fileMem) FatFile;
    if (m_fFile && m_fFile->open(vol->m_fVol, path, oflag)) {
      return true;
    }
    m_fFile = nullptr;
    return false;
  } else if (vol->m_xVol) {
    m_xFile = new (m_fileMem) ExFatFile;
    if (m_xFile && m_xFile->open(vol->m_xVol, path, oflag)) {
      return true;
    }
    m_xFile = nullptr;
  }
  return false;
}
//-----------------------------------------------------------------------------
bool FsFile::open(FsFile* dir, const char* path, uint8_t oflag) {
  close();
  if (dir->m_fFile) {
    m_fFile = new (m_fileMem) FatFile;
    if (m_fFile->open(dir->m_fFile, path, oflag)) {
      return true;
    }
    m_fFile = nullptr;
  } else if (dir->m_xFile) {
    m_xFile = new (m_fileMem) ExFatFile;
    if (m_xFile->open(dir->m_xFile, path, oflag)) {
      return true;
    }
    m_xFile = nullptr;
  }
  return false;
}
//-----------------------------------------------------------------------------
bool FsFile::openNext(FsFile* dir, uint8_t oflag) {
  close();
  if (dir->m_fFile) {
    m_fFile = new (m_fileMem) FatFile;
    if (m_fFile->openNext(dir->m_fFile, oflag)) {
      return true;
    }
    m_fFile = nullptr;
  } else if (dir->m_xFile) {
    m_xFile = new (m_fileMem) ExFatFile;
    if (m_xFile->openNext(dir->m_xFile, oflag)) {
      return true;
    }
    m_xFile = nullptr;
  }
  return false;
}
//-----------------------------------------------------------------------------
bool FsFile::remove() {
  if (m_fFile) {
    if (m_fFile->remove()) {
      m_fFile = nullptr;
      return true;
    }
  } else if (m_xFile) {
    if (m_xFile->remove()) {
      m_xFile = nullptr;
      return true;
    }
  }
  return false;
}
//-----------------------------------------------------------------------------
bool FsFile::rmdir() {
  if (m_fFile) {
    if (m_fFile->rmdir()) {
      m_fFile = nullptr;
      return true;
    }
  } else if (m_xFile) {
    if (m_xFile->rmdir()) {
      m_xFile = nullptr;
      return true;
    }
  }
  return false;
}
