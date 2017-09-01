/**
 * Copyright (c) 20011-2017 Bill Greiman
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
