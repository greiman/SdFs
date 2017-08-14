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
#include "ExFatVolume.h"
#include "upcase.h"
#include "ExFatFile.h"
#include "../common/FsDateTime.h"
#if ENABLE_ARDUINO_FEATURES
//-----------------------------------------------------------------------------
static void printExFatBoot(Print* pr, pbs_t* pbs) {
  BpbExFat_t* ebs = (BpbExFat_t*)pbs->bpb;
  pr->print(F("bpbSig: "));
  pr->println(getLe16(pbs->signature), HEX);
  pr->print(F("FileSystemName: "));
  pr->write((uint8_t*)pbs->oemName, 8);
  pr->println();
  for (size_t i = 0; i < sizeof(ebs->mustBeZero); i++) {
    if (ebs->mustBeZero[i]) {
      pr->println(F("mustBeZero error"));
      break;
    }
  }
  pr->print(F("PartitionOffset: "));
  uint64_t partitionOffset = getLe64(ebs->partitionOffset);
//  pr->print((uint32_t)(partitionOffset >> 32) , HEX);
 // pr->print(" ");
  pr->println((uint32_t)partitionOffset, HEX);
  pr->print(F("VolumeLength: "));
  uint64_t volumeLength = getLe64(ebs->volumeLength);
//  pr->print((uint32_t)(volumeLength >> 32) , HEX);
//  pr->print(" ");
  pr->println((uint32_t)volumeLength);
  pr->print(F("FatOffset: "));
  uint32_t FatOffset = getLe32(ebs->fatOffset);
  pr->println(FatOffset, HEX);
  pr->print(F("FatLength: "));
  pr->println(getLe32(ebs->fatLength), HEX);
  pr->print(F("ClusterHeapOffset: "));
  pr->println(getLe32(ebs->clusterHeapOffset), HEX);
  pr->print(F("ClusterCount: "));
  pr->println(getLe32(ebs->clusterCount));
  pr->print(F("RootDirectoryCluster: "));
  pr->println(getLe32(ebs->rootDirectoryCluster), HEX);
  pr->print(F("VolumeSerialNumber: "));
  pr->println(getLe32(ebs->volumeSerialNumber), HEX);
  pr->print(F("FileSystemRevision: "));
  pr->println(getLe32(ebs->fileSystemRevision), HEX);
  pr->print(F("VolumeFlags : "));
  pr->println(getLe16(ebs->volumeFlags) , HEX);
  pr->print(F("BytesPerSectorShift: "));
  pr->println(ebs->bytesPerSectorShift, HEX);
  pr->print(F("SectorsPerClusterShift : "));
  pr->println(ebs->sectorsPerClusterShift, HEX);
  pr->print(F("NumberOfFats : "));
  pr->println(ebs->numberOfFats, HEX);
  pr->print(F("DriveSelect : "));
  pr->println(ebs->driveSelect, HEX);
  pr->print(F("PercentInUse : "));
  pr->println(ebs->percentInUse, HEX);
}
//-----------------------------------------------------------------------------
static void printHex(Print* pr, uint8_t h) {
  if (h < 16) {
    pr->write('0');
  }
  pr->print(h, HEX);
}
//-----------------------------------------------------------------------------
static void printMbr(Print* pr, MbrSector_t* mbr) {
  pr->print(F("mbrSig: "));
  pr->println(getLe16(mbr->signature), HEX);
  for (int i = 0; i < 4; i++) {
    printHex(pr, mbr->part[i].boot);
    pr->write(' ');
    for (int k = 0; k < 3; k++) {
      printHex(pr, mbr->part[i].beginCHS[k]);
      pr->write(' ');
    }
    printHex(pr, mbr->part[i].type);
    pr->write(' ');
    for (int k = 0; k < 3; k++) {
      printHex(pr, mbr->part[i].endCHS[k]);
      pr->write(' ');
    }
    pr->print(getLe32(mbr->part[i].relativeSectors), HEX);
    pr->print(' ');
    pr->println(getLe32(mbr->part[i].totalSectors), HEX);
  }
}

//-----------------------------------------------------------------------------
static void printHex(Print* pr, uint16_t val) {
   bool space = true;
  for (uint8_t i = 0; i < 4; i++) {
    uint8_t h = (val >> (12 - 4*i)) & 15;
    if (h || i == 3) {
      space = false;
    }
    if (space) {
      pr->write(' ');
    } else {
      pr->print(h, HEX);
    }
  }
}
//-----------------------------------------------------------------------------
static void printHex(Print* pr, uint32_t val) {
  bool space = true;
  for (uint8_t i = 0; i < 8; i++) {
    uint8_t h = (val >> (28 - 4*i)) & 15;
    if (h || i == 7) {
      space = false;
    }
    if (space) {
      pr->write(' ');
    } else {
      pr->print(h, HEX);
    }
  }
}
//-----------------------------------------------------------------------------
void ExFatPartition::dmpSector(Print* pr, uint32_t sector) {
  uint8_t* cache = m_cache.fill(sector, FsCache::CACHE_FOR_READ);
  if (!cache) {
    pr->println(F("dmpSector failed"));
    return;
  }
  for (uint16_t i = 0; i < 512; i++) {
    if (i%32 == 0) {
      if (i) {
        pr->println();
      }
      printHex(pr, i);
    }
    pr->write(' ');
    printHex(pr, cache[i]);
  }
  pr->println();
}
//-----------------------------------------------------------------------------
void ExFatPartition::dmpBitmap(Print* pr) {
  pr->println(F("bitmap:"));
  dmpSector(pr, m_clusterHeapStartSector);
}
//-----------------------------------------------------------------------------
void ExFatPartition::dmpFat(Print* pr, uint32_t start, uint32_t count) {
  uint32_t sector = m_fatStartSector + start;
  uint32_t cluster = 128*start;
  pr->println(F("FAT:"));
  for (uint32_t i = 0; i < count; i++) {
    uint32_t *cache = (uint32_t*)m_cache.fill(sector + i, FsCache::CACHE_FOR_READ);
    if (!cache) {
      pr->println(F("cache read failed"));
      return;
    }
    for (size_t k = 0; k < 128; k++) {
      if (0 == cluster%8) {
        if (k) {
          pr->println();
        }
        printHex(pr, cluster);
      }
      cluster++;
      pr->write(' ');
      printHex(pr, cache[k]);
    }
    pr->println();
  }
}
//-----------------------------------------------------------------------------
void ExFatPartition::printFat(Print* pr) {
  uint32_t next;
  int8_t status;
  pr->println(F("FAT:"));
  for (uint32_t cluster = 0; cluster < 16; cluster++) {
    status = fatGet(cluster, &next);
    pr->print(cluster, HEX);
    pr->write(' ');
    if (status == 0) {
      next = EXFAT_EOC;
    }
    pr->println(next, HEX);
  }
}
//-----------------------------------------------------------------------------
bool ExFatPartition::printVolInfo(Print* pr) {
  MbrSector_t* mbr = (MbrSector_t*)m_cache.fill(0, FsCache::CACHE_FOR_READ);
  if (!mbr) {
    pr->println(F("read mbr failed"));
    return false;
  }
  printMbr(pr, mbr);
  uint32_t volStart = getLe32(mbr->part->relativeSectors);
  uint32_t volSize = getLe32(mbr->part->totalSectors);
  if (volSize == 0) {
    pr->print(F("bad partition size"));
    return false;
  }
  pbs_t* pbs = (pbs_t*)m_cache.fill(volStart, FsCache::CACHE_FOR_READ);
  if (!pbs) {
    pr->println(F("read pbs failed"));
    return false;
  }
  printExFatBoot(pr, pbs);
  return true;
}
//-----------------------------------------------------------------------------
static void printDateTime(Print* pr, uint32_t timeDate, uint8_t ms, int8_t tz){
  fsPrintDateTime(pr, timeDate, ms, tz);
  pr->println();
}
//-----------------------------------------------------------------------------
static void printDirBitmap(Print* pr, DirBitmap_t* dir) {
  pr->print(F("dirBitmap: "));
  pr->println(dir->type, HEX);
  pr->print(F("flags: "));
  pr->println(dir->flags, HEX);
  pr->print(F("firstCluster: "));
  pr->println(getLe32(dir->firstCluster));
  pr->print(F("size: "));
  pr->println((uint32_t)getLe64(dir->size));
}
//-----------------------------------------------------------------------------
static void printDirUpcase(Print* pr, DirUpcase_t* dir) {
  pr->print(F("dirUpcase: "));
  pr->println(dir->type, HEX);
    pr->print(F("checksum: "));
  pr->println(getLe32(dir->checksum), HEX);
  pr->print(F("firstCluster: "));
  pr->println(getLe32(dir->firstCluster));
  pr->print(F("size: "));
  pr->println((uint32_t)getLe64(dir->size));
}
//-----------------------------------------------------------------------------
static void printDirLabel(Print* pr, DirLabel_t* dir) {
  pr->print(F("dirLabel: "));
  pr->println(dir->type, HEX);
  pr->print(F("labelLength: "));
  pr->println(dir->labelLength);
  pr->print(F("unicode: "));
  for (size_t i = 0; i < dir->labelLength; i++) {
    pr->write(dir->unicode[2*i]);
  }
  pr->println();
}
//-----------------------------------------------------------------------------
static void printDirFile(Print* pr, DirFile_t* dir) {
  pr->print(F("dirFile: "));
  pr->println(dir->type, HEX);
  pr->print(F("setCount: "));
  pr->println(dir->setCount);
  pr->print(F("setChecksum: "));
  pr->println(getLe16(dir->setChecksum), HEX);
  pr->print(F("attributes: "));
  pr->println(getLe16(dir->attributes), HEX);
  pr->print(F("createTime: "));
  printDateTime(pr, getLe32(dir->createTime),
                dir->createTimeMs, dir->createTimezone);
  pr->print(F("modifyDate: "));
  printDateTime(pr, getLe32(dir->modifyTime),
                dir->modifyTimeMs, dir->createTimezone);
  pr->print(F("accessTime: "));
  printDateTime(pr, getLe32(dir->accessTime),
                dir->accessTimeMs, dir->createTimezone);
}
//-----------------------------------------------------------------------------
static void printDirStream(Print* pr, DirStream_t* dir) {
  pr->print(F("dirStream: "));
  pr->println(dir->type, HEX);
  pr->print(F("flags: "));
  pr->println(dir->flags, HEX);
  pr->print(F("nameLength: "));
  pr->println(dir->nameLength);
  pr->print(F("nameHash: "));
  pr->println(getLe16(dir->nameHash));
  pr->print(F("validLength: "));
  pr->println((uint32_t)getLe64(dir->validLength));
  pr->print(F("firstCluster: "));
  pr->println(getLe32(dir->firstCluster));
  pr->print(F("dataLength: "));
  pr->println((uint32_t)getLe64(dir->dataLength));
}
//-----------------------------------------------------------------------------
static void printDirName(Print* pr, DirName_t* dir) {
  pr->print(F("dirName: "));
  pr->println(dir->type, HEX);
  pr->print(F("unicode: "));
  for (size_t i = 0; i < 30; i += 2) {
    if (dir->unicode[i] == 0) break;
    pr->write(dir->unicode[i]);
  }
  pr->println();
}
//-----------------------------------------------------------------------------
static uint16_t exFatDirChecksum(const uint8_t* data, uint16_t checksum) {
  bool skip = data[0] == EXFAT_TYPE_FILE;
  for (size_t i = 0; i < 32; i += (i == 1 && skip ? 3 : 1)) {
		checksum = ((checksum << 15) | (checksum >> 1)) + data[i];
  }
  return checksum;
}
//-----------------------------------------------------------------------------
void ExFatPartition::checkUpcase(Print* pr) {
  bool skip = false;
  uint16_t u = 0;
  uint8_t* upcase = nullptr;
  uint32_t sector;
  uint32_t size = 0;
  DirUpcase_t* dir;
  sector = clusterStartSector(m_rootDirectoryCluster);
  dir = (DirUpcase_t*)m_cache.fill(sector, FsCache::CACHE_FOR_READ);
  if (!dir) {
    pr->println(F("read root failed"));
    return;
  }
  for (size_t i = 0; i < 16; i++) {
    if (dir[i].type == EXFAT_TYPE_UPCASE) {
      sector = clusterStartSector(getLe32(dir[i].firstCluster));
      size = getLe64(dir[i].size);
      break;
    }
  }
  if (!size) {
    pr->println(F("upcase not found"));
    return;
  }
  for (size_t i = 0; i < size/2; i++) {
    if ((i%256) == 0) {
      upcase = m_cache.fill(sector++, FsCache::CACHE_FOR_READ);
      if (!upcase) {
        pr->println(F("read upcase failed"));
        return;
      }
    }
    uint16_t v = getLe16(&upcase[2*(i & 0XFF)]);
    if (skip) {
      pr->print("skip ");
      pr->print(u);
      pr->write(' ');
      pr->println(v);
    }
    if (v == 0XFFFF) {
      skip = true;
    } else if (skip) {
      for (uint16_t k = 0; k < v; k++) {
        uint16_t x = toUpcase(u + k);
        if (x != (u + k)) {
          printHex(pr, (uint16_t)(u+k));
          pr->write(',');
          printHex(pr, x);
          pr->println("<<<<<<<<<<<<<<<<<<<<");
        }
      }
      u += v;
      skip = false;
    } else {
      uint16_t x = toUpcase(u);
      if (v != x) {
        printHex(pr, u);
        pr->write(',');
        printHex(pr, x);
        pr->write(',');
        printHex(pr, v);
        pr->println();
      }
      u++;
    }
  }
  pr->println(F("Done"));
}
//-----------------------------------------------------------------------------
bool ExFatPartition::printDir(Print* pr, ExFatFile* file) {
  DirGeneric_t* dir = nullptr;
  DirFile_t* dirFile;
  DirStream_t* dirStream;
  DirName_t* dirName;
  uint16_t calcHash = 0;
  uint16_t nameHash = 0;
  uint16_t setChecksum = 0;
  uint16_t calcChecksum = 0;;
  uint8_t  nameLength = 0;
  uint8_t  setCount = 0;
  uint8_t  nUnicode;

//#define RAW_ROOT
#ifndef RAW_ROOT
  ExFatFile root;
  if (!file) {
    if (!root.openRoot(this)) {
      pr->println(F("openRoot failed"));
      return false;
    }
    file = &root;
  }
  while (1) {
    uint8_t buf[32];
    if (file->read(buf, 32) != 32) {
      break;
    }
    dir = (DirGeneric_t*)buf;
#else
    uint32_t nDir = 1 << (m_sectorsPerClusterShift + 4);
    uint32_t sector = clusterStartSector(m_rootDirectoryCluster);
  for (uint32_t iDir = 0; iDir < nDir; iDir++) {
    size_t i = iDir%16;
    if (i == 0) {
      dir = (DirGeneric_t*)m_cache.fill(sector++, FsCache::CACHE_FOR_READ);
      if (!dir) {
        return false;
      }
    } else {
      dir++;
    }
#endif
    if (dir->type == 0) {
      break;
    }
    pr->println();
    switch (dir->type) {
      case EXFAT_TYPE_BITMAP:
        printDirBitmap(pr, (DirBitmap_t*)dir);
        break;

      case EXFAT_TYPE_UPCASE:
        printDirUpcase(pr, (DirUpcase_t*)dir);
        break;

      case EXFAT_TYPE_LABEL:
        printDirLabel(pr, (DirLabel_t*)dir);
        break;

      case EXFAT_TYPE_FILE:
        dirFile = (DirFile_t*)dir;
        printDirFile(pr, dirFile);
        setCount = dirFile->setCount;
        setChecksum = getLe16(dirFile->setChecksum);
        calcChecksum = exFatDirChecksum((uint8_t*)dir, 0);
        break;

      case EXFAT_TYPE_STREAM:
        dirStream = (DirStream_t*)dir;
        printDirStream(pr, dirStream);
        nameLength = dirStream->nameLength;
        nameHash = getLe16(dirStream->nameHash);
        calcChecksum = exFatDirChecksum((uint8_t*)dir, calcChecksum);
        setCount--;
        calcHash = 0;
        break;

       case EXFAT_TYPE_NAME:
        dirName = (DirName_t*)dir;
        printDirName(pr, dirName);
        calcChecksum = exFatDirChecksum((uint8_t*)dir, calcChecksum);
        nUnicode = nameLength > 15 ? 15 : nameLength;
        calcHash = exFatHashName((ExChar16_t*)dirName->unicode, nUnicode, calcHash);
        nameLength -= nUnicode;
        setCount--;
        if (nameLength == 0  || setCount == 0) {
          pr->print(F("setChecksum: "));
          pr->print(setChecksum, HEX);
          if (setChecksum != calcChecksum) {
            pr->print(F(" != calcChecksum: "));
          } else {
            pr->print(F(" == calcChecksum: "));
          }
          pr->println(calcChecksum, HEX);
          pr->print(F("nameHash: "));
          pr->print(nameHash, HEX);
          if (nameHash != calcHash) {
            pr->print(F(" != calcHash: "));
          } else {
            pr->print(F(" == calcHash: "));
          }
          pr->println(calcHash, HEX);
        }
        break;

      default:
        if (dir->type & 0x80) {
          pr->print(F("Unknown dirType: "));
        } else {
          pr->print(F("Unused dirType: "));
        }
        pr->println(dir->type, HEX);

        for (uint8_t k = 0; k < 31; k++) {
          if (k) {
            pr->write(' ');
          }
          printHex(pr, dir->data[k]);
        }
        pr->println();
        break;
    }
  }
  pr->println(F("Done"));
  return true;
}
//-----------------------------------------------------------------------------
void ExFatPartition::printUpcase(Print* pr) {
  uint8_t* upcase = nullptr;
  uint32_t sector;
  uint32_t size = 0;
  uint32_t checksum = 0;
  DirUpcase_t* dir;
  sector = clusterStartSector(m_rootDirectoryCluster);
  dir = (DirUpcase_t*)m_cache.fill(sector, FsCache::CACHE_FOR_READ);
  if (!dir) {
    pr->println(F("read root failed"));
    return;
  }
  for (size_t i = 0; i < 16; i++) {
    if (dir[i].type == EXFAT_TYPE_UPCASE) {
      sector = clusterStartSector(getLe32(dir[i].firstCluster));
      size = getLe64(dir[i].size);
      break;
    }
  }
  if (!size) {
    pr->println(F("upcase not found"));
    return;
  }
  for (uint16_t i = 0; i < size/2; i++) {
    if ((i%256) == 0) {
      upcase = m_cache.fill(sector++, FsCache::CACHE_FOR_READ);
      if (!upcase) {
        pr->println(F("read upcase failed"));
        return;
      }
    }
    if (i%16 == 0) {
      pr->println();
      printHex(pr, i);
    }
    pr->write(' ');
    uint16_t uc = getLe16(&upcase[2*(i & 0XFF)]);
    printHex(pr, uc);
    checksum = upcaseChecksum(uc, checksum);
  }
  pr->println();
  pr->print(F("checksum: "));
  printHex(pr, checksum);
  pr->println();
}
#endif  // ENABLE_ARDUINO_FEATURES