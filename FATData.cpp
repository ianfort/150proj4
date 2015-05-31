#include "FATData.h"
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <cstring>


using namespace std;


FATData::FATData(const char* mount)
{
  unsigned int imFileNameLen = strlen(mount);
  imFileName = new char[imFileNameLen];
  memcpy(imFileName, mount, imFileNameLen);
  BPB = new uint8_t[BPB_SIZE];
  unsigned int FATSz;
  unsigned int ROOTSz;
  ifstream imageFile(mount, ios::in | ios::binary);
  imageFile.read((char*)BPB, BPB_SIZE);

  bytesPerSector      = bytesToUnsigned(&BPB[BPB_BYTES_PER_SEC_OFFSET], BPB_BYTES_PER_SEC_SIZE);
  sectorsPerCluster   = bytesToUnsigned(&BPB[BPB_SEC_PER_CLUS_OFFSET],  BPB_SEC_PER_CLUS_SIZE);
  reservedSectorCount = bytesToUnsigned(&BPB[BPB_RSVD_SEC_CNT_OFFSET],  BPB_RSVD_SEC_CNT_SIZE);
  rootEntryCount      = bytesToUnsigned(&BPB[BPB_ROOT_ENT_CNT_OFFSET],  BPB_ROOT_ENT_CNT_SIZE);
  totalSectors16      = bytesToUnsigned(&BPB[BPB_TOT_SEC_16_OFFSET],    BPB_TOT_SEC_16_SIZE);
  FATSz16             = bytesToUnsigned(&BPB[BPB_FAT_SZ16_OFFSET],      BPB_FAT_SZ16_SIZE);
  totalSectors32      = bytesToUnsigned(&BPB[BPB_TOT_SEC_32_OFFSET],    BPB_TOT_SEC_32_SIZE);

  unsigned int rootDirectorySectors = (rootEntryCount * 32) / 512;
  unsigned int firstRootSector = reservedSectorCount + BPB_NUM_FATS * FATSz16;
  dataStart = firstRootSector + rootDirectorySectors;
  numClusters = (totalSectors32 - dataStart) / sectorsPerCluster;

  FATSz = BPB_NUM_FATS * FATSz16 * 512;
  ROOTSz = rootEntryCount * ROOT_ENT_SZ;
  FAT = new uint8_t[FATSz];
  ROOT = new uint8_t[ROOTSz];

  imageFile.seekg(bytesPerSector * reservedSectorCount);
  imageFile.read((char*)FAT, FATSz);
  imageFile.seekg(bytesPerSector * (reservedSectorCount + BPB_NUM_FATS * FATSz16));
  imageFile.read((char*)ROOT, ROOTSz);
  imageFile.close();
  rootEnts = new vector<SVMDirectoryEntry>;
  for ( unsigned int rootEntStart = 0 ; rootEntStart < ROOTSz ; rootEntStart += ROOT_ENT_SZ )
  {
    addRootEntry(rootEntStart);
  }//fill rootentries listing with the dirents in the root directory
}//FATData constructor


FATData::~FATData()
{
  delete rootEnts;
  delete[] BPB;
  delete[] FAT;
  delete[] ROOT;
  delete[] imFileName;
}//FATData destructor


/*
typedef struct{
    char DLongFileName[VM_FILE_SYSTEM_MAX_PATH];
    char DShortFileName[VM_FILE_SYSTEM_SFN_SIZE];
    unsigned int DSize;
    unsigned char DAttributes;
    SVMDateTime DCreate;
    SVMDateTime DAccess;
    SVMDateTime DModify;
} SVMDirectoryEntry, *SVMDirectoryEntryRef;

typedef struct{
    unsigned int DYear;
    unsigned char DMonth;
    unsigned char DDay;
    unsigned char DHour;
    unsigned char DMinute;
    unsigned char DSecond;
    unsigned char DHundredth;
} SVMDateTime, *SVMDateTimeRef;
*/


void FATData::addRootEntry(unsigned int offset)
{
  SVMDirectoryEntry rootEnt;
  fillDirEnt(&rootEnt, &ROOT[offset]);
  rootEnts->push_back(rootEnt);
}//void FATData::addRootEntry(unsigned int offset)


void FATData::fatls()
{
  cout << "   DATE   |  TIME  | TYPE |    SIZE   |    SFN      |  LFN\n";
  cout << rootEnts->at(1).DModify.DYear << "/" << (int)rootEnts->at(1).DModify.DMonth << "/" << (int)rootEnts->at(1).DModify.DDay << " ";
  cout << (int)(rootEnts->at(1).DModify.DHour)%12 << ":" << (int)rootEnts->at(1).DModify.DMinute << " ";
  if (rootEnts->at(1).DModify.DHour/12)
    cout << "PM";
  else
    cout << "AM";
  cout << " ";
  if (rootEnts->at(1).DAttributes & VM_FILE_SYSTEM_ATTR_DIRECTORY)
    cout << "<Dir>  ";
  else
    cout << "<File> ";
// "xxx,x25,526" size
  cout << "xxx,xxx,xxx ";
  cout << rootEnts->at(1).DShortFileName;
}//void FATData::fatls()


void FATData::fatout()
{
  unsigned int FATSz = BPB_NUM_FATS * FATSz16 * 512;
  unsigned int ROOTSz = rootEntryCount * ROOT_ENT_SZ; //Dividing by 512 is black magic from a handout

  for (int i = 0; i < BPB_SIZE; i++)
    cout << (int)BPB[i] << ",";
  cout << "|" << endl;
  for (unsigned int i = 0; i < FATSz; i++)
    cout << (unsigned int)FAT[i] << ",";
  cout << "|" << endl;
  for (unsigned int i = 0; i < ROOTSz; i++)
    cout << (unsigned int)ROOT[i] << ",";
  cout << "|" << endl;
}//void FATData::fatout()


void FATData::fatvol()
{
  unsigned int rootDirectorySectors = (rootEntryCount * 32) / 512;
  unsigned int firstRootSector = reservedSectorCount + BPB_NUM_FATS * FATSz16;
  unsigned int firstDataSector = firstRootSector + rootDirectorySectors;
  unsigned int clusterCount = (totalSectors32 - firstDataSector) / sectorsPerCluster;
  cout << "OEM Name           : " << (char*)&BPB[3] << endl;
  cout << "Bytes Per Sector   : " << bytesPerSector << endl;
  cout << "Sectors Per Cluster: " << sectorsPerCluster << endl;
  cout << "Reserved Sectors   : " << reservedSectorCount << endl;
  cout << "FAT Count          : " << BPB_NUM_FATS << endl;
  cout << "Root Entry         : " << rootEntryCount << endl;
  cout << "Sector Count 16    : " << totalSectors16 << endl;
  cout << "Media              : " << (unsigned int)BPB[21] << endl;
  cout << "FAT Size 16        : " << FATSz16 << endl;
  cout << "Sectors Per Track  : " << bytesToUnsigned(&BPB[24], 2) << endl;
  cout << "Head Count         : " << bytesToUnsigned(&BPB[26], 2) << endl;
  cout << "Hidden Sector Count: " << bytesToUnsigned(&BPB[28], 4) << endl;
  cout << "Sector Count 32    : " << totalSectors32 << endl;
  cout << "Drive Number       : " << bytesToUnsigned(&BPB[36], 1) << endl;
  cout << "Boot Signature     : " << bytesToUnsigned(&BPB[38], 1) << endl;
  cout << "Volume ID          : " << bytesToUnsigned(&BPB[39], 4) << endl;
  cout << "Volume Label       : \"";
  cout.flush();
  write(1, (char*)&BPB[43], 11);
  cout << "\"" << endl; //currently incorrect
  cout << "File System Type   : \"" << (char*)&BPB[54] << "\"" << endl;
  cout << "Root Dir Sectors   : " << rootDirectorySectors << endl;
  cout << "First Root Sector  : " << firstRootSector << endl;
  cout << "First Data Sector  : " << firstDataSector << endl;
  cout << "Cluster Count      : " << clusterCount << endl;
}//void FATData::fatvol()


unsigned int FATData::getBytesPerSector()
{
  return bytesPerSector;
}//unsigned int FATData::getBytesPerSector()


string FATData::getFileContents(string fName /* Short file name? */)
{
  uint16_t *FATPtr;
  uint8_t dataOffset;
  unsigned int clusterSize = bytesPerSector * sectorsPerCluster;
  char *curDataCluster;
  curDataCluster = new char[clusterSize];
  unsigned int curDataSize;
  string retStr;
  
  // TODO: Search vector of root entries for file with proper name, and get starting point.
  // Store it in FATPtr and dataOffset.
  
  ifstream imageFile(imFileName, ios::in | ios::binary); // TODO: fstream method temporary. To be replaced by MachineFile function calls.
  // Note: Maybe make a wrapper function to make it easier?
  
  while (true)
  {
    imageFile.seekg(dataStart + dataOffset);
    imageFile.read(curDataCluster, clusterSize);
    // TODO: Detect how much data is in cluster. Store in curDataSize
    // curDataSize should equal clusterSize in all but the last cluster in the chain.
    retStr.append(curDataCluster, curDataSize);
    
    if (!*FATPtr || *FATPtr == 0xfff8)
    {
      break;
    }
    
    // TODO: Calculate new FATPtr location, and new data offset
  }
  imageFile.close();
  delete [] curDataCluster;
  return retStr;
}


void FATData::setFileContents(string fName, string newContents)
{
  uint16_t *FATPtr;
  uint8_t dataOffset;
  unsigned int clusterSize = bytesPerSector * sectorsPerCluster;
  
  // TODO: Find first unallocated FAT entry and store it in FATPtr.
}


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
  dt->DYear = 1980 + (((date[HI]) >> 1) & 255); //0000 0000 1111 1110
  dt->DMonth = ((date[HI] & 1) << 1) + ((date[LO] >> 5) & 7); //1110 0000 0000 0001
  dt->DDay = date[LO] & 31; //0001 1111 0000 0000
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
  dt->DHour = ((time[HI] >> 3) & 31); //0000 0000 1111 1000
  dt->DMinute = ((time[HI] & 7) << 3) + ((time[LO] >> 5) & 7);; //1110 0000 0000 0111
  dt->DSecond = 2 * (time[LO] & 31); //0001 1111 0000 0000, 2-sec count
  dt->DHundredth = dh;
  if (dh >= 100)
  {
    dt->DSecond += 1;
    dt->DHundredth -= 100;
  }//if there's between one and two hundredths of an extra second, it's a full extra second
}//void fillTime(SVMDateTimeRef dt, uint8_t time[2], unsigned char dh)


