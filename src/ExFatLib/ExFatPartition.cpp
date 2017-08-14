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
#include "ExFatVolume.h"
#include "../common/FsStructs.h"

//-----------------------------------------------------------------------------
void FsCache::invalidate() {
  m_status = 0;
  m_sector = 0XFFFFFFFF;
}
//-----------------------------------------------------------------------------
uint8_t* FsCache::fill(uint32_t sector, uint8_t option) {
  if (!m_blockDev) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (m_sector != sector) {
    if (!sync()) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    if (!(option & CACHE_OPTION_NO_READ)) {
      if (!m_blockDev->readSector(sector, m_cacheBuffer)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
    }
    m_status = 0;
    m_sector = sector;
  }
  m_status |= option & CACHE_STATUS_MASK;
  return m_cacheBuffer;

fail:
  return nullptr;
}
//------------------------------------------------------------------------------
bool FsCache::sync() {
  if (m_status & CACHE_STATUS_DIRTY) {
    if (!m_blockDev->writeSector(m_sector, m_cacheBuffer)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    m_status &= ~CACHE_STATUS_DIRTY;
  }
  return true;

fail:
  return false;
}
//=============================================================================
bool ExFatPartition::init(uint8_t part) {
  uint32_t volStart = 0;
  uint8_t *cache;
  pbs_t* pbs;
  BpbExFat_t* bpb;
  MbrSector_t* mbr;
  MbrPart_t* mp;

  m_cache.init(m_blockDev);
  cache = m_cache.fill(0, FsCache::CACHE_FOR_READ);
  if (part > 4 || !cache) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (part >= 1) {
    mbr = reinterpret_cast<MbrSector_t*>(cache);
    mp = &mbr->part[part - 1];
    if ((mp->boot != 0 && mp->boot != 0X80) || mp->type == 0) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    volStart = getLe32(mp->relativeSectors);
    cache = m_cache.fill(volStart, FsCache::CACHE_FOR_READ);
    if (!cache) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  }
  pbs = reinterpret_cast<pbs_t*>(cache);
  if (strncmp(pbs->oemName, "EXFAT", 5)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  bpb = reinterpret_cast<BpbExFat_t*>(pbs->bpb);
  if (bpb->bytesPerSectorShift != m_bytesPerSectorShift) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  m_fatStartSector = volStart + getLe32(bpb->fatOffset);
  m_fatLength = getLe32(bpb->fatLength);
  m_clusterHeapStartSector = volStart + getLe32(bpb->clusterHeapOffset);
  m_clusterCount = getLe32(bpb->clusterCount);
  m_rootDirectoryCluster = getLe32(bpb->rootDirectoryCluster);
  m_sectorsPerClusterShift = bpb->sectorsPerClusterShift;
  m_bytesPerCluster = 1UL << (m_bytesPerSectorShift + m_sectorsPerClusterShift);
  m_clusterMask = m_bytesPerCluster - 1;
  return true;

 fail:
  return false;
}
//-----------------------------------------------------------------------------
// return 0 if error, 1 if no space, else start cluster.
uint32_t ExFatPartition::bitmapFind(uint32_t cluster, uint32_t count) {
  ////////////////////  fix for search start  ///////////////////////////////////////////////////////
  uint32_t start = (cluster -= 2) >= m_clusterCount ? 0 : cluster;
  uint32_t endAlloc = start;
  uint32_t bgnAlloc = start;
  uint16_t sectorSize = 1 << m_bytesPerSectorShift;
  size_t i = (start >> 3) & (sectorSize - 1);
  uint8_t* cache;
  uint8_t mask = 1 << (start & 7);
  while (true) {
    uint32_t sector = m_clusterHeapStartSector +
                     (endAlloc >> (m_bytesPerSectorShift + 3));
    cache = m_cache.fill(sector, FsCache::CACHE_FOR_READ);
    if (!cache) {
      return 0;
    }
    for (; i < sectorSize; i++) {
      for (; mask; mask <<= 1) {
        endAlloc++;
        if (!(mask & cache[i])) {
          if ((endAlloc - bgnAlloc) == count) {
            return bgnAlloc + 2;
          }
        } else {
          bgnAlloc = endAlloc;
        }
        if (endAlloc == start) {
          return 1;
        }
        if (endAlloc >= m_clusterCount) {
          endAlloc = bgnAlloc = 0;
          i = sectorSize;
          break;
        }
      }
      mask = 1;
    }
    i = 0;
  }
  return 0;
}
//-----------------------------------------------------------------------------
bool ExFatPartition::bitmapModify(uint32_t cluster,
                                  uint32_t count, bool value) {
  uint32_t sector;
  size_t i;
  uint8_t* cache;
  uint8_t mask;
  cluster -= 2;
  if ((cluster + count) > m_clusterCount) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  mask = 1 << (cluster & 7);
  sector = m_clusterHeapStartSector +
                   (cluster >> (m_bytesPerSectorShift + 3));
  i = (cluster >> 3) & m_sectorMask;
  while (true) {
    cache = m_cache.fill(sector++, FsCache::CACHE_FOR_WRITE);
    if (!cache) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    for (; i < m_bytesPerSector; i++) {
      for (; mask; mask <<= 1) {
        if (value == static_cast<bool>(cache[i] & mask)) {
          DBG_FAIL_MACRO;
          goto fail;
        }
        cache[i] ^= mask;
        if (--count == 0) {
          return true;
        }
      }
      mask = 1;
    }
    i = 0;
  }

 fail:
  return false;
}
//-----------------------------------------------------------------------------
uint32_t ExFatPartition::chainSize(uint32_t cluster) {
  uint32_t n = 0;
  int8_t status;
  do {
    status = fatGet(cluster, & cluster);
    if (status < 0) return 0;
    n++;
  } while (status);
  return n;
}
//-----------------------------------------------------------------------------
uint8_t* ExFatPartition::dirCache(DirPos_t* pos, uint8_t options) {
  uint32_t sector = clusterStartSector(pos->cluster);
  sector += (m_clusterMask & pos->position) >> m_bytesPerSectorShift;
  uint8_t* cache = cacheFill(sector, options);
  return cache ? cache + (pos->position & m_sectorMask) : nullptr;
}
//-----------------------------------------------------------------------------
// return -1 error, 0 EOC, 1 OK
int8_t ExFatPartition::dirSeek(DirPos_t* pos, uint32_t offset) {
  int8_t status;
  uint32_t tmp = (m_clusterMask & pos->position) + offset;
  pos->position += offset;
  tmp >>= bytesPerClusterShift();
  while (tmp--) {
    if (pos->isContiguous) {
      pos->cluster++;
    } else {
      status = fatGet(pos->cluster, &pos->cluster);
      if (status != 1) {
        return status;
      }
    }
  }
  return 1;
}
//-----------------------------------------------------------------------------
uint8_t ExFatPartition::fatGet(uint32_t cluster, uint32_t* value) {
  uint8_t* cache;
uint32_t next;
  uint32_t sector;

  if (cluster > (m_clusterCount + 1)) {
    DBG_FAIL_MACRO;
    return -1;
  }
  sector = m_fatStartSector + (cluster >> (m_bytesPerSectorShift - 2));
//  sector = m_fatStartSector + (cluster >> m_uint32PerSectorShift);

  cache = m_cache.fill(sector, FsCache::CACHE_FOR_READ);
  if (!cache) {
    return -1;
  }
  next = getLe32(cache + ((cluster << 2) & m_sectorMask));

  if (next == EXFAT_EOC) {
    return 0;
  }
  *value = next;
  return 1;
}
//-----------------------------------------------------------------------------
bool ExFatPartition::fatPut(uint32_t cluster, uint32_t value) {
  uint32_t sector;
  uint8_t* cache;
  if (cluster < 2 || cluster > (m_clusterCount + 1)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  sector = m_fatStartSector + (cluster >> (m_bytesPerSectorShift - 2));
  cache = m_cache.fill(sector, FsCache::CACHE_FOR_WRITE);
  if (!cache) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  setLe32(cache + ((cluster << 2) & m_sectorMask), value);
  return true;

 fail:
  return false;
}
//-----------------------------------------------------------------------------
bool ExFatPartition::freeChain(uint32_t cluster) {
  uint32_t next;
  uint32_t start = cluster;
  int8_t status;
  do {
    status = fatGet(cluster, &next);
    if (status < 0) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    if (!fatPut(cluster, 0)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    if ((cluster + 1) != next || status == 0) {
      if (!bitmapModify(start, cluster - start + 1, 0)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
      start = next;
    }
    cluster = next;
  } while (status);

  return true;

 fail:
  return false;
}
//-----------------------------------------------------------------------------
uint32_t ExFatPartition::freeClusterCount() {
  uint32_t nc = 0;
  uint32_t sector = m_clusterHeapStartSector;
  uint32_t usedCount = 0;
  uint8_t* cache;

  while (true) {
    cache = m_cache.fill(sector++, FsCache::CACHE_FOR_READ);
    if (!cache) {
      return 0;
    }
    for (size_t i = 0; i < m_bytesPerSector; i++) {
      if (cache[i] == 0XFF) {
        usedCount+= 8;
      } else if (cache[i]) {
        for (uint8_t mask = 1; mask ; mask <<=1) {
          if ((mask & cache[i])) {
            usedCount++;
          }
        }
      }
      nc += 8;
      if (nc >= m_clusterCount) {
        return m_clusterCount - usedCount;
      }
    }
  }
}
//-----------------------------------------------------------------------------
uint32_t ExFatPartition::rootLength() {
  uint32_t nc = chainSize(m_rootDirectoryCluster);
  return nc << bytesPerClusterShift();
}
