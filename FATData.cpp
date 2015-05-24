#include "FATData.h"
#include <fstream>
#include <iostream>
#include <sys/stat.h>


using namespace std;

FATData::FATData(const char* mount)
{
  uint8_t* BPB = new uint8_t[BPB_SIZE];
  ifstream imageFile(mount, ios::in | ios::binary);
  imageFile.read((char*)BPB, BPB_SIZE);
  imageFile.close();

  bytesPerSector = bytesToUnsigned(&BPB[BPB_BYTES_PER_SEC_OFFSET], BPB_BYTES_PER_SEC_SIZE);
  sectorsPerCluster = bytesToUnsigned(&BPB[BPB_SEC_PER_CLUS_OFFSET], BPB_SEC_PER_CLUS_SIZE);
  reservedSectorCount = bytesToUnsigned(&BPB[BPB_RSVD_SEC_CNT_OFFSET], BPB_RSVD_SEC_CNT_SIZE);
  rootEntityCount = bytesToUnsigned(&BPB[BPB_ROOT_ENT_CNT_OFFSET], BPB_ROOT_ENT_CNT_SIZE);
  totalSectors16 = bytesToUnsigned(&BPB[BPB_TOT_SEC_16_OFFSET],BPB_TOT_SEC_16_SIZE);
  FATSz16 = bytesToUnsigned(&BPB[BPB_FATS_Z16_OFFSET], BPB_FATS_Z16_SIZE);
  totalSectors32 = bytesToUnsigned(&BPB[BPB_TOT_SEC_32_OFFSET],BPB_TOT_SEC_32_SIZE);

  delete BPB;
}//FATData constructor


FATData::~FATData()
{
}//FATData destructor


unsigned int FATData::getBytesPerSector()
{
  return bytesPerSector;
}//unsigned int FATData::getBytesPerSector()

//***************************************************************************//
// Begin Utility Functions for FATData                                       //
//***************************************************************************//
unsigned int bytesToUnsigned(uint8_t* start, unsigned int size)
{
  unsigned int accum = 0;
  for (unsigned int i = 0 ; i < size ; i++)
  {
    accum += ((unsigned int)start[i]) << (8 * i);
  }
  return accum;
}//unsigned int bytesToUnsigned(uint8_t* start, size)


