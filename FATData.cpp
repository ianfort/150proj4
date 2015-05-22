#include "FATData.h"
#include <fstream>
#include <iostream>
#include <sys/stat.h>

using namespace std;

/*
  const unsigned int BPBSize;

  const unsigned int bytesPerSecOffset;
  const unsigned int bytesPerSecSize;

  const unsigned int secPerClusOffset;
  const unsigned int secPerClusSize;

  const unsigned int rsvdSecCntOffset;
  const unsigned int rsvdSecCntSize;

  const unsigned int rootEntCntOffset;
  const unsigned int rootEntCntSize;

  const unsigned int totSec16Offset;
  const unsigned int totSec16Size;

  const unsigned int FATSz16Offset;
  const unsigned int FATSz16Size;

  const unsigned int totSec32Offset;
  const unsigned int totSec32Size;
*/

FATData::FATData(const char* mount) : BPBSize(36) ,
bytesPerSecOffset(11) , bytesPerSecSize(2) , secPerClusOffset(13) , secPerClusSize(1) ,
rsvdSecCntOffset(14) , rsvdSecCntSize(2) , rootEntCntOffset(17) , rootEntCntSize(2) ,
totSec16Offset(19) , totSec16Size(2) , FATSz16Offset(22) , FATSz16Size(2) ,
totSec32Offset(32) , totSec32Size(4)
{
  BPB = new uint8_t[BPBSize];
  ifstream imageFileIn(mount, ios::in | ios::binary);
  imageFile.read(BPB, BPBSize);
  imageFile.close();
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

