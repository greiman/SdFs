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
