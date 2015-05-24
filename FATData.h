#ifndef FATDATA_H
#define FATDATA_H

#define BPB_SIZE 36
#define BPB_BYTES_PER_SEC_OFFSET 11
#define BPB_BYTES_PER_SEC_SIZE 2
#define BPB_SEC_PER_CLUS_OFFSET 13
#define BPB_SEC_PER_CLUS_SIZE 1
#define BPB_RSVD_SEC_CNT_OFFSET 14
#define BPB_RSVD_SEC_CNT_SIZE 2
#define BPB_ROOT_ENT_CNT_OFFSET 17
#define BPB_ROOT_ENT_CNT_SIZE 2
#define BPB_TOT_SEC_16_OFFSET 19
#define BPB_TOT_SEC_16_SIZE 2
#define BPB_FAT_SZ16_OFFSET 22
#define BPB_FAT_SZ16_SIZE 2
#define BPB_TOT_SEC_32_OFFSET 32
#define BPB_TOT_SEC_32_SIZE 4
#define BPB_NUM_FATS 2
#define ROOT_ENT_SZ 32
#define DIRENT_NAME_OFFSET 0
#define DIRENT_NAME_SZ 11
#define DIRENT_DATE_OFFSET 16
#define DIRENT_DATE_SZ 2
#define DIRENT_TIME_OFFSET 14
#define DIRENT_TIME_SZ 2
#define DIRENT_TIME_CS_OFFSET 13
#define DIRENT_TIME_CS_SZ 1
#define DIRENT_ATTR_OFFSET 11
#define DIRENT_ATTR_SZ 1
#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20
#define ATTR_LONG_NAME (0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20)

#include <stdint.h>
#include "VirtualMachine.h"
#include <cstring>

class FATData
{
  unsigned int bytesPerSector;
  unsigned int sectorsPerCluster;
  unsigned int reservedSectorCount;
  unsigned int rootEntityCount;
  unsigned int totalSectors16;
  unsigned int FATSz16;
  unsigned int totalSectors32;
  uint8_t* BPB2;
  uint8_t* FAT;
  uint8_t* ROOT;
public:
  FATData(const char* mount);
  ~FATData();
  unsigned int getBytesPerSector();
  void fatls();
  void fatvol();
  void fillDateTime(SVMDateTimeRef dt, uint8_t* loc);
  void fillDirEnt(SVMDirectoryEntryRef dir, uint8_t* loc);
};

unsigned int bytesToUnsigned(uint8_t* start, unsigned int size);


#endif
