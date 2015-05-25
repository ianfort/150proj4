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
  cout << "OEM Name           : " << 0 << endl;
  cout << "Bytes Per Sector   : " << 0 << endl;
  cout << "Sectors Per Cluster: " << 0 << endl;
  cout << "Reserved Sectors   : " << 0 << endl;
  cout << "FAT Count          : " << 0 << endl;
  cout << "Root Entry         : " << 0 << endl;
  cout << "Sector Count 16    : " << 0 << endl;
  cout << "Media              : " << 0 << endl;
  cout << "FAT Size 16        : " << 0 << endl;
  cout << "Sectors Per Track  : " << 0 << endl;
  cout << "Head Count         : " << 0 << endl;
  cout << "Hidden Sector Count: " << 0 << endl;
  cout << "Sector Count 32    : " << 0 << endl;
  cout << "Drive Number       : " << 0 << endl;
  cout << "Boot Signature     : " << 0 << endl;
  cout << "Volume ID          : " << 0 << endl;
  cout << "Volume Label       : " << 0 << endl;
  cout << "File System Type   : " << 0 << endl;
  cout << "Root Dir Sectors   : " << 0 << endl;
  cout << "First Root Sector  : " << 0 << endl;
  cout << "First Data Sector  : " << 0 << endl;
  cout << "Cluster Count      : " << 0 << endl;
}//void FATData::fatvol()

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


void fillDate(SVMDateTimeRef dt, uint8_t date[2])
{
  dt->DYear = 1980 + ((date[HI] << 1) & 127); //0000 0000 0111 1111
  dt->DMonth = 1 + ((date[LO] << 5) & 7) + (date[HI] & 1); //0000 0111 1000 0000
  dt->DDay = 1 + (date[LO] & 31); //1111 1000 0000 0000
}//void fillDate(SVMDateTimeRef dt, uint8_t date[2])


void fillDirEnt(SVMDirectoryEntryRef dir, uint8_t* loc)
{
  uint8_t empty[2] = {0, 0};
//  dir->DLongFileName; //TODO
  //13 chars: 8 chars of filename, 1 for '.', 3 for file extension, 1 for '\0' terminating char
  memcpy(dir->DShortFileName, loc+DIRENT_NAME_OFFSET, 8);
  dir->DShortFileName[8] = '.';
  memcpy((dir->DShortFileName)+9, loc+DIRENT_NAME_OFFSET+8, 3);
  dir->DSize = *(loc + DIRENT_FILESIZE_OFFSET);
  dir->DAttributes = *(loc + DIRENT_ATTR_OFFSET);
  fillDate(&(dir->DCreate), (loc+DIRENT_CRT_DATE_OFFSET));
  fillTime(&(dir->DCreate), (loc+DIRENT_CRT_TIME_OFFSET), *(loc+DIRENT_CRT_TIME_CS_OFFSET));
  fillDate(&(dir->DAccess), (loc+DIRENT_ACC_DATE_OFFSET));
  fillTime(&(dir->DAccess), empty);
  fillDate(&(dir->DModify), (loc+DIRENT_WRT_DATE_OFFSET));
  fillTime(&(dir->DModify), (loc+DIRENT_WRT_TIME_OFFSET));
}//void fillDirEnt(SVMDirectoryEntryRef dir, uint8_t* loc)


void fillTime(SVMDateTimeRef dt, uint8_t time[2], unsigned char dh)
{
  dt->DHour = ((time[HI] << 3) & 31); //0000 0000 0001 1111
  dt->DMinute = ((time[LO] << 5) & 7) + (time[HI] & 7); //0000 0111 1110 0000
  dt->DSecond = 2 * (time[LO] & 31); //1111 1000 0000 0000, 2-sec count
  dt->DHundredth = dh;
  if (dh >= 100)
  {
    dt->DSecond += 1;
    dt->DHundredth -= 100;
  }//if there's between one and two hundredths of an extra second, it's a full extra second
}//void fillTime(SVMDateTimeRef dt, uint8_t time[2], unsigned char dh)


