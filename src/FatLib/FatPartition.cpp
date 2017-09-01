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
#include <string.h>
#include "../common/DebugMacros.h"
#include "../common/FsStructs.h"
#include "FatPartition.h"

//------------------------------------------------------------------------------
cache_t* FatCache::read(uint32_t sector, uint8_t option) {
  if (m_lbn != sector) {
    if (!sync()) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    if (!(option & CACHE_OPTION_NO_READ)) {
      if (!m_part->readSector(sector, m_buffer.data)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
    }
    m_status = 0;
    m_lbn = sector;
  }
  m_status |= option & CACHE_STATUS_MASK;
  return &m_buffer;

fail:

  return nullptr;
}
//------------------------------------------------------------------------------
bool FatCache::sync() {
  if (m_status & CACHE_STATUS_DIRTY) {
    if (!m_part->writeSector(m_lbn, m_buffer.data)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    // mirror second FAT
    if (m_status & CACHE_STATUS_MIRROR_FAT) {
      uint32_t sector = m_lbn + m_part->sectorsPerFat();
      if (!m_part->writeSector(sector, m_buffer.data)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
    }
    m_status &= ~CACHE_STATUS_DIRTY;
  }
  return true;

fail:
  return false;
}
//------------------------------------------------------------------------------
bool FatPartition::allocateCluster(uint32_t current, uint32_t* next) {
  uint32_t find = current ? current : m_allocSearchStart;
  uint32_t start = find;
  while (1) {
    find++;
    // If at end of FAT go to beginning of FAT.
    if (find > m_lastCluster) {
      find = 2;
    }
    uint32_t f;
    int8_t fg = fatGet(find, &f);
    if (fg < 0) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    if (fg && f == 0) {
      break;
    }
    if (find == start) {
      // Can't find space checked all clusters.
      DBG_FAIL_MACRO;
      goto fail;
    }
  }
  // mark end of chain
  if (!fatPutEOC(find)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (current) {
    // link clusters
    if (!fatPut(current, find)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  } else {
    // Remember place for search start.
    m_allocSearchStart = find;
  }
  updateFreeClusterCount(-1);
  *next = find;
  return true;

fail:
  return false;
}
//------------------------------------------------------------------------------
// find a contiguous group of clusters
bool FatPartition::allocContiguous(uint32_t count, uint32_t* firstCluster) {
  // flag to save place to start next search
  bool setStart = true;
  // start of group
  uint32_t bgnCluster;
  // end of group
  uint32_t endCluster;
  // Start at cluster after last allocated cluster.
  uint32_t startCluster = m_allocSearchStart;
  endCluster = bgnCluster = startCluster + 1;

  // search the FAT for free clusters
  while (1) {
    // If past end - start from beginning of FAT.
    if (endCluster > m_lastCluster) {
      bgnCluster = endCluster = 2;
    }
    uint32_t f;
    int8_t fg = fatGet(endCluster, &f);
    if (fg < 0) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    if (f || fg == 0) {
      // cluster in use try next cluster as bgnCluster
      bgnCluster = endCluster + 1;

      // don't update search start if unallocated clusters before endCluster.
      if (bgnCluster != endCluster) {
        setStart = false;
      }
    } else if ((endCluster - bgnCluster + 1) == count) {
      // done - found space
      break;
    }
    // Can't find space if all clusters checked.
    if (startCluster == endCluster) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    endCluster++;
  }
  // remember possible next free cluster
  if (setStart) {
    m_allocSearchStart = endCluster + 1;
  }

  // mark end of chain
  if (!fatPutEOC(endCluster)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // link clusters
  while (endCluster > bgnCluster) {
    if (!fatPut(endCluster - 1, endCluster)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    endCluster--;
  }
  // Maintain count of free clusters.
  updateFreeClusterCount(-count);

  // return first cluster number to caller
  *firstCluster = bgnCluster;
  return true;

fail:
  return false;
}
//------------------------------------------------------------------------------
uint32_t FatPartition::clusterStartSector(uint32_t cluster) const {
  return m_dataStartSector + ((cluster - 2) << m_sectorsPerClusterShift);
}
//------------------------------------------------------------------------------
// Fetch a FAT entry - return -1 error, 0 EOC, else 1.
int8_t FatPartition::fatGet(uint32_t cluster, uint32_t* value) {
  uint32_t sector;
  uint32_t next;
  cache_t* pc;

  // error if reserved cluster of beyond FAT
  DBG_HALT_IF(cluster < 2 || cluster > m_lastCluster);

  if (fatType() == 32) {
    sector = m_fatStartSector + (cluster >> (m_bytesPerSectorShift - 2));
    pc = cacheFetchFat(sector, FatCache::CACHE_FOR_READ);
    if (!pc) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    next = getLe32(reinterpret_cast<uint8_t*>
                  (&pc->fat32[cluster & (m_sectorMask >> 2)]));
  } else if (fatType() == 16) {
    cluster &= 0XFFFF;
    sector = m_fatStartSector + (cluster >> (m_bytesPerSectorShift - 1) );
    pc = cacheFetchFat(sector, FatCache::CACHE_FOR_READ);
    if (!pc) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    next = getLe16(reinterpret_cast<uint8_t*>
                  (&pc->fat16[cluster & (m_sectorMask >> 1)]));
  } else if (FAT12_SUPPORT && fatType() == 12) {
    uint16_t index = cluster;
    index += index >> 1;
    sector = m_fatStartSector + (index >> m_bytesPerSectorShift);
    pc = cacheFetchFat(sector, FatCache::CACHE_FOR_READ);
    if (!pc) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    index &= m_sectorMask;
    uint16_t tmp = pc->data[index];
    index++;
    if (index == m_bytesPerSector) {
      pc = cacheFetchFat(sector + 1, FatCache::CACHE_FOR_READ);
      if (!pc) {
        DBG_FAIL_MACRO;
        goto fail;
      }
      index = 0;
    }
    tmp |= pc->data[index] << 8;
    next = cluster & 1 ? tmp >> 4 : tmp & 0XFFF;
  } else {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (isEOC(next)) {
    return 0;
  }
  *value = next;
  return 1;

fail:
  return -1;
}
//------------------------------------------------------------------------------
// Store a FAT entry
bool FatPartition::fatPut(uint32_t cluster, uint32_t value) {
  uint32_t sector;
  cache_t* pc;

  // error if reserved cluster of beyond FAT
  DBG_HALT_IF(cluster < 2 || cluster > m_lastCluster);

  if (fatType() == 32) {
    sector = m_fatStartSector + (cluster >> (m_bytesPerSectorShift - 2));
    pc = cacheFetchFat(sector, FatCache::CACHE_FOR_WRITE);
    if (!pc) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    setLe32(reinterpret_cast<uint8_t*>
           (&pc->fat32[cluster & (m_sectorMask >> 2)]), value);
    return true;
  }

  if (fatType() == 16) {
    cluster &= 0XFFFF;
    sector = m_fatStartSector + (cluster >> (m_bytesPerSectorShift - 1) );
    pc = cacheFetchFat(sector, FatCache::CACHE_FOR_WRITE);
    if (!pc) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    setLe16(reinterpret_cast<uint8_t*>
           (&pc->fat16[cluster & (m_sectorMask >> 1)]), value);
    return true;
  }

  if (FAT12_SUPPORT && fatType() == 12) {
    uint16_t index = cluster;
    index += index >> 1;
    sector = m_fatStartSector + (index >> m_bytesPerSectorShift);
    pc = cacheFetchFat(sector, FatCache::CACHE_FOR_WRITE);
    if (!pc) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    index &= m_sectorMask;
    uint8_t tmp = value;
    if (cluster & 1) {
      tmp = (pc->data[index] & 0XF) | tmp << 4;
    }
    pc->data[index] = tmp;

    index++;
    if (index == m_bytesPerSector) {
      sector++;
      index = 0;
      pc = cacheFetchFat(sector, FatCache::CACHE_FOR_WRITE);
      if (!pc) {
        DBG_FAIL_MACRO;
        goto fail;
      }
    }
    tmp = value >> 4;
    if (!(cluster & 1)) {
      tmp = ((pc->data[index] & 0XF0)) | tmp >> 4;
    }
    pc->data[index] = tmp;
    return true;
  } else {
    DBG_FAIL_MACRO;
    goto fail;
  }

fail:
  return false;
}
//------------------------------------------------------------------------------
// free a cluster chain
bool FatPartition::freeChain(uint32_t cluster) {
  uint32_t next;
  int8_t fg;
  do {
    fg = fatGet(cluster, &next);
    if (fg < 0) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    // free cluster
    if (!fatPut(cluster, 0)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    // Add one to count of free clusters.
    updateFreeClusterCount(1);

    if (cluster < m_allocSearchStart) {
      m_allocSearchStart = cluster;
    }
    cluster = next;
  } while (fg);

  return true;

fail:
  return false;
}
//------------------------------------------------------------------------------
int32_t FatPartition::freeClusterCount() {
#if MAINTAIN_FREE_CLUSTER_COUNT
  if (m_freeClusterCount >= 0) {
    return m_freeClusterCount;
  }
#endif  // MAINTAIN_FREE_CLUSTER_COUNT
  uint32_t free = 0;
  uint32_t sector;
  uint32_t todo = m_lastCluster + 1;
  uint16_t n;

  if (FAT12_SUPPORT && fatType() == 12) {
    for (unsigned i = 2; i < todo; i++) {
      uint32_t c;
      int8_t fg = fatGet(i, &c);
      if (fg < 0) {
        DBG_FAIL_MACRO;
        goto fail;
      }
      if (fg && c == 0) {
        free++;
      }
    }
  } else if (fatType() == 16 || fatType() == 32) {
    sector = m_fatStartSector;
    while (todo) {
      cache_t* pc = cacheFetchFat(sector++, FatCache::CACHE_FOR_READ);
      if (!pc) {
        DBG_FAIL_MACRO;
        goto fail;
      }
      n =  fatType() == 16 ? m_bytesPerSector/2 : m_bytesPerSector/4;
      if (todo < n) {
        n = todo;
      }
      if (fatType() == 16) {
        for (uint16_t i = 0; i < n; i++) {
          if (pc->fat16[i] == 0) {
            free++;
          }
        }
      } else {
        for (uint16_t i = 0; i < n; i++) {
          if (pc->fat32[i] == 0) {
            free++;
          }
        }
      }
      todo -= n;
    }
  } else {
    // invalid FAT type
    DBG_FAIL_MACRO;
    goto fail;
  }
  setFreeClusterCount(free);
  return free;

fail:
  return -1;
}
//------------------------------------------------------------------------------
bool FatPartition::init(BlockDevice* dev, uint8_t part) {
  uint32_t clusterCount;
  uint32_t totalSectors;
  uint32_t volumeStartSector = 0;
  m_blockDev = dev;
  pbs_t* pbs;
  BpbFat32_t* bpb;
  MbrSector_t* mbr;
  uint8_t tmp;
  m_fatType = 0;
  m_allocSearchStart = 1;
  m_cache.init(this);
#if USE_SEPARATE_FAT_CACHE
  m_fatCache.init(this);
#endif  // USE_SEPARATE_FAT_CACHE
  // if part == 0 assume super floppy with FAT boot sector in sector zero
  // if part > 0 assume mbr volume with partition table
  if (part) {
    if (part > 4) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    mbr = reinterpret_cast<MbrSector_t*>
          (cacheFetchData(0, FatCache::CACHE_FOR_READ));
    MbrPart_t *mp = mbr->part + part - 1;

    if (!mbr || mp->type == 0 || (mp->boot != 0 && mp->boot != 0X80)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    volumeStartSector = getLe32(mp->relativeSectors);
  }
  pbs = reinterpret_cast<pbs_t*>
        (cacheFetchData(volumeStartSector, FatCache::CACHE_FOR_READ));
  bpb = reinterpret_cast<BpbFat32_t*>(pbs->bpb);
  if (!pbs || bpb->fatCount != 2 || getLe16(bpb->bytesPerSector) != 512) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  m_sectorsPerCluster = bpb->sectorsPerCluster;
  m_clusterSectorMask = m_sectorsPerCluster - 1;
  // determine shift that is same as multiply by m_sectorsPerCluster
  m_sectorsPerClusterShift = 0;
  for (tmp = 1; m_sectorsPerCluster != tmp; tmp <<= 1) {
    if (tmp == 0) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    m_sectorsPerClusterShift++;
  }
  m_sectorsPerFat = getLe16(bpb->sectorsPerFat16);
  if (m_sectorsPerFat == 0) {
    m_sectorsPerFat = getLe32(bpb->sectorsPerFat32);
  }
  m_fatStartSector = volumeStartSector + getLe16(bpb->reservedSectorCount);

  // count for FAT16 zero for FAT32
  m_rootDirEntryCount = getLe16(bpb->rootDirEntryCount);

  // directory start for FAT16 dataStart for FAT32
  m_rootDirStart = m_fatStartSector + 2 * m_sectorsPerFat;
  // data start for FAT16 and FAT32
  m_dataStartSector = m_rootDirStart +
    ((32 * m_rootDirEntryCount + m_bytesPerSector - 1)/m_bytesPerSector);

  // total sectors for FAT16 or FAT32
  totalSectors = getLe16(bpb->totalSectors16);
  if (totalSectors == 0) {
    totalSectors = getLe32(bpb->totalSectors32);
  }
  // total data sectors
  clusterCount = totalSectors - (m_dataStartSector - volumeStartSector);

  // divide by cluster size to get cluster count
  clusterCount >>= m_sectorsPerClusterShift;
  m_lastCluster = clusterCount + 1;

  // Indicate unknown number of free clusters.
  setFreeClusterCount(-1);
  // FAT type is determined by cluster count
  if (clusterCount < 4085) {
    m_fatType = 12;
    if (!FAT12_SUPPORT) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  } else if (clusterCount < 65525) {
    m_fatType = 16;
  } else {
    m_rootDirStart = getLe32(bpb->fat32RootCluster);
    m_fatType = 32;
  }
  return true;

fail:
  return false;
}
//------------------------------------------------------------------------------
bool FatPartition::wipe(print_t* pr) {
  cache_t* cache;
  uint16_t count;
  uint32_t sector;
  if (!fatType()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  cache = cacheClear();
  if (!cache) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  memset(cache->data, 0, m_bytesPerSector);
  // Zero root.
  if (fatType() == 32) {
    sector = clusterStartSector(m_rootDirStart);
    count = m_sectorsPerCluster;
  } else {
    sector = m_rootDirStart;
    count = m_rootDirEntryCount/16;
  }
  for (uint32_t n = 0; n < count; n++) {
    if (!writeSector(sector + n, cache->data)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  }
  // Clear FATs.
  count = 2*m_sectorsPerFat;
  sector = m_fatStartSector;
  for (uint32_t ns = 0; ns < count; ns++) {
    if (pr && (ns & 0XFF) == 0) {
      pr->write('.');
    }
    if (!writeSector(sector + ns, cache->data)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  }
  // Reserve first two clusters.
  if (fatType() == 32) {
    setLe32(reinterpret_cast<uint8_t*>(&cache->fat32[0]), 0x0FFFFFF8);
    setLe32(reinterpret_cast<uint8_t*>(&cache->fat32[1]), 0x0FFFFFFF);
  } else if (fatType() == 16) {
    setLe16(reinterpret_cast<uint8_t*>(&cache->fat16[0]), 0xFFF8);
    setLe16(reinterpret_cast<uint8_t*>(&cache->fat16[1]), 0xFFFF);
  } else if (FAT12_SUPPORT && fatType() == 12) {
    cache->data[0] = 0xF8;
    cache->data[1] = 0xFF;
    cache->data[2] = 0xFF;
  } else {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (!writeSector(m_fatStartSector, cache->data) ||
      !writeSector(m_fatStartSector + m_sectorsPerFat, cache->data)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (fatType() == 32) {
    // Reserve root cluster.
    if (!fatPutEOC(m_rootDirStart) || !cacheSync()) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  }
  if (pr) {
    pr->write('\r');
    pr->write('\n');
  }
  m_fatType = 0;
  return true;

fail:
  m_fatType = 0;
  return false;
}
