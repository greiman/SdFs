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
#ifndef FmtNumber_h
#define FmtNumber_h
#include <math.h>
#include <stdint.h>
#include <stddef.h>
inline bool isDigit(char c) {
  return '0' <= (c) && (c) <= '9';
}
inline bool isSpace(char c) {
  return (c) == ' ' || (0X9 <= (c) && (c) <= 0XD);
}
char* fmtBase10(char* str, uint16_t n);
char* fmtBase10(char* str, uint32_t n);
char* fmtDouble(char *str, double d, uint8_t prec, bool altFmt);
char* fmtDouble(char* str, double d, uint8_t prec, bool altFmt, char expChar);
char* fmtSigned(char* str, int32_t n, uint8_t base, bool caps);
char* fmtUnsigned(char* str, uint32_t n, uint8_t base, bool caps);
#endif  // FmtNumber_h
