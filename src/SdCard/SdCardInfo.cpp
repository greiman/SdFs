/* Arduino SdCard Library
 * Copyright (C) 2011..2017 by William Greiman
 *
 * This file is part of the Arduino SdCard Library
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
 * along with the Arduino SdCard Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include "SdCardInfo.h"
#include "SysCall.h"
//------------------------------------------------------------------------------
#undef SD_CARD_ERROR
#define SD_CARD_ERROR(e, m) case SD_CARD_ERROR_##e: pr->print(F(#e)); break;
void printSdErrorSymbol(print_t* pr, uint8_t code) {
  pr->print(F("SD_CARD_ERROR_"));
  switch (code) {
    SD_ERROR_CODE_LIST
    default: pr->print(F("UNKNOWN"));
  }
}
//------------------------------------------------------------------------------
#undef SD_CARD_ERROR
#define SD_CARD_ERROR(e, m) case SD_CARD_ERROR_##e: pr->print(F(m)); break;
void printSdErrorText(print_t* pr, uint8_t code) {
  switch
  (code) {
    SD_ERROR_CODE_LIST
    default: pr->print(F("Unknown error"));
  }
}
