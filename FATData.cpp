#include "FATData.h"
#include <fstream>
#include <iostream>
#include <sys/stat.h>


using namespace std;


FATData::FATData(const char* mount)
{
  uint8_t* BPB = new uint8_t[BPB_SIZE];
  BPB2 = new uint8_t[BPB_SIZE];
  unsigned int FATSz;
  unsigned int ROOTSz;
  ifstream imageFile(mount, ios::in | ios::binary);
  imageFile.read((char*)BPB, BPB_SIZE);
  memcpy(BPB2, BPB, BPB_SIZE);

  bytesPerSector      = bytesToUnsigned(&BPB[BPB_BYTES_PER_SEC_OFFSET], BPB_BYTES_PER_SEC_SIZE);
  sectorsPerCluster   = bytesToUnsigned(&BPB[BPB_SEC_PER_CLUS_OFFSET],  BPB_SEC_PER_CLUS_SIZE);
  reservedSectorCount = bytesToUnsigned(&BPB[BPB_RSVD_SEC_CNT_OFFSET],  BPB_RSVD_SEC_CNT_SIZE);
  rootEntryCount      = bytesToUnsigned(&BPB[BPB_ROOT_ENT_CNT_OFFSET],  BPB_ROOT_ENT_CNT_SIZE);
  totalSectors16      = bytesToUnsigned(&BPB[BPB_TOT_SEC_16_OFFSET],    BPB_TOT_SEC_16_SIZE);
  FATSz16             = bytesToUnsigned(&BPB[BPB_FAT_SZ16_OFFSET],      BPB_FAT_SZ16_SIZE);
  totalSectors32      = bytesToUnsigned(&BPB[BPB_TOT_SEC_32_OFFSET],    BPB_TOT_SEC_32_SIZE);

  FATSz = BPB_NUM_FATS * FATSz16;
  ROOTSz = rootEntryCount * ROOT_ENT_SZ / 512; //Dividing by 512 is black magic from a handout
  FAT = new uint8_t[FATSz];
  ROOT = new uint8_t[ROOTSz];
  imageFile.read((char*)FAT, FATSz);
  imageFile.read((char*)ROOT, ROOTSz);

  imageFile.close();
  delete BPB;
}//FATData constructor


FATData::~FATData()
{
  delete BPB2;
  delete FAT;
  delete ROOT;
}//FATData destructor


unsigned int FATData::getBytesPerSector()
{
  return bytesPerSector;
}//unsigned int FATData::getBytesPerSector()


void FATData::fatls()
{
  cout << "   DATE   |  TIME  | TYPE |    SIZE   |    SFN      |  LFN\n";
}//void FATData::fatls()


void FATData::fatvol()
{
  unsigned int RootDirectorySectors = (rootEntryCount * 32) / 512;
  unsigned int firstRootSector = reservedSectorCount + BPB_NUM_FATS * FATSz16;
  unsigned int firstDataSector = firstRootSector + rootDirectorySectors;
  unsigned int clusterCount = (totalSectors32 - firstDataSector) / sectorsPerCluster;
  cout << "OEM Name           : " << (char*)&BPB2[3] << endl;
  cout << "Bytes Per Sector   : " << bytesPerSector << endl;
  cout << "Sectors Per Cluster: " << sectorsPerCluster << endl;
  cout << "Reserved Sectors   : " << reservedSectorCount << endl;
  cout << "FAT Count          : " << BPB_NUM_FATS << endl;
  cout << "Root Entry         : " << rootEntryCount << endl;
  cout << "Sector Count 16    : " << totalSectors16 << endl;
  cout << "Media              : " << (unsigned int)BPB2[21] << endl;
  cout << "FAT Size 16        : " << FATSz16 << endl;
  cout << "Sectors Per Track  : " << bytesToUnsigned(&BPB2[24], 2) << endl;
  cout << "Head Count         : " << bytesToUnsigned(&BPB2[26], 2) << endl;
  cout << "Hidden Sector Count: " << bytesToUnsigned(&BPB2[28], 4) << endl;
  cout << "Sector Count 32    : " << totalSectors32 << endl;
  cout << "Drive Number       : " << bytesToUnsigned(&BPB2[36], 1) << endl;
  cout << "Boot Signature     : " << bytesToUnsigned(&BPB2[38], 1) << endl;
  cout << "Volume ID          : " << bytesToUnsigned(&BPB2[39], 4) << endl;
  cout << "Volume Label       : " << (char*)&BPB2[43] << endl;
  cout << "File System Type   : " << (char*)&BPB2[54] << endl;
  cout << "Root Dir Sectors   : " << RootDirectorySectors << endl;
  cout << "First Root Sector  : " << firstRootSector << endl;
  cout << "First Data Sector  : " << firstDataSector << endl;
  cout << "Cluster Count      : " << clusterCount << endl;
}//void FATData::fatvol()


void FATData::fillDateTime(SVMDateTimeRef dt, time_t t, unsigned char dh = 0)
{
  struct tm LocalTime = *localtime(&t);
  dt->DYear = LocalTime.tm_year + 1900;
  dt->DMonth = LocalTime.tm_mon + 1;
  dt->DDay = LocalTime.tm_mday;
  dt->DHour = LocalTime.tm_hour;
  dt->DMinute = LocalTime.tm_min;
  dt->DSecond = LocalTime.tm_sec;
  dt->DHundredth = dh;
}//void FATData::fillDateTime(SVMDateTimeRef dt, time_t t, unsigned char dh = 0)


void FATData::fillDirEnt(SVMDirectoryEntryRef dir, uint8_t* loc)
{
  dir->DLongFileName;
  dir->DShortFileName;
  dir->DSize;
  dir->DAttributes;
  dir->DCreate;
  dir->DAccess;
  dir->DModify;
}//void FATData::fillDirEnt(SVMDirectoryEntryRef dir, uint8_t* loc)

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


