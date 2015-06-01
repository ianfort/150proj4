#ifndef FATDATA_H
#define FATDATA_H

#define BPB_SIZE 62
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
#define DIRENT_CRT_DATE_OFFSET 16
#define DIRENT_CRT_DATE_SZ 2
#define DIRENT_CRT_TIME_OFFSET 14
#define DIRENT_CRT_TIME_SZ 2
#define DIRENT_CRT_TIME_CS_OFFSET 13
#define DIRENT_CRT_TIME_CS_SZ 1
#define DIRENT_ACC_DATE_OFFSET 18
#define DIRENT_ACC_DATE_SZ 2
#define DIRENT_WRT_DATE_OFFSET 24
#define DIRENT_WRT_DATE_SZ 2
#define DIRENT_WRT_TIME_OFFSET 22
#define DIRENT_WRT_TIME_SZ 2
#define DIRENT_ATTR_OFFSET 11
#define DIRENT_ATTR_SZ 1
#define DIRENT_FILESIZE_OFFSET 28
#define DIRENT_FILESIZE_SZ 4
#define DIRENT_ATTR_OFFSET 11
#define DIRENT_ATTR_SZ 1
#define ATTR_LONG_NAME (0x01 | 0x02 | 0x04 | 0x08)
#define LO 0
#define HI 1

#include <stdint.h>
#include "VirtualMachine.h"
#include <cstring>
#include <vector>
#include <string>

using namespace std;


class FATData
{
  unsigned int bytesPerSector;
  unsigned int sectorsPerCluster;
  unsigned int reservedSectorCount;
  unsigned int rootEntryCount;
  unsigned int totalSectors16;
  unsigned int FATSz16;
  unsigned int totalSectors32;
  unsigned int dataStart;
  unsigned int numClusters;
  char *imFileName;
  uint8_t* BPB;
  uint16_t* FAT;
  uint8_t* ROOT;
  vector<SVMDirectoryEntry> *rootEnts;
  vector<unsigned int> *fileStarts;
public:
  FATData(const char* mount);
  ~FATData();
  void addRootEntry(unsigned int offset);
  bool changeFileContents(uint16_t* fileStart);
  void fatls();
  void fatout();
  void fatvol();
  unsigned int getBytesPerSector();
  uint8_t* getROOT();
  bool readFromFile(string fName, unsigned int length, string* ret);
  bool writeToFile(string fName, string newContents);
};

unsigned int bytesToUnsigned(uint8_t* start, unsigned int size);
string convertFNameToShort(string toConvert);

#endif
