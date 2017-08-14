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
#ifndef ArduinoFiles_h
#define ArduinoFiles_h
#include "FsConfig.h"
//------------------------------------------------------------------------------
/** Arduino SD.h style flag for open for read. */
#ifndef FILE_READ
#define FILE_READ O_READ
#endif  // FILE_READ
/** Arduino SD.h style flag for open at EOF for read/write with create. */
#ifndef FILE_WRITE
#define FILE_WRITE (O_RDWR | O_CREAT | O_AT_END)
#endif  // FILE_WRITE
//-----------------------------------------------------------------------------
/**
 * \class PrintFile
 * \brief PrintFile class.
 */
template<class BaseFile>
class PrintFile : public Print, public BaseFile {
 public:
  using BaseFile::clearWriteError;
  using BaseFile::getWriteError;
  using BaseFile::read;
  using BaseFile::write;
  using BaseFile::printf;
  /** Write a single byte.
   * \param[in] b byte to write.
   * \return one for success.
   */
  size_t write(uint8_t b) {
    return BaseFile::write(&b, 1);
  }
};
//-----------------------------------------------------------------------------
/**
 * \class StreamFile
 * \brief StreamFile class.
 */
template<class BaseFile, typename PosType>
class StreamFile : public Stream, public BaseFile {
 public:
  using BaseFile::clearWriteError;
  using BaseFile::getWriteError;
  using BaseFile::read;
  using BaseFile::write;
  using BaseFile::printf;
  using Stream::print;
  using Stream::println;

  StreamFile() {}

  /** \return number of bytes available from the current position to EOF
   *   or INT_MAX if more than INT_MAX bytes are available.
   */
  int available() {
    PosType n = BaseFile::available();
    return n > INT_MAX ? INT_MAX : n;
  }
  /** Ensure that any bytes written to the file are saved to the SD card. */
  void flush() {
    BaseFile::sync();
  }
  /** This function reports if the current file is a directory or not.
  * \return true if the file is a directory.
  */
  bool isDirectory() {
    return BaseFile::isDir();
  }
  /** No longer implemented due to Long File Names.
   *
   * Use getName(char* name, size_t size).
   * \return a pointer to replacement suggestion.
   */
  const char* name() const {
    return "use getName()";
  }
  /** Opens the next file or folder in a directory.
   *
   * \param[in] mode open mode flags.
   * \return a FatStream object.
   */
  StreamFile openNextFile(uint8_t mode = O_READ) {
    StreamFile tmpFile;
    tmpFile.openNext(this, mode);
    return tmpFile;
  }
  /** Return the next available byte without consuming it.
   *
   * \return The byte if no error and not at eof else -1;
   */
  int peek() {
    return BaseFile::peek();
  }
  /** \return the current file position. */
  PosType position() {
    return BaseFile::curPosition();
  }
  /** Read the next byte from a file.
   *
   * \return For success return the next byte in the file as an int.
   * If an error occurs or end of file is reached return -1.
   */
  int read() {
    return BaseFile::read();
  }
  /** Rewind a file if it is a directory */
  void rewindDirectory() {
    if (BaseFile::isDir()) {
      BaseFile::rewind();
    }
  }
  /**
   * Seek to a new position in the file, which must be between
   * 0 and the size of the file (inclusive).
   *
   * \param[in] pos the new file position.
   * \return true for success else false.
   */
  bool seek(PosType pos) {
    return BaseFile::seekSet(pos);
  }
  /** \return the file's size. */
  PosType size() {
    return BaseFile::fileSize();
  }
  /** Write a byte to a file. Required by the Arduino Print class.
   * \param[in] b the byte to be written.
   * Use getWriteError to check for errors.
   * \return 1 for success and 0 for failure.
   */
  size_t write(uint8_t b) {
    return BaseFile::write(b);
  }
};
#endif  // ArduinoFiles_h
