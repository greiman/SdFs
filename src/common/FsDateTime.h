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
#ifndef FsDateTime_h
#define FsDateTime_h
#include <stdint.h>
#include "SysCall.h"
/** Backward compatible definition. */
#define FAT_DATE(y, m, d) FS_DATE(y, m, d)

/** Backward compatible definition. */
#define FAT_TIME(h, m, s) FS_TIME(h, m, s)
/** Date time callback */
namespace FsDateTime {
extern  void (*callback)(uint16_t* date, uint16_t* time);
}
/** date field for directory entry
 * \param[in] year [1980,2107]
 * \param[in] month [1,12]
 * \param[in] day [1,31]
 *
 * \return Packed date for directory entry.
 */
static inline uint16_t FS_DATE(uint16_t year, uint8_t month, uint8_t day) {
  year -= 1980;
  return year > 127 || month > 12 || day > 31 ? 0 :
         year << 9 | month << 5 | day;
}
static inline uint16_t FS_YEAR(uint16_t fatDate) {
  return 1980 + (fatDate >> 9);
}
/** month part of FAT directory date field
 * \param[in] fatDate Date in packed dir format.
 *
 * \return Extracted month [1,12]
 */
static inline uint8_t FS_MONTH(uint16_t fatDate) {
  return (fatDate >> 5) & 0XF;
}
/** day part of FAT directory date field
 * \param[in] fatDate Date in packed dir format.
 *
 * \return Extracted day [1,31]
 */
static inline uint8_t FS_DAY(uint16_t fatDate) {
  return fatDate & 0X1F;
}
/** time field for directory entry
 * \param[in] hour [0,23]
 * \param[in] minute [0,59]
 * \param[in] second [0,59]
 *
 * \return Packed time for directory entry.
 */
static inline uint16_t FS_TIME(uint8_t hour, uint8_t minute, uint8_t second) {
  return hour > 23 || minute > 59 || second > 59 ? 0 :
         hour << 11 | minute << 5 | second >> 1;
}
/** hour part of FAT directory time field
 * \param[in] fatTime Time in packed dir format.
 *
 * \return Extracted hour [0,23]
 */
static inline uint8_t FS_HOUR(uint16_t fatTime) {
  return fatTime >> 11;
}
/** minute part of FAT directory time field
 * \param[in] fatTime Time in packed dir format.
 *
 * \return Extracted minute [0,59]
 */
static inline uint8_t FS_MINUTE(uint16_t fatTime) {
  return (fatTime >> 5) & 0X3F;
}
/** second part of FAT directory time field
 * Note second/2 is stored in packed time.
 *
 * \param[in] fatTime Time in packed dir format.
 *
 * \return Extracted second [0,58]
 */
static inline uint8_t FS_SECOND(uint16_t fatTime) {
  return 2*(fatTime & 0X1F);
}
char* fsFmtDate(char* str, uint16_t date);
char* fsFmtTime(char* str, uint16_t time);
char* fsFmtTime(char* str, uint16_t time, uint8_t sec100);
char* fsFmtTimeZone(char* str, int8_t tz);
size_t fsPrintDate(print_t* pr, uint16_t date);
size_t fsPrintDateTime(print_t* pr, uint16_t date, uint16_t time);
size_t fsPrintDateTime(print_t* pr, uint32_t dateTime);
size_t fsPrintDateTime(print_t* pr, uint32_t dateTime, uint8_t s100, int8_t tz);
size_t fsPrintTime(print_t* pr, uint16_t time);
size_t fsPrintTime(print_t* pr, uint16_t time, uint8_t sec100);
size_t fsPrintTimeZone(print_t* pr, int8_t tz);
#endif  // FsDateTime_h
