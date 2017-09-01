/**
 * Copyright (c) 20011..2017 Bill Greiman
 * This file is part of the SdFs library for SD memory cards.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
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
  /**
   * Set volume working directory to root.
   * \return true for success else false.
   */
  bool chdir() {
    return m_fVol ? m_fVol->chdir() :
           m_xVol ? m_xVol->chdir() : false;
  }
  /**
   * Set volume working directory.
   * \param[in] path Path for volume working directory.
   * \return true for success or false for failure.
   */
    bool chdir(const char* path) {
    return m_fVol ? m_fVol->chdir(path) :
           m_xVol ? m_xVol->chdir(path) : false;
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
  /** \return Partition type, FAT_TYPE_EXFAT, FAT_TYPE_FAT32,
   *          FAT_TYPE_FAT16, or zero for error.
   */
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
