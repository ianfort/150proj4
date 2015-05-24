#include "FATData.h"
#include <fstream>
#include <iostream>
#include <sys/stat.h>

/*
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
#define BPB_FATS_Z16_OFFSET 22
#define BPB_FATS_Z16_SIZE 2
#define BPB_TOT_SEC_32_OFFSET 32
#define BPB_TOT_SEC_32_SIZE 4
*/ 

using namespace std;

FATData::FATData(const char* mount)
{
  uint8_t* BPB = new uint8_t[BPBSize];
  ifstream imageFileIn(mount, ios::in | ios::binary);
  imageFile.read(BPB, BPBSize);
  imageFile.close();

  bytesPerSector = bytesToUnsigned(&BPB[BPB_BYTES_PER_SEC_OFFSET], BPB_BYTES_PER_SEC_SIZE);
  sectorsPerCluster = bytesToUnsigned(&BPB[BPB_SEC_PER_CLUS_OFFSET], BPB_SEC_PER_CLUS_SIZE);
  reservedSectorCount = bytesToUnsigned(&BPB[BPB_RSVD_SEC_CNT_OFFSET], BPB_RSVD_SEC_CNT_SIZE);
  rootEntityCount = bytesToUnsigned(&BPB[BPB_ROOT_ENT_CNT_OFFSET], BPB_ROOT_ENT_CNT_SIZE);
  totalSectors16 = bytesToUnsigned(&BPB[BPB_TOT_SEC_16_OFFSET],BPB_TOT_SEC_16_SIZE);
  FATSz16 = bytesToUnsigned(&BPB[BPB_FATS_Z16_OFFSET], BPB_FATS_Z16_SIZE);
  totalSectors32 = bytesToUnsigned(&BPB[BPB_TOT_SEC_32_OFFSET],BPB_TOT_SEC_32_SIZE);

  delete BPB;
}


FATData::~FATData()
{
  delete [] BPB;
}


unsigned int getBytesPerSector()
{
  return bytesToUnsigned(BPB[bytesPerSecOffset], bytesPerSecSize);
  /*
  uint8_t leastSig;
  uint8_t mostSig;

  leastSig = BPB[bytesPerSecOffset];
  mostSig = BPB[bytesPerSecOffset + 1];

  return (((unsigned int)mostSig)) << 8) + (unsigned int)leastSig;
  */
}


unsigned int bytesToUnsigned(uint8_t* start, size)
{
  unsigned int accum = 0;
  for (unsigned int i = 0 ; i < size ; i++)
  {
    accum += ((unsigned int)start[i]) << (8 * i);
  }
  return accum;
}

