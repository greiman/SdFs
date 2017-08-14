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
#ifndef DebugMacros_h
#define DebugMacros_h
#include "FsConfig.h"
#define USE_DBG_MACROS 0

#if USE_DBG_MACROS

static void dbgPrint(const __FlashStringHelper* file, uint16_t line) {
  Serial.print(F("DBG_FAIL: "));
  Serial.print(file);
  Serial.write(':');
  Serial.println(line);
}

#define DBG_PRINT_IF(b) if (b) {Serial.print(F(__FILE__));\
                        Serial.println(__LINE__);}
#define DBG_HALT_IF(b) if (b) { Serial.print(F("DBG_HALT "));\
                       Serial.print(F(__FILE__)); Serial.println(__LINE__);\
                       while (true) {}}
#define DBG_FAIL_MACRO dbgPrint(F(__FILE__), __LINE__);
#else  // USE_DBG_MACROS
#define DBG_FAIL_MACRO
#define DBG_PRINT_IF(b)
#define DBG_HALT_IF(b)
#endif  // USE_DBG_MACROS
#endif  // DebugMacros_h
