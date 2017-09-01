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
#ifndef fstream_h
#define fstream_h
#include "iostream.h"
#include "../ExFatLib/ExFatLib.h"
#include "../FatLib/FatLib.h"
#include "FsVolume.h"
#include "FsFile.h"

//-----------------------------------------------------------------------------
/**
 * \class BaseStream
 * \brief base type for FAT and exFAT streams
 */
template<class BaseFile>
class BaseStream : protected BaseFile, virtual public ios {
 protected:
  /// @cond SHOW_PROTECTED
  //---------------------------------------------------------------------------
  int16_t getch() {
    uint8_t c;
    int8_t s = BaseFile::read(&c, 1);
    if (s != 1) {
      if (s < 0) {
        setstate(badbit);
      } else {
        setstate(eofbit);
      }
      return -1;
    }
    if (c != '\r' || (getmode() & ios::binary)) {
      return c;
    }
    s = BaseFile::read(&c, 1);
    if (s == 1 && c == '\n') {
      return c;
    }
    if (s == 1) {
      BaseFile::seekCur(-1);
    }
    return '\r';
  }
  bool getWriteError() {
    return m_writeError;
  }
  void clearWriteError() {
    m_writeError = false;
  }
  //---------------------------------------------------------------------------
  void open(const char* path, ios::openmode mode) {
    uint8_t flags;  //////////////////////////////////////////  fix flags  /////////////////////////////////
    switch (mode & (app | in | out | trunc)) {
    case app | in:
    case app | in | out:
      flags = O_RDWR | O_APPEND | O_CREAT;
      break;

    case app:
    case app | out:
      flags = O_WRITE | O_APPEND | O_CREAT;
      break;

    case in:
      flags = O_READ;
      break;

    case in | out:
      flags = O_RDWR | O_CREAT;
      break;

    case in | out | trunc:
      flags = O_RDWR | O_TRUNC | O_CREAT;
      break;

    case out:
    case out | trunc:
      flags = O_WRITE | O_TRUNC | O_CREAT;
      break;

    default:
      goto fail;
    }
    if (mode & ios::ate) {
      flags |= O_AT_END;
    }
    if (!BaseFile::open(path, flags)) {
      goto fail;
    }
    setmode(mode);
    clear();
    return;

  fail:
    BaseFile::close();
    setstate(failbit);
    return;
  }
  //---------------------------------------------------------------------------
  /** Internal do not use
   * \return mode
   */
  ios::openmode getmode() {
    return m_mode;
  }
  //---------------------------------------------------------------------------
  void putch(char c) {
    if (c == '\n' && !(getmode() & ios::binary)) {
      write('\r');
    }
    write(c);
    if (getWriteError()) {
      setstate(badbit);
    }
  }
  //---------------------------------------------------------------------------
  void putstr(const char* str) {
    size_t n = 0;
    while (1) {
      char c = str[n];
      if (c == '\0' || (c == '\n' && !(getmode() & ios::binary))) {
        if (n > 0) {
          write(str, n);
        }
        if (c == '\0') {
          break;
        }
        write('\r');
        str += n;
        n = 0;
      }
      n++;
    }
    if (getWriteError()) {
      setstate(badbit);
    }
  }
  //---------------------------------------------------------------------------
  /** Internal do not use
   * \param[in] off
   * \param[in] way
   */
  bool seekoff(off_type off, seekdir way) {
    pos_type pos;
    switch (way) {
    case beg:
      pos = off;
      break;

    case cur:
      pos = BaseFile::curPosition() + off;
      break;

    case end:
      pos = BaseFile::fileSize() + off;
      break;

    default:
      return false;
    }
    return seekpos(pos);
  }
  //---------------------------------------------------------------------------
  /** Internal do not use
   * \param[in] pos
   */
  bool seekpos(pos_type pos) {
    return BaseFile::seekSet(pos);
  }
  //---------------------------------------------------------------------------
  /** Internal do not use
   * \param[in] mode
   */
  void setmode(ios::openmode mode) {
    m_mode = mode;
  }
  //---------------------------------------------------------------------------
  int write(const void* buf, size_t n) {
    int rtn = BaseFile::write(buf, n);
    if (static_cast<int>(n) != rtn) {
      m_writeError = true;
    }
    return rtn;
  }
  //---------------------------------------------------------------------------
  void write(char c) {
    BaseFile::write(&c, 1);
  }
  /// @endcond

 private:
  bool m_writeError;
  ios::openmode m_mode;
};
//=============================================================================
/**
 * \class fstreamBase
 * \brief base type for FAT and exFAT fstreams
 */
template<class BaseFile>
class fstreamBase : public iostream, BaseStream<BaseFile>  {
 public:
  using iostream::peek;
#if DESTRUCTOR_CLOSES_FILE
  ~fstreamBase() {}
#endif  // DESTRUCTOR_CLOSES_FILE
  /** Clear state and writeError
   * \param[in] state new state for stream
   */
  void clear(iostate state = goodbit) {
    ios::clear(state);
    BaseStream<BaseFile>::clearWriteError();
  }
  /**  Close a file and force cached data and directory information
   *  to be written to the storage device.
   */
  void close() {
    BaseStream<BaseFile>::close();
  }
  /** Open a fstream
   * \param[in] path file to open
   * \param[in] mode open mode
   *
   * Valid open modes are (at end, ios::ate, and/or ios::binary may be added):
   *
   * ios::in - Open file for reading.
   *
   * ios::out or ios::out | ios::trunc - Truncate to 0 length, if existent,
   * or create a file for writing only.
   *
   * ios::app or ios::out | ios::app - Append; open or create file for
   * writing at end-of-file.
   *
   * ios::in | ios::out - Open file for update (reading and writing).
   *
   * ios::in | ios::out | ios::trunc - Truncate to zero length, if existent,
   * or create file for update.
   *
   * ios::in | ios::app or ios::in | ios::out | ios::app - Append; open or
   * create text file for update, writing at end of file.
   */
  void open(const char* path, openmode mode = in | out) {
    BaseStream<BaseFile>::open(path, mode);
  }
  /** \return True if stream is open else false. */
  bool is_open() {
    return BaseFile::isOpen();
  }

 protected:
  /// @cond SHOW_PROTECTED
  /** Internal - do not use
   * \return
   */
  int16_t getch() {
    return BaseStream<BaseFile>::getch();
  }
  /** Internal - do not use
  * \param[out] pos
  */
  void getpos(pos_t* pos) {
    BaseFile::fgetpos(pos);
  }
  /** Internal - do not use
   * \param[in] c
   */
  void putch(char c) {
    BaseStream<BaseFile>::putch(c);
  }
  /** Internal - do not use
   * \param[in] str
   */
  void putstr(const char *str) {
    BaseStream<BaseFile>::putstr(str);
  }
  /** Internal - do not use
   * \param[in] pos
   */
  bool seekoff(off_type off, seekdir way) {
    return BaseStream<BaseFile>::seekoff(off, way);
  }
  bool seekpos(pos_type pos) {
    return BaseStream<BaseFile>::seekpos(pos);
  }
  void setpos(pos_t* pos) {
    BaseFile::fsetpos(pos);
  }
  bool sync() {
    return BaseStream<BaseFile>::sync();
  }
  pos_type tellpos() {
    return BaseFile::curPosition();
  }
  /// @endcond
};
//==============================================================================
/**
 * \class ifstreamBase
 * \brief file input stream.
 */
//-----------------------------------------------------------------------------
template<class BaseFile>
class ifstreamBase : public istream, BaseStream<BaseFile>  {
 public:
  using istream::peek;
#if DESTRUCTOR_CLOSES_FILE
  ~ifstream() {}
#endif  // DESTRUCTOR_CLOSES_FILE
  /**  Close a file and force cached data and directory information
   *  to be written to the storage device.
   */
  void close() {
    BaseStream<BaseFile>::close();
  }
  /** \return True if stream is open else false. */
  bool is_open() {
    return BaseFile::isOpen();
  }
  /** Open an ifstream
   * \param[in] path file to open
   * \param[in] mode open mode
   *
   * \a mode See fstream::open() for valid modes.
   */
  void open(const char* path, openmode mode = in) {
    BaseStream<BaseFile>::open(path, mode | in);
  }

 protected:
  /// @cond SHOW_PROTECTED
  /** Internal - do not use
   * \return
   */
  int16_t getch() {
    return BaseStream<BaseFile>::getch();
  }
  /** Internal - do not use
   * \param[out] pos
   */
  void getpos(pos_t* pos) {
    BaseFile::fgetpos(pos);
  }
  /** Internal - do not use
   * \param[in] pos
   */
  bool seekoff(off_type off, seekdir way) {
    return BaseStream<BaseFile>::seekoff(off, way);
  }
  bool seekpos(pos_type pos) {
    return BaseStream<BaseFile>::seekpos(pos);
  }
  void setpos(pos_t* pos) {
    BaseFile::fsetpos(pos);
  }
  pos_type tellpos() {
    return BaseFile::curPosition();
  }
  /// @endcond
};
//==============================================================================
/**
 * \class ofstreamBase
 * \brief file output stream.
 */
//-----------------------------------------------------------------------------
template<class BaseFile>
class ofstreamBase : public ostream, BaseStream<BaseFile>  {
 public:
#if DESTRUCTOR_CLOSES_FILE
  ~ofstreamBase() {}
#endif  // DESTRUCTOR_CLOSES_FILE
  /** Clear state and writeError
   * \param[in] state new state for stream
   */
  void clear(iostate state = goodbit) {
    ios::clear(state);
    BaseStream<BaseFile>::clearWriteError();
  }
  /**  Close a file and force cached data and directory information
   *  to be written to the storage device.
   */
  void close() {
    BaseStream<BaseFile>::close();
  }
  /** Open an ofstream
   * \param[in] path file to open
   * \param[in] mode open mode
   *
   * \a mode See fstream::open() for valid modes.
   */
  void open(const char* path, openmode mode = out) {
    BaseStream<BaseFile>::open(path, mode | out);
  }
  /** \return True if stream is open else false. */
  bool is_open() {
    return BaseFile::isOpen();
  }

 protected:
  /// @cond SHOW_PROTECTED
  /**
   * Internal do not use
   * \param[in] c
   */
  void putch(char c) {
    BaseStream<BaseFile>::putch(c);
  }
  void putstr(const char* str) {
    BaseStream<BaseFile>::putstr(str);
  }
  bool seekoff(off_type off, seekdir way) {
    return BaseStream<BaseFile>::seekoff(off, way);
  }
  bool seekpos(pos_type pos) {
    return BaseStream<BaseFile>::seekpos(pos);
  }
  /**
   * Internal do not use
   * \param[in] b
   */
  bool sync() {
    return BaseStream<BaseFile>::sync();
  }
  pos_type tellpos() {
    return BaseFile::curPosition();
  }
  /// @endcond
};

//------------------------------------------------------------------------------
/**
 * \class estream
 * \brief exFAT file input/output stream.
 */
class estream : public fstreamBase<ExFatFile> {
 public:
  estream() {}
  /** Constructor with open
   * \param[in] path file to open
   * \param[in] mode open mode
   */
  explicit estream(const char* path, openmode mode = in | out) {
    open(path, mode);
  }
};
/**
 * \class iestream
 * \brief exFAT file input stream.
 */
class iestream : public ifstreamBase<ExFatFile> {
 public:
  iestream() {}
   /** Constructor with open
   * \param[in] path file to open
   * \param[in] mode open mode
   */
  explicit iestream(const char* path, openmode mode = in) {
    open(path, mode);
  }
};
/**
 * \class oestream
 * \brief exFAT file output stream.
 */
class oestream : public ofstreamBase<ExFatFile> {
 public:
  oestream() {}
  /** Constructor with open
   * \param[in] path file to open
   * \param[in] mode open mode
   */
  explicit oestream(const char* path, openmode mode = out) {
    open(path, mode);
  }
};
/**
 * \class fstream
 * \brief FAT file input/output stream.
 */
class fstream : public fstreamBase<FatFile> {
 public:
  fstream() {}
  /** Constructor with open
   * \param[in] path file to open
   * \param[in] mode open mode
   */
  explicit fstream(const char* path, openmode mode = in | out) {
    open(path, mode);
  }
};
/**
 * \class ifstream
 * \brief FAT file input stream.
 */
class ifstream : public ifstreamBase<FatFile> {
 public:
  ifstream() {}
  /** Constructor with open
   * \param[in] path file to open
   * \param[in] mode open mode
   */
  explicit ifstream(const char* path, openmode mode = in) {
    open(path, mode);
  }
};
/**
 * \class ofstream
 * \brief FAT file output stream.
 */
class ofstream : public ofstreamBase<FatFile> {
 public:
  ofstream() {}
  /** Constructor with open
   * \param[in] path file to open
   * \param[in] mode open mode
   */
  explicit ofstream(const char* path, openmode mode = out) {
    open(path, mode);
  }
};
#endif  // fstream_h
