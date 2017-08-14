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
#ifndef upcase_h
#define upcase_h
#include "ExFatFile.h"
bool exFatCmpName(const DirName_t* unicode,
                  const char* name, size_t offset, size_t n);
bool exFatCmpName(const DirName_t* unicode,
                  const ExChar16_t* name, size_t offset, size_t n);
uint16_t exFatHashName(const char* name, size_t n, uint16_t hash);
uint16_t exFatHashName(const ExChar16_t* name, size_t n, uint16_t hash);
uint16_t toUpcase(uint16_t chr);
uint32_t upcaseChecksum(uint16_t unicode, uint32_t checksum);
#endif  // upcase_h
