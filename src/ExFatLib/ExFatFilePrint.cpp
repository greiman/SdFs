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
#include "ExFatFile.h"
#include "upcase.h"
#include "../common/DebugMacros.h"
#include "../common/PrintTemplates.h"
//-----------------------------------------------------------------------------
size_t ExFatFile::printFileSize(print_t* pr) {
  uint64_t n = m_validLength;
  char buf[21];
  char *str = &buf[sizeof(buf) - 1];
  char *bgn = str - 12;
  *str = '\0';
  do {
    uint64_t m = n;
    n /= 10;
    *--str = m - 10*n + '0';
  } while (n);
  while (str > bgn) {
    *--str = ' ';
  }
  return pr->write(str);
}
//-----------------------------------------------------------------------------
size_t ExFatFile::printCreateDateTime(print_t* pr) {
  DirFile_t* df = reinterpret_cast<DirFile_t*>
                 (m_part->dirCache(&m_dirPos, FsCache::CACHE_FOR_READ));
  if (!df) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  return fsPrintDateTime(pr, getLe32(df->createTime));
fail:
  return 0;
}
//-----------------------------------------------------------------------------
size_t ExFatFile::printModifyDateTime(print_t* pr) {
  DirFile_t* df = reinterpret_cast<DirFile_t*>
                 (m_part->dirCache(&m_dirPos, FsCache::CACHE_FOR_READ));
  if (!df) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  return fsPrintDateTime(pr, getLe32(df->modifyTime));
fail:
  return 0;
}
//------------------------------------------------------------------------------
void ExFatFile::ls(print_t* pr) {
  ExFatFile file;
  rewind();
  while (file.openNext(this, O_READ)) {
    if (!file.isHidden()) {
      file.printName(pr);
      if (file.isDir()) {
        pr->write('/');
      }
      pr->write('\r');
      pr->write('\n');
    }
    file.close();
  }
}
//------------------------------------------------------------------------------
void ExFatFile::ls(print_t* pr, uint8_t flags, uint8_t indent) {
  ExFatFile file;
  rewind();
  while (file.openNext(this, O_READ)) {
    // indent for dir level
    if (!file.isHidden() || (flags & LS_A)) {
      for (uint8_t i = 0; i < indent; i++) {
        pr->write(' ');
      }
      if (flags & LS_DATE) {
        file.printModifyDateTime(pr);
        pr->write(' ');
      }
      if (flags & LS_SIZE) {
        file.printFileSize(pr);
        pr->write(' ');
      }
      file.printName(pr);
      if (file.isDir()) {
        pr->write('/');
      }
      pr->write('\r');
      pr->write('\n');
      if ((flags & LS_R) && file.isDir()) {
        file.ls(pr, flags, indent + 2);
      }
    }
    file.close();
  }
}
//------------------------------------------------------------------------------
int ExFatFile::printf(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  return vfprintf(this, fmt, ap);
}
//------------------------------------------------------------------------------
int ExFatFile::mprintf(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  return vmprintf(this, fmt, ap);
}
#if ENABLE_ARDUINO_FEATURES
//------------------------------------------------------------------------------
int ExFatFile::mprintf(const __FlashStringHelper *ifsh, ...) {
  va_list ap;
  va_start(ap, ifsh);
  return vmprintf(this, ifsh, ap);
}
#endif  // ENABLE_ARDUINO_FEATURES
//------------------------------------------------------------------------------
/** Template for ExFatFile::printField() */
template <typename Type>
static int printFieldT(ExFatFile* file, char sign, Type value, char term) {
  char buf[3*sizeof(Type) + 3];
  char* str = buf + sizeof(buf);

  if (term) {
    *--str = term;
    if (term == '\n') {
      *--str = '\r';
    }
  }
  str = fmtBase10(str, value);
  if (sign) {
    *--str = sign;
  }
  return file->write(str, &buf[sizeof(buf)] - str);
}
//------------------------------------------------------------------------------
int ExFatFile::printField(uint16_t value, char term) {
  return printFieldT(this, 0, value, term);
}
//------------------------------------------------------------------------------
int ExFatFile::printField(int16_t value, char term) {
  char sign = 0;
  if (value < 0) {
    sign = '-';
    value = -value;
  }
  return printFieldT(this, sign, (uint16_t)value, term);
}
//------------------------------------------------------------------------------
int ExFatFile::printField(uint32_t value, char term) {
  return printFieldT(this, 0, value, term);
}
//------------------------------------------------------------------------------
int ExFatFile::printField(int32_t value, char term) {
  char sign = 0;
  if (value < 0) {
    sign = '-';
    value = -value;
  }
  return printFieldT(this, sign, (uint32_t)value, term);
}
