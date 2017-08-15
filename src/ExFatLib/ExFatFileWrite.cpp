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
#include "../common/DebugMacros.h"
#include "ExFatFile.h"
#include "upcase.h"
//=============================================================================
#if READ_ONLY
bool ExFatFile::sync() {
  return false;
}
bool ExFatFile::mkdir(ExFatFile* parent, const ExChar_t* path, bool pFlag) {
  (void) parent;
  (void)path;
  (void)pFlag;
  return false;
}
bool ExFatFile::preAllocate(uint64_t length) {
  (void)length;
  return false;
}
bool ExFatFile::rename(ExFatFile* dirFile, const ExChar_t* newPath) {
  (void)dirFile;
  (void)newPath;
  return false;
}
bool ExFatFile::truncate() {
  return false;
}

size_t ExFatFile::write(const void* buf, size_t nbyte) {
  (void)buf;
  (void)nbyte;
  return false;
}
//=============================================================================
#else  // READ_ONLY
//-----------------------------------------------------------------------------
static uint16_t exFatDirChecksum(const uint8_t* data, uint16_t checksum) {
  bool skip = data[0] == EXFAT_TYPE_FILE;
  for (size_t i = 0; i < 32; i += i == 1 && skip ? 3 : 1) {
    checksum = ((checksum << 15) | (checksum >> 1)) + data[i];
  }
  return checksum;
}
//-----------------------------------------------------------------------------
bool ExFatFile::addCluster() {
  uint32_t find = m_part->bitmapFind(m_curCluster + 1, 1);
  if (find < 2) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (!m_part->bitmapModify(find, 1, 1)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (m_curCluster == 0) {
    m_flags |= FILE_FLAG_CONTIGUOUS;
    goto done;
  }
  if (isContiguous()) {
    if (find == (m_curCluster + 1)) {
      goto done;
    }
    m_flags &= ~FILE_FLAG_CONTIGUOUS;
    uint32_t cluster = m_firstCluster;

    while (cluster < m_curCluster) {
      if (!m_part->fatPut(cluster, cluster + 1)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
    }
  }
  if (!m_part->fatPut(find, EXFAT_EOC)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (m_curCluster) {
    if (!m_part->fatPut(m_curCluster, find)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  }

 done:
  m_curCluster = find;
  return true;

 fail:
  return false;
}
//-----------------------------------------------------------------------------
bool ExFatFile::addDirCluster() {
  uint32_t sector;
  uint32_t dl = isRoot() ? m_part->rootLength() : m_dataLength;
  uint8_t* cache;
  dl += m_part->bytesPerCluster();
  if (dl >= 0X4000000) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (!addCluster()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  cache =  m_part->cacheClear();
  if (!cache) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  memset(cache, 0, m_part->bytesPerSector());
  sector = m_part->clusterStartSector(m_curCluster);
  for (uint32_t i = 0; i < m_part->sectorsPerCluster(); i++) {
    if (!m_part->writeSector(sector + i, cache)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  }
  if (!isRoot()) {
    m_flags |= FILE_FLAG_DIR_DIRTY;
    m_dataLength  += m_part->bytesPerCluster();
    m_validLength += m_part->bytesPerCluster();
  }
  return sync();

 fail:
  return false;
}
//------------------------------------------------------------------------------
bool ExFatFile::mkdir(ExFatFile* parent, const ExChar_t* path, bool pFlag) {
  ExName_t fname;
  ExFatFile tmpDir;

  if (isOpen() || !parent->isDir()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (isDirSeparator(*path)) {
    while (isDirSeparator(*path)) {
      path++;
    }
    if (!tmpDir.openRoot(parent->m_part)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    parent = &tmpDir;
  }
  while (1) {
    if (!parsePathName(path, &fname, &path)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    if (!*path) {
      break;
    }
    if (!open(parent, &fname, O_READ)) {
      if (!pFlag || !mkdir(parent, &fname)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
    }
    tmpDir = *this;
    parent = &tmpDir;
    close();
  }
  return mkdir(parent, &fname);

fail:
  return false;
}
//------------------------------------------------------------------------------
bool ExFatFile::mkdir(ExFatFile* parent, ExName_t* fname) {
  if (!parent->isDir()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // create a normal file
  if (!open(parent, fname, O_CREAT | O_EXCL | O_RDWR)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // convert file to directory

  m_attributes = FILE_ATTR_SUBDIR;

  // allocate and zero first cluster
  if (!addDirCluster()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  m_firstCluster = m_curCluster;

  // Set to start of dir
  rewind();
  m_flags = O_READ | FILE_FLAG_DIR_DIRTY;
  return sync();

fail:
  return false;
}
//------------------------------------------------------------------------------
bool ExFatFile::preAllocate(uint64_t length) {
  uint32_t find;
  uint32_t need;
  if (!length || !isFile() || !(m_flags & O_WRITE) || m_firstCluster) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  need = 1 + ((length - 1) >> m_part->bytesPerClusterShift());
  find = m_part->bitmapFind(0, need);
  if (find < 2) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (!m_part->bitmapModify(find, need, 1)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  m_dataLength = length;
  m_firstCluster = find;
  m_flags |= FILE_FLAG_DIR_DIRTY | FILE_FLAG_CONTIGUOUS;
  if (!sync()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  return true;

 fail:
  return false;
}
//------------------------------------------------------------------------------
bool ExFatFile::remove() {
  DirPos_t pos = m_dirPos;
  uint8_t* cache;
  if (!isFile() || !(m_flags & O_WRITE)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // Free any clusters.
  if (m_firstCluster) {
    if (isContiguous()) {
      uint32_t nc = 1 + ((m_dataLength - 1) >> m_part->bytesPerClusterShift());
      if (!m_part->bitmapModify(m_firstCluster, nc, 0)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
    } else {
      if (!m_part->freeChain(m_firstCluster)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
    }
  }

  for (uint8_t i = 0; i <= m_setCount; i++) {
    if (i && m_part->dirSeek(&pos, 32) != 1) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    cache = m_part->dirCache(&pos, FsCache::CACHE_FOR_WRITE);
    if (!cache) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    // Mark entry not used.
    cache[0] &= 0x7F;
  }
  // Set this file closed.
  m_attributes = FILE_ATTR_CLOSED;

  // Write entry to device.
  return m_part->cacheSync();

 fail:
  return false;
}
//------------------------------------------------------------------------------
bool ExFatFile::rename(ExFatFile* dirFile, const ExChar_t* newPath) {
  ExFatFile file;
  ExFatFile oldFile;

  // Must be an open file or subdirectory.
  if (!(isFile() || isSubDir())) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // Can't move file to new volume.
  if (m_part != dirFile->m_part) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (!file.open(dirFile, newPath, O_CREAT | O_EXCL | O_WRITE)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  oldFile = *this;
  m_dirPos = file.m_dirPos;
  m_setCount = file.m_setCount;
  m_flags |= FILE_FLAG_DIR_DIRTY;
  if (!sync()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // Remove old directory entry;
  oldFile.m_firstCluster = 0;
  oldFile.m_flags = O_WRITE;
  oldFile.m_attributes = FILE_ATTR_FILE;
  return oldFile.remove();

fail:
  return false;
}
//------------------------------------------------------------------------------
bool ExFatFile::rmdir() {
  int n;
  uint8_t dir[32];
  // must be open subdirectory
  if (!isSubDir()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  rewind();

  // make sure directory is empty
  while (1) {
    n = read(dir, 32);
    if (n == 0) {
      break;
    }
    if (n != 32 || dir[0] & 0X80) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    if (dir[0] == 0) {
      break;
    }
  }
  // convert empty directory to normal file for remove
  m_attributes = FILE_ATTR_FILE;
  m_flags |= O_WRITE;
  return remove();

fail:
  return false;
}
//-----------------------------------------------------------------------------
bool ExFatFile::sync() {
  if (!isOpen()) {
    return true;
  }
  if (m_flags & FILE_FLAG_DIR_DIRTY) {
    // clear directory dirty
    m_flags &= ~FILE_FLAG_DIR_DIRTY;
    return syncDir();
  }
  if (!m_part->cacheSync()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  return true;

 fail:
  m_error |= WRITE_ERROR;
  return false;
}
//-----------------------------------------------------------------------------
bool ExFatFile::syncDir() {
  DirFile_t* df;
  DirStream_t* ds;
  uint8_t* cache;
  uint16_t checksum = 0;
  uint8_t setCount = 0;

  DirPos_t pos = m_dirPos;

  for (uint8_t i = 0;; i++) {
    cache = m_part->dirCache(&pos, FsCache::CACHE_FOR_READ);
    if (!cache) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    switch (cache[0]) {
      case EXFAT_TYPE_FILE:
        df = reinterpret_cast<DirFile_t*>(cache);
        setCount = df->setCount;
        setLe16(df->attributes, m_attributes & FILE_ATTR_COPY);
        if (FsDateTime::callback) {
          m_part->cacheDirty();
          uint16_t date, time;
          FsDateTime::callback(&date, &time);
          setLe16(df->modifyTime, time);
          setLe16(df->modifyDate, date);
          setLe16(df->accessTime, time);
          setLe16(df->accessDate, date);
        }
        break;

      case EXFAT_TYPE_STREAM:
        ds = reinterpret_cast<DirStream_t*>(cache);
        if (isContiguous()) {
          ds->flags |= EXFAT_FLAG_CONTIGUOUS;
        } else {
          ds->flags &= ~EXFAT_FLAG_CONTIGUOUS;
        }
        setLe64(ds->validLength, m_validLength);
        setLe32(ds->firstCluster, m_firstCluster);
        setLe64(ds->dataLength, m_dataLength);
        m_part->cacheDirty();
        break;

      case EXFAT_TYPE_NAME:
        break;

      default:
        DBG_FAIL_MACRO;
        goto fail;
        break;
    }
    checksum = exFatDirChecksum(cache, checksum);
    if (i == setCount) break;
    if (m_part->dirSeek(&pos, 32) != 1) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  }
  df = reinterpret_cast<DirFile_t *>
       (m_part->dirCache(&m_dirPos, FsCache::CACHE_FOR_WRITE));
  if (!df) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  setLe16(df->setChecksum, checksum);
  if (!m_part->cacheSync()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  return true;

 fail:
  m_error |= WRITE_ERROR;
  return false;
}
//------------------------------------------------------------------------------
bool ExFatFile::truncate() {
  uint32_t toFree;
  // error if not a normal file or read-only
  if (!isFile() || !(m_flags & O_WRITE)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (m_firstCluster == 0) {
      return true;
  }
  if (isContiguous()) {
    uint32_t nc = 1 + ((m_dataLength - 1) >> m_part->bytesPerClusterShift());
    if (m_curCluster) {
      toFree = m_curCluster + 1;
      nc -= 1 + m_curCluster - m_firstCluster;
    } else {
      toFree = m_firstCluster;
      m_firstCluster = 0;
    }
    if (nc && !m_part->bitmapModify(toFree, nc, 0)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  } else {
    // need to free chain
    if (m_curCluster) {
      toFree = 0;
      int8_t fg = m_part->fatGet(m_curCluster, &toFree);
      if (fg < 0) {
        DBG_FAIL_MACRO;
        goto fail;
      }
      if (fg) {
        // current cluster is end of chain
        if (!m_part->fatPut(m_curCluster, EXFAT_EOC)) {
          DBG_FAIL_MACRO;
          goto fail;
        }
      }
    } else {
      toFree = m_firstCluster;
      m_firstCluster = 0;
    }
    if (toFree) {
      if (!m_part->freeChain(toFree)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
    }
  }
  m_dataLength = m_curPosition;
  m_validLength = m_curPosition;
  return true;

 fail:
  return false;
}
//------------------------------------------------------------------------------
size_t ExFatFile::write(const void* buf, size_t nbyte) {
  // convert void* to uint8_t*  -  must be before goto statements
  const uint8_t* src = reinterpret_cast<const uint8_t*>(buf);
  uint8_t* cache;
  uint8_t cacheOption;
  uint16_t sectorOffset;
  uint32_t sector;
  uint32_t clusterOffset;

  // number of bytes left to write  -  must be before goto statements
  size_t toWrite = nbyte;
  size_t n;
  // error if not an open file or is read-only
  if (!isFile() || !(m_flags & O_WRITE)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // seek to end of file if append flag
  if ((m_flags & O_APPEND)) {
    if (!seekSet(m_validLength)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  }
  while (toWrite) {
    clusterOffset = m_curPosition & m_part->clusterMask();
    sectorOffset = clusterOffset & m_part->sectorMask();
    if (clusterOffset == 0) {
      // start of new cluster
      if (m_curCluster != 0) {
        int fg;

        if (isContiguous()) {
          uint32_t lc = m_firstCluster;
          lc += (m_dataLength - 1) >> m_part->bytesPerClusterShift();
          if (m_curCluster < lc) {
            m_curCluster++;
            fg = 1;
          } else {
            fg = 0;
          }
        } else {
          fg = m_part->fatGet(m_curCluster, &m_curCluster);
          if (fg < 0) {
            DBG_FAIL_MACRO;
            goto fail;
          }
        }
        if (fg == 0) {
          // add cluster if at end of chain
          if (!addCluster()) {
            DBG_FAIL_MACRO;
            goto fail;
          }
        }
      } else {
        if (m_firstCluster == 0) {
          // allocate first cluster of file
          if (!addCluster()) {
            DBG_FAIL_MACRO;
            goto fail;
          }
          m_firstCluster = m_curCluster;
        } else {
          m_curCluster = m_firstCluster;
        }
      }
    }
    // sector for data write
    sector = m_part->clusterStartSector(m_curCluster) +
             (clusterOffset >> m_part->bytesPerSectorShift());

    if (sectorOffset != 0 || toWrite < m_part->bytesPerSector()) {
      // partial sector - must use cache
      // max space in sector
      n = m_part->bytesPerSector() - sectorOffset;
      // lesser of space and amount to write
      if (n > toWrite) {
        n = toWrite;
      }

      if (sectorOffset == 0 && m_curPosition >= m_validLength) {
        // start of new sector don't need to read into cache
        cacheOption = FsCache::CACHE_RESERVE_FOR_WRITE;
      } else {
        // rewrite part of sector
        cacheOption = FsCache::CACHE_FOR_WRITE;
      }
      cache = m_part->cacheFill(sector, cacheOption);
      if (!cache) {
        DBG_FAIL_MACRO;
        goto fail;
      }
      uint8_t* dst = cache + sectorOffset;
      memcpy(dst, src, n);
      if (m_part->bytesPerSector() == (n + sectorOffset)) {
        // Force write if sector is full - improves large writes.
        if (!m_part->cacheSyncData()) {
          DBG_FAIL_MACRO;
          goto fail;
        }
      }
#if USE_MULTI_SECTOR_IO
    } else if (toWrite >= 2*m_part->bytesPerSector()) {
      // use multiple sector write command
      uint32_t ns = toWrite >> m_part->bytesPerSectorShift();
      // Limit writes to current cluster.
      uint32_t maxNs = m_part->sectorsPerCluster()
                       - (clusterOffset >> m_part->bytesPerSectorShift());
      if (ns > maxNs) {
        ns = maxNs;
      }
      n = ns << m_part->bytesPerSectorShift();
      if (sector <= m_part->cacheSector()
          && m_part->cacheSector() < (sector + ns)) {
        // invalidate cache if sector is in cache
        m_part->cacheInvalidate();
      }
      if (!m_part->writeSectors(sector, src, ns)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
#endif  // USE_MULTI_SECTOR_IO
    } else {
      // use single sector write command
      n = m_part->bytesPerSector();
      if (m_part->cacheSector() == sector) {
        m_part->cacheInvalidate();
      }
      if (!m_part->writeSector(sector, src)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
    }
    m_curPosition += n;
    src += n;
    toWrite -= n;
    if (m_curPosition > m_validLength) {
      m_flags |= FILE_FLAG_DIR_DIRTY;
      m_validLength = m_curPosition;
    }
  }

  if (m_curPosition > m_dataLength) {
    m_dataLength = m_curPosition;
    // update fileSize and insure sync will update dir entr
    m_flags |= FILE_FLAG_DIR_DIRTY;
  } else if (FsDateTime::callback) {
    // insure sync will update modified date and time
    m_flags |= FILE_FLAG_DIR_DIRTY;
  }
  return nbyte;

fail:
  // return for write error
  m_error |= WRITE_ERROR;
  return -1;
}
#endif  // READ_ONLY
