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
#ifndef FsNew_h
#define FsNew_h
#include <stddef.h>
#include <stdint.h>

/** 32-bit alignment */
typedef uint32_t newalign_t;

/** Size required for exFAT or FAT class. */
#define FS_SIZE(etype, ftype) \
  (sizeof(ftype) < sizeof(etype) ? sizeof(etype) : sizeof(ftype))

/** Dimension of aligned area. */
#define NEW_ALIGN_DIM(n) \
  (((size_t)(n) + sizeof(newalign_t) - 1U)/sizeof(newalign_t))

/** Dimension of aligned area for etype or ftype class. */
#define FS_ALIGN_DIM(etype, ftype) NEW_ALIGN_DIM(FS_SIZE(etype, ftype))

/** Custom new placement operator */
void* operator new(size_t size, newalign_t* ptr);
#endif  // FsNew_h
