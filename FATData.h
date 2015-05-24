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

#include <stdint.h>

class FATData
{
  unsigned int bytesPerSector;
  unsigned int sectorsPerCluster;
  unsigned int reservedSectorCount;
  unsigned int rootEntityCount;
  unsigned int totalSectors16;
  unsigned int FATSz16;
  unsigned int totalSectors32;
  uint8_t* FAT;
  uint8_t* ROOT;
public:
  FATData(const char* mount);
  ~FATData();
  unsigned int getBytesPerSector();
};

unsigned int bytesToUnsigned(uint8_t* start, unsigned int size);


#endif
