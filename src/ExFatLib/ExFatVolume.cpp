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
#include "ExFatVolume.h"
ExFatVolume* ExFatVolume::m_cwv = nullptr;
//----------------------------------------------------------------------------
bool ExFatVolume::chdir(const char *path) {
  ExFatFile dir;
  if (!dir.open(vwd(), path, O_READ)) {
    goto fail;
  }
  if (!dir.isDir()) {
    goto fail;
  }
  m_vwd = dir;
  return true;

fail:
  return false;
}
