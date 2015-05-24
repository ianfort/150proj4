#include "FATData.h"
#include <fstream>
#include <iostream>
#include <sys/stat.h>


using namespace std;


FATData::FATData(const char* mount)
{
  uint8_t* BPB = new uint8_t[BPB_SIZE];
  unsigned int FATSz;
  unsigned int ROOTSz;
  ifstream imageFile(mount, ios::in | ios::binary);
  imageFile.read((char*)BPB, BPB_SIZE);

  bytesPerSector = bytesToUnsigned(&BPB[BPB_BYTES_PER_SEC_OFFSET], BPB_BYTES_PER_SEC_SIZE);
  sectorsPerCluster = bytesToUnsigned(&BPB[BPB_SEC_PER_CLUS_OFFSET], BPB_SEC_PER_CLUS_SIZE);
  reservedSectorCount = bytesToUnsigned(&BPB[BPB_RSVD_SEC_CNT_OFFSET], BPB_RSVD_SEC_CNT_SIZE);
  rootEntityCount = bytesToUnsigned(&BPB[BPB_ROOT_ENT_CNT_OFFSET], BPB_ROOT_ENT_CNT_SIZE);
  totalSectors16 = bytesToUnsigned(&BPB[BPB_TOT_SEC_16_OFFSET],BPB_TOT_SEC_16_SIZE);
  FATSz16 = bytesToUnsigned(&BPB[BPB_FAT_SZ16_OFFSET], BPB_FAT_SZ16_SIZE);
  totalSectors32 = bytesToUnsigned(&BPB[BPB_TOT_SEC_32_OFFSET],BPB_TOT_SEC_32_SIZE);

  FATSz = BPB_NUM_FATS * FATSz16;
  ROOTSz = rootEntityCount * ROOT_ENT_SZ / 512; //Dividing by 512 is black magic from a handout
  FAT = new uint8_t[FATSz];
  ROOT = new uint8_t[ROOTSz];
  imageFile.read((char*)FAT, FATSz);
  imageFile.read((char*)ROOT, ROOTSz);

  imageFile.close();
  delete BPB;
}//FATData constructor


FATData::~FATData()
{
}//FATData destructor


unsigned int FATData::getBytesPerSector()
{
  return bytesPerSector;
}//unsigned int FATData::getBytesPerSector()


void FATData::fatls()
{
  cout << "   DATE   |  TIME  | TYPE |    SIZE   |    SFN      |  LFN\n";
}//void FATData::fatls()

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


