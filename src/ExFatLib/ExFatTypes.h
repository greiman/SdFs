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
#ifndef ExFatTypes_h
#define ExFatTypes_h
#include "ExFatConfig.h"

#if __cplusplus < 201103
#warning no char16_t
typedef uint16_t ExChar16_t;
//  #error C++11 Support required
#else
typedef char16_t ExChar16_t;
#endif

#if USE_UNICODE_NAMES
/** exFAT API character type */
typedef ExChar16_t ExChar_t;
#else  // USE_UNICODE_NAMES
/** exFAT API character type */
typedef char ExChar_t;
#endif  // USE_UNICODE_NAMES
/**
 * \struct DirPos_t
 * \brief Internal type for position in directory file.
 */
struct DirPos_t {
  /** current cluster */
  uint32_t cluster;
  /** offset */
  uint32_t position;
  /** directory is contiguous */
  bool     isContiguous;
};
#endif  // ExFatTypes_h
