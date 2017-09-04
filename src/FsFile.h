/**
 * Copyright (c) 20011-2017 Bill Greiman
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
#ifndef FsFile_h
#define FsFile_h
/**
 * \file
 * \brief FsFile include file.
 */
#include "common/FsNew.h"
#include "FatLib/FatLib.h"
#include "ExFatLib/ExFatLib.h"
/**
 * \class FsFile
 * \brief FsFile class.
 */
#if ENABLE_ARDUINO_FEATURES
class FsFile : public Stream {
#else  // ENABLE_ARDUINO_FEATURES
class FsFile {
#endif  // ENABLE_ARDUINO_FEATURES
 public:
  FsFile() : m_fFile(nullptr), m_xFile(nullptr) {}

  ~FsFile() {close();}
  /** Copy constructor.
   *
   * \param[in] from Object used to initialize this instance.
   */
  FsFile(const FsFile& from);
  /** Copy assignment operator
   * \param[in] from Object used to initialize this instance.
   * \return assigned object.
   */
  FsFile& operator=(const FsFile& from);
  /** The parenthesis operator.
    *
    * \return true if a file is open.
    */
  operator bool() {return isOpen();}
  /** \return number of bytes available from the current position to EOF
   *   or INT_MAX if more than INT_MAX bytes are available.
   */
  int available() {
    return m_fFile ? m_fFile->available() :
           m_xFile ? m_xFile->available() : 0;
  }

  /** Set writeError to zero */
  void clearWriteError() {
    if (m_fFile) m_fFile->clearWriteError();
    if (m_xFile) m_xFile->clearWriteError();
  }
  /** Close a file and force cached data and directory information
   *  to be written to the storage device.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool close();
  /** \return The current position for a file or directory. */
  uint64_t curPosition() {
    return m_fFile ? m_fFile->curPosition() :
           m_xFile ? m_xFile->curPosition() : 0;
  }
  /** Test for the existence of a file in a directory
   *
   * \param[in] path Path of the file to be tested for.
   *
   * The calling instance must be an open directory file.
   *
   * dirFile.exists("TOFIND.TXT") searches for "TOFIND.TXT" in  the directory
   * dirFile.
   *
   * \return true if the file exists else false.
   */
  bool exists(const char* path) {
    return m_fFile ? m_fFile->exists(path) :
           m_xFile ? m_xFile->exists(path) : false;
  }
 /**
   * Get a string from a file.
   *
   * fgets() reads bytes from a file into the array pointed to by \a str, until
   * \a num - 1 bytes are read, or a delimiter is read and transferred to \a str,
   * or end-of-file is encountered. The string is then terminated
   * with a null byte.
   *
   * fgets() deletes CR, '\\r', from the string.  This insures only a '\\n'
   * terminates the string for Windows text files which use CRLF for newline.
   *
   * \param[out] str Pointer to the array where the string is stored.
   * \param[in] num Maximum number of characters to be read
   * (including the final null byte). Usually the length
   * of the array \a str is used.
   * \param[in] delim Optional set of delimiters. The default is "\n".
   *
   * \return For success fgets() returns the length of the string in \a str.
   * If no data is read, fgets() returns zero for EOF or -1 if an error occurred.
   */
  int fgets(char* str, int num, char* delim = nullptr) {
    return m_fFile ? m_fFile->fgets(str, num, delim) :
           m_xFile ? m_xFile->fgets(str, num, delim) : -1;
  }
  /** \return The total number of bytes in a file. */
  uint64_t fileSize() {
    return m_fFile ? m_fFile->fileSize() :
           m_xFile ? m_xFile->fileSize() : 0;
  }
  /** Ensure that any bytes written to the file are saved to the SD card. */
  void flush() {sync();}
  /**
   * Get a file's name followed by a zero byte.
   *
   * \param[out] name An array of characters for the file's name.
   * \param[in] len The size of the array in bytes. The array
   *             must be at least 13 bytes long.  The file's name will be
   *             truncated if the file's name is too long.
   * \return The length of the returned string.
   */
  size_t getName(char* name, size_t len) {
    *name = 0;
    return m_fFile ? m_fFile->getName(name, len) :
           m_xFile ? m_xFile->getName(name, len) : 0;
  }

  /** \return value of writeError */
  bool getWriteError() {
    return m_fFile ? m_fFile->getWriteError() :
           m_xFile ? m_xFile->getWriteError() : true;
  }
  /** \return True if this is a directory else false. */
  bool isDir() {
    return m_fFile ? m_fFile->isDir() :
           m_xFile ? m_xFile->isDir() : false;
  }
  /** This function reports if the current file is a directory or not.
   * \return true if the file is a directory.
   */
  bool isDirectory() {return isDir();}
  /** \return True if this is a hidden file else false. */
  bool isHidden() {
    return m_fFile ? m_fFile->isHidden() :
           m_xFile ? m_xFile->isHidden() : false;
  }
  /** \return True if this is an open file/directory else false. */
  bool isOpen() {return m_fFile || m_xFile;}
#if ENABLE_ARDUINO_FEATURES
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
  /** List directory contents. */
  void ls() {
    ls(&Serial);
  }
#endif  // ENABLE_ARDUINO_FEATURES
  /** List directory contents.
   *
   * \param[in] pr Print object.
   */
  void ls(print_t* pr) {
    if (m_fFile) m_fFile->ls(pr);
    if (m_xFile) m_xFile->ls(pr);
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
    if (m_fFile) m_fFile->ls(pr, flags);
    if (m_xFile) m_xFile->ls(pr, flags);
  }
  /** Make a new directory.
   *
   * \param[in] dir An open FatFile instance for the directory that will
   *                   contain the new directory.
   *
   * \param[in] path A path with a valid 8.3 DOS name for the new directory.
   *
   * \param[in] pFlag Create missing parent directories if true.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool mkdir(FsFile* dir, const char* path, bool pFlag = true);
  /** No longer implemented due to Long File Names.
   *
   * Use getName(char* name, size_t size).
   * \return a pointer to replacement suggestion.
   */
  const char* name() const {
    return "use getName()";
  }
  /** Open a file or directory by name.
   *
   * \param[in] dir An open file instance for the directory containing
   *                    the file to be opened.
   *
   * \param[in] path A path with a valid 8.3 DOS name for a file to be opened.
   *
   * \param[in] oflag Values for \a oflag are constructed by a
   *                  bitwise-inclusive OR of flags from the following list
   *
   * O_READ - Open for reading.
   *
   * O_RDONLY - Same as O_READ.
   *
   * O_WRITE - Open for writing.
   *
   * O_WRONLY - Same as O_WRITE.
   *
   * O_RDWR - Open for reading and writing.
   *
   * O_APPEND - If set, the file offset shall be set to the end of the
   * file prior to each write.
   *
   * O_AT_END - Set the initial position at the end of the file.
   *
   * O_CREAT - If the file exists, this flag has no effect except as noted
   * under O_EXCL below. Otherwise, the file shall be created
   *
   * O_EXCL - If O_CREAT and O_EXCL are set, open() shall fail if the file exists.
   *
   * O_TRUNC - If the file exists and is a regular file, and the file is
   * successfully opened and is not read only, its length shall be truncated to 0.
   *
   * WARNING: A given file must not be opened by more than one file object
   * or file corruption may occur.
   *
   * \note Directory files must be opened read only.  Write and truncation is
   * not allowed for directory files.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool open(FsFile* dir, const char* path, uint8_t oflag = O_READ);
  /** Open a file or directory by name.
   *
   * \param[in] vol Volume where the file is located.
   *
   * \param[in] path A path for a file to be opened.
   *
   * \param[in] oflag Values for \a oflag are constructed by a
   *                  bitwise-inclusive OR of open flags.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool open(FsVolume* vol, const char* path, uint8_t oflag);
  /** Open a file or directory by name.
   *
   * \param[in] path A path for a file to be opened.
   *
   * \param[in] oflag Values for \a oflag are constructed by a
   *                  bitwise-inclusive OR of open flags.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool open(const char* path, uint8_t oflag = O_READ) {
    return FsVolume::m_cwv && open(FsVolume::m_cwv, path, oflag);
  }
  /** Opens the next file or folder in a directory.
   * \param[in] dir directory containing files.
   * \param[in] oflag open flags.
   * \return a file object.
   */
  bool openNext(FsFile* dir, uint8_t oflag = O_READ);
  /** Return the next available byte without consuming it.
   *
   * \return The byte if no error and not at eof else -1;
   */
  int peek() {
    return m_fFile ? m_fFile->peek() :
           m_xFile ? m_xFile->peek() : -1;
  }
  /** Print a file's modify date and time
   *
   * \param[in] pr Print stream for output.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  size_t printModifyDateTime(print_t* pr) {
    return m_fFile ? m_fFile->printModifyDateTime(pr) :
           m_xFile ? m_xFile->printModifyDateTime(pr) : 0;
  }
  /** Print a file's name
   *
   * \param[in] pr Print stream for output.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  size_t printName(print_t* pr) {
    return m_fFile ? m_fFile->printName(pr) :
           m_xFile ? m_xFile->printName(pr) : 0;
  }
  /** Print a file's size.
   *
   * \param[in] pr Print stream for output.
   *
   * \return The number of characters printed is returned
   *         for success and zero is returned for failure.
   */
  size_t printFileSize(print_t* pr) {
    return m_fFile ? m_fFile->printFileSize(pr) :
           m_xFile ? m_xFile->printFileSize(pr) : 0;
  }
  /** Allocate contiguous clusters to an empty file.
   *
   * The file must be empty with no clusters allocated.
   *
   * The file will contain uninitialized data for FAT16/FAT32 files.
   * exFAT files will have zero validLength and dataLength will equal
   * the requested length.
   *
   * \param[in] length size of the file in bytes.
   * \return true for success else false.
   */
  bool preAllocate(uint64_t length) {
    if (m_fFile) {
      return length < (1ULL << 32) ? m_fFile->preAllocate(length) : false;
    }
    return m_xFile ? m_xFile->preAllocate(length) : false;
  }
  /** \return the current file position. */
  uint64_t position() {return curPosition();}
   /** Print a number followed by a field terminator.
   * \param[in] value The number to be printed.
   * \param[in] term The field terminator.  Use '\\n' for CR LF.
   * \param[in] prec Number of digits after decimal point.
   * \return The number of bytes written or -1 if an error occurs.
   */
  size_t printField(double value, char term, uint8_t prec = 2) {
    return m_fFile ? m_fFile->printField(value, term, prec) :
           m_xFile ? m_xFile->printField(value, term, prec) : 0;
  }
  /** Print a number followed by a field terminator.
   * \param[in] value The number to be printed.
   * \param[in] term The field terminator.  Use '\\n' for CR LF.
   * \param[in] prec Number of digits after decimal point.
   * \return The number of bytes written or -1 if an error occurs.
   */
  size_t printField(float value, char term, uint8_t prec = 2) {
     return printField(static_cast<double>(value), term, prec);
  }
  /** Print a number followed by a field terminator.
   * \param[in] value The number to be printed.
   * \param[in] term The field terminator.  Use '\\n' for CR LF.
   * \return The number of bytes written or -1 if an error occurs.
   */
  template<typename Type>
  size_t printField(Type value, char term) {
    return m_fFile ? m_fFile->printField(value, term) :
           m_xFile ? m_xFile->printField(value, term) : 0;
  }
  /** Read the next byte from a file.
   *
   * \return For success return the next byte in the file as an int.
   * If an error occurs or end of file is reached return -1.
   */
  int read() {
    uint8_t b;
    return read(&b, 1) == 1 ? b : -1;
  }
  /** Read data from a file starting at the current position.
   *
   * \param[out] buf Pointer to the location that will receive the data.
   *
   * \param[in] count Maximum number of bytes to read.
   *
   * \return For success read() returns the number of bytes read.
   * A value less than \a count, including zero, will be returned
   * if end of file is reached.
   * If an error occurs, read() returns -1.  Possible errors include
   * read() called before a file has been opened, corrupt file system
   * or an I/O error occurred.
   */
  int read(void* buf, size_t count) {
    return m_fFile ? m_fFile->read(buf, count) :
           m_xFile ? m_xFile->read(buf, count) : -1;
  }
  /** Remove a file.
   *
   * The directory entry and all data for the file are deleted.
   *
   * \note This function should not be used to delete the 8.3 version of a
   * file that has a long name. For example if a file has the long name
   * "New Text Document.txt" you should not delete the 8.3 name "NEWTEX~1.TXT".
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool remove();
   /** Remove a file.
   *
   * The directory entry and all data for the file are deleted.
   *
   * \param[in] path Path for the file to be removed.
   *
   * Example use: dirFile.remove(filenameToRemove);
   *
   * \note This function should not be used to delete the 8.3 version of a
   * file that has a long name. For example if a file has the long name
   * "New Text Document.txt" you should not delete the 8.3 name "NEWTEX~1.TXT".
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool remove(const char* path) {
    return m_fFile ? m_fFile->remove(path) :
           m_xFile ? m_xFile->remove(path) : false;
  }
  /** Rename a file or subdirectory.
   *
   * \param[in] newPath New path name for the file/directory.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool rename(const char* newPath) {
    return m_fFile ? m_fFile->rename(newPath) :
           m_xFile ? m_xFile->rename(newPath) : false;
  }
  /** Rename a file or subdirectory.
   *
   * \param[in] dirFile Directory for the new path.
   * \param[in] newPath New path name for the file/directory.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool rename(FsFile* dirFile, const char* newPath) {
    return m_fFile ? m_fFile->rename(dirFile->m_fFile, newPath) :
           m_xFile ? m_xFile->rename(dirFile->m_xFile, newPath) : false;
  }
  /** Set the file's current position to zero. */
  void rewind() {
    if (m_fFile) m_fFile->rewind();
    if (m_xFile) m_xFile->rewind();
  }
  /** Rewind a file if it is a directory */
  void rewindDirectory() {
    if (isDir()) rewind();
  }
  /** Remove a directory file.
   *
   * The directory file will be removed only if it is empty and is not the
   * root directory.  rmdir() follows DOS and Windows and ignores the
   * read-only attribute for the directory.
   *
   * \note This function should not be used to delete the 8.3 version of a
   * directory that has a long name. For example if a directory has the
   * long name "New folder" you should not delete the 8.3 name "NEWFOL~1".
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool rmdir();
  /** Seek to a new position in the file, which must be between
   * 0 and the size of the file (inclusive).
   *
   * \param[in] pos the new file position.
   * \return true for success else false.
   */
  bool seek(uint64_t pos) {return seekSet(pos);}
  /** Set the files position to current position + \a pos. See seekSet().
   * \param[in] offset The new position in bytes from the current position.
   * \return true for success or false for failure.
   */
  bool seekCur(int64_t offset) {
    return m_fFile ? m_fFile->seekCur(offset) :
           m_xFile ? m_xFile->seekCur(offset) : false;
  }
  /** Set the files position to end-of-file + \a offset. See seekSet().
   * Can't be used for directory files since file size is not defined.
   * \param[in] offset The new position in bytes from end-of-file.
   * \return true for success or false for failure.
   */
  bool seekEnd(int64_t offset = 0) {
    return m_fFile ? m_fFile->seekEnd(offset) :
           m_xFile ? m_xFile->seekEnd(offset) : false;
  }
  /** Sets a file's position.
   *
   * \param[in] pos The new position in bytes from the beginning of the file.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool seekSet(uint64_t pos) {
    return m_fFile ? m_fFile->seekSet(pos) :
           m_xFile ? m_xFile->seekSet(pos) : false;
  }
  /** \return the file's size. */
  uint64_t size() {return fileSize();}
  /** The sync() call causes all modified data and directory fields
   * to be written to the storage device.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool sync() {
    return m_fFile ? m_fFile->sync() :
           m_xFile ? m_xFile->sync() : false;
  }
  /** Truncate a file to the current position.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool truncate() {
    return m_fFile ? m_fFile->truncate() :
           m_xFile ? m_xFile->truncate() : false;
  }
  /** Truncate a file to a specified length.
   * The current file position will be set to end of file.
   *
   * \param[in] length The desired length for the file.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool truncate(uint64_t length) {
    return m_fFile ? m_fFile->truncate(length) :
           m_xFile ? m_xFile->truncate(length) : false;
  }
  /** Write a byte to a file. Required by the Arduino Print class.
   * \param[in] b the byte to be written.
   * Use getWriteError to check for errors.
   * \return 1 for success and 0 for failure.
   */
  size_t write(uint8_t b) {return write(&b, 1);}
  /** Write data to an open file.
   *
   * \note Data is moved to the cache but may not be written to the
   * storage device until sync() is called.
   *
   * \param[in] buf Pointer to the location of the data to be written.
   *
   * \param[in] count Number of bytes to write.
   *
   * \return For success write() returns the number of bytes written, always
   * \a nbyte.  If an error occurs, write() returns -1.  Possible errors
   * include write() is called before a file has been opened, write is called
   * for a read-only file, device is full, a corrupt file system or an
   * I/O error.
   */
  size_t write(const void* buf, size_t count) {
    return m_fFile ? m_fFile->write(buf, count) :
           m_xFile ? m_xFile->write(buf, count) : 0;
  }

 private:
  newalign_t m_fileMem[FS_ALIGN_DIM(ExFatFile, FatFile)];
  FatFile*   m_fFile;
  ExFatFile* m_xFile;
};
#endif  //  FsFile_h
