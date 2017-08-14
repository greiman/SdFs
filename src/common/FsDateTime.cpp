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
#include "SysCall.h"
#include "FsDateTime.h"
#include "FmtNumber.h"
//-------------------------------------------------------------------------------
/** Date time callback. */
void (*FsDateTime::callback)(uint16_t* date, uint16_t* time) = nullptr;
//-------------------------------------------------------------------------------
static char* fsFmtField(char* str, uint16_t n, char sep) {
  if (sep) {
    *--str = sep;
  }
  str = fmtBase10(str, n);
  if (n < 10) {
    *--str = '0';
  }
  return str;
}
//------------------------------------------------------------------------------
char* fsFmtDate(char* str, uint16_t date) {
  str = fsFmtField(str, date & 31, 0);
  date >>= 5;
  str = fsFmtField(str, date & 15, '-');
  date >>= 4;
  return fsFmtField(str, 1980 + date, '-');
}
//------------------------------------------------------------------------------
char* fsFmtTime(char* str, uint16_t time) {
  time >>= 5;
  str = fsFmtField(str, time & 63, 0);
  return fsFmtField(str, time >> 6, ':');
}
//------------------------------------------------------------------------------
char* fsFmtTime(char* str, uint16_t time, uint8_t sec100) {
  str = fsFmtField(str, sec100%100, 0);
  str = fsFmtField(str, 2*(time & 31) + sec100/100, '.');
  *--str = ':';
  return fsFmtTime(str, time);
}
//------------------------------------------------------------------------------
char* fsFmtTimeZone(char* str, int8_t tz) {
  char sign;
  if (tz & 0X80) {
    if (tz & 0X40) {
      sign = '-';
      tz = -tz;
    } else {
      sign = '+';
      tz &= 0X7F;
    }
    if (tz) {
      str = fsFmtField(str, 15*(tz%4), 0);
      str = fsFmtField(str, tz/4, ':');
      *--str = sign;
    }
    *--str = 'C';
    *--str = 'T';
    *--str = 'U';
  }
  return str;
}
//------------------------------------------------------------------------------
size_t fsPrintDate(print_t* pr, uint16_t date) {
  // Allow YYYY-MM-DD
  char buf[sizeof("YYYY-MM-DD") -1];
  char* str = buf + sizeof(buf);
  str = fsFmtDate(str, date);
  return pr->write(str, buf + sizeof(buf) - str);
}
//------------------------------------------------------------------------------
size_t fsPrintDateTime(print_t* pr, uint16_t date, uint16_t time) {
  // Allow YYYY-MM-DD hh:mm
  char buf[sizeof("YYYY-MM-DD hh:mm") -1];
  char* str = buf + sizeof(buf);
  if (date) {
    str = fsFmtTime(str, time);
    *--str = ' ';
    str = fsFmtDate(str, date);
  } else {
    do {
      *--str = ' ';
    } while (str > buf);
  }
  return pr->write(str, buf + sizeof(buf) - str);
}
//------------------------------------------------------------------------------
size_t fsPrintDateTime(print_t* pr, uint32_t dateTime) {
  return fsPrintDateTime(pr, dateTime >> 16, dateTime & 0XFFFF);
}
//------------------------------------------------------------------------------
size_t fsPrintDateTime(print_t* pr,
                       uint32_t dateTime, uint8_t s100, int8_t tz) {
  // Allow YYYY-MM-DD hh:mm:ss.ss UTC+hh:mm
  char buf[sizeof("YYYY-MM-DD hh:mm:ss.ss UTC+hh:mm") -1];
  char* str = buf + sizeof(buf);
  if (tz) {
    str = fsFmtTimeZone(str, tz);
    *--str = ' ';
  }
  str = fsFmtTime(str, (uint16_t)dateTime, s100);
  *--str = ' ';
  str = fsFmtDate(str, (uint16_t)(dateTime >> 16));
  return pr->write(str, buf + sizeof(buf) - str);
}
//------------------------------------------------------------------------------
size_t fsPrintTime(print_t* pr, uint16_t time) {
  // Allow hh:mm
  char buf[sizeof("hh:mm") -1];
  char* str = buf + sizeof(buf);
  str = fsFmtTime(str, time);
  return pr->write(str, buf + sizeof(buf) - str);
}
//------------------------------------------------------------------------------
size_t fsPrintTime(print_t* pr, uint16_t time, uint8_t sec100) {
  // Allow hh:mm:ss.ss
  char buf[sizeof("hh:mm:ss.ss") -1];
  char* str = buf + sizeof(buf);
  str = fsFmtTime(str, time, sec100);
  return pr->write(str, buf + sizeof(buf) - str);
}
//------------------------------------------------------------------------------
size_t fsPrintTimeZone(print_t* pr, int8_t tz) {
  // Allow UTC+hh:mm
  char buf[sizeof("UTC+hh:mm") -1];
  char* str = buf + sizeof(buf);
  str = fsFmtTimeZone(str, tz);
  return pr->write(str, buf + sizeof(buf) - str);
}
