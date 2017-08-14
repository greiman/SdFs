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
#ifndef FsVolume_h
#define FsVolume_h
/**
 * \file
 * \brief FsVolume include file.
 */
#include "common/FsNew.h"
#include "FatLib/FatLib.h"
#include "ExFatLib/ExFatLib.h"

class FsFile;
/**
 * \class FsVolume
 * \brief FsVolume class.
 */
class FsVolume {
 public:
  FsVolume() : m_fVol(nullptr), m_xVol(nullptr) {}

  ~FsVolume() {end();}

  /**
   * Initialize an FatVolume object.
   * \param[in] blockDev Device block driver.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool begin(BlockDevice* blockDev);
  /** \return current working volume. */
  static FsVolume* cwv() {return m_cwv;}
  /** Change global working volume to this volume. */
  void chvol() {m_cwv = this;}
  /** \return The total number of clusters in the volume. */
  uint32_t clusterCount() {
    return m_fVol ? m_fVol->clusterCount() :
           m_xVol ? m_xVol->clusterCount() : 0;
  }
  /** \return The logical sector number for the start of file data. */
  uint32_t dataStartSector() const {
    return m_fVol ? m_fVol->dataStartSector() :
           m_xVol ? m_xVol->clusterHeapStartSector() : 0;
  }
  /** \return The logical sector number for the start of the first FAT. */
  uint32_t fatStartSector() const {
    return m_fVol ? m_fVol->fatStartSector() :
           m_xVol ? m_xVol->fatStartSector() : 0;
  }
  /** \return the free cluster count. */
  uint32_t freeClusterCount() {
    return m_fVol ? m_fVol->freeClusterCount() :
           m_xVol ? m_xVol->freeClusterCount() : 0;
  }
  /** \return The volume's cluster size in sectors. */
  uint32_t sectorsPerCluster() const {
    return m_fVol ? m_fVol->sectorsPerCluster() :
           m_xVol ? m_xVol->sectorsPerCluster() : 0;
  }

  /** free dynamic memory and end access to volume */
  void end() {
    m_fVol = nullptr;
    m_xVol = nullptr;
  }
  /** Test for the existence of a file in a directory
   *
   * \param[in] path Path of the file to be tested for.
   *
   * \return true if the file exists else false.
   */
  bool exists(const char* path) {
    return m_fVol ? m_fVol->exists(path) :
           m_xVol ? m_xVol->exists(path) : false;
  }
  /** \return Partition type. */
  uint8_t fatType() const {
    return m_fVol ? m_fVol->fatType() :
           m_xVol ? m_xVol->fatType() : 0;
  }
  /** List directory contents.
   *
   * \param[in] pr Print object.
   */
  void ls(print_t* pr) {
    if (m_fVol) m_fVol->ls(pr);
    if (m_xVol) m_xVol->ls(pr);
  }
  /** List directory contents.
   *
   * \param[in] pr Print object.
   * \param[in] flags The inclusive OR of
   *
   * LS_DATE - %Print file modification date
   *
   * LS_SIZE - %Print file size.
   *
   * LS_R - Recursive list of subdirectories.
   */
  void ls(print_t* pr, uint8_t flags) {
    if (m_fVol) m_fVol->ls(pr, flags);
    if (m_xVol) m_xVol->ls(pr, flags);
  }
  /** List the directory contents of a directory.
   *
   * \param[in] pr Print stream for list.
   *
   * \param[in] path directory to list.
   *
   * \param[in] flags The inclusive OR of
   *
   * LS_DATE - %Print file modification date
   *
   * LS_SIZE - %Print file size.
   *
   * LS_R - Recursive list of subdirectories.
   */
  void ls(print_t* pr, const char* path, uint8_t flags);
   /** Make a subdirectory in the volume root directory.
   *
   * \param[in] path A path with a valid 8.3 DOS name for the subdirectory.
   *
   * \param[in] pFlag Create missing parent directories if true.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool mkdir(const char *path, bool pFlag = true) {
    return m_fVol ? m_fVol->mkdir(path, pFlag) :
           m_xVol ? m_xVol->mkdir(path, pFlag) : false;
  }

  /** open a file
   *
   * \param[in] path location of file to be opened.
   * \param[in] oflag open mode flags.
   * \return a File object.
   */
  FsFile open(const char* path, uint8_t oflag = O_READ);

  /** Remove a file from the volume root directory.
  *
  * \param[in] path A path with a valid 8.3 DOS name for the file.
  *
  * \return The value true is returned for success and
  * the value false is returned for failure.
  */
  bool remove(const char *path) {
    return m_fVol ? m_fVol->remove(path) :
           m_xVol ? m_xVol->remove(path) : false;
  }

  /** Rename a file or subdirectory.
   *
   * \param[in] oldPath Path name to the file or subdirectory to be renamed.
   *
   * \param[in] newPath New path name of the file or subdirectory.
   *
   * The \a newPath object must not exist before the rename call.
   *
   * The file to be renamed must not be open.  The directory entry may be
   * moved and file system corruption could occur if the file is accessed by
   * a file object that was opened before the rename() call.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool rename(const char *oldPath, const char *newPath) {
    return m_fVol ? m_fVol->rename(oldPath, newPath) :
           m_xVol ? m_xVol->rename(oldPath, newPath) : false;
  }
  /** Remove a subdirectory from the volume's root directory.
   *
   * \param[in] path A path with a valid 8.3 DOS name for the subdirectory.
   *
   * The subdirectory file will be removed only if it is empty.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool rmdir(const char *path) {
    return m_fVol ? m_fVol->rmdir(path) :
           m_xVol ? m_xVol->rmdir(path) : false;
  }

#if ENABLE_ARDUINO_FEATURES
  /** Test for the existence of a file in a directory
   *
   * \param[in] path Path of the file to be tested for.
   *
   * \return true if the file exists else false.
   */
  bool exists(const String &path) {
    return exists(path.c_str());
  }

  /** List directory contents. */
  void ls() {
    ls(&Serial);
  }
  /** List directory contents.
   *
   * \param[in] flags The inclusive OR of
   *
   * LS_DATE - %Print file modification date
   *
   * LS_SIZE - %Print file size.
   *
   * LS_R - Recursive list of subdirectories.
   */
  void ls(uint8_t flags) {
    ls(&Serial, flags);
  }
  /** List the directory contents of a directory to Serial.
   *
   * \param[in] path directory to list.
   *
   * \param[in] flags The inclusive OR of
   *
   * LS_DATE - %Print file modification date
   *
   * LS_SIZE - %Print file size.
   *
   * LS_R - Recursive list of subdirectories.
   */
  void ls(const char* path, uint8_t flags = 0) {
    ls(&Serial, path, flags);
  }
  /** Make a subdirectory in the volume root directory.
   *
   * \param[in] path A path with a valid 8.3 DOS name for the subdirectory.
   *
   * \param[in] pFlag Create missing parent directories if true.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool mkdir(const String &path, bool pFlag = true) {
    return mkdir(path.c_str(), pFlag);
  }
  /** open a file
   *
   * \param[in] path location of file to be opened.
   * \param[in] oflag open mode flags.
   * \return a File object.
   */
  FsFile open(const String &path, uint8_t oflag = O_READ);
  /** Remove a file from the volume root directory.
  *
  * \param[in] path A path with a valid 8.3 DOS name for the file.
  *
  * \return The value true is returned for success and
  * the value false is returned for failure.
  */
  bool remove(const String &path) {
    return remove(path.c_str());
  }
  /** Remove a subdirectory from the volume's root directory.
   *
   * \param[in] path A path with a valid 8.3 DOS name for the subdirectory.
   *
   * The subdirectory file will be removed only if it is empty.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool rmdir(const String &path) {
    return rmdir(path.c_str());
  }
#endif  // ENABLE_ARDUINO_FEATURES

 private:
  static FsVolume* m_cwv;
  friend class FsFile;
  FsVolume(const FsVolume& from);
  FsVolume& operator=(const FsVolume& from);

  newalign_t   m_volMem[FS_ALIGN_DIM(ExFatVolume, FatVolume)];
  FatVolume*   m_fVol;
  ExFatVolume* m_xVol;
  BlockDevice* m_blockDev;
};
#endif  // FsVolume_h
