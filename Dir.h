#ifndef DIR_H
#define DIR_H

#include "VirtualMachine.h"
#include "FATData.h"

#define DIR_ROOT_INDEX 0

using namespace std;

class FATData;

class Dir
{
  SVMDirectoryEntryRef dirent;
  int dirdesc;
  unsigned int pos;
public:
  Dir(const char *dirname, int *dirdescriptor, FATData* VMFAT);
  ~Dir();
  int getDirdesc();
  int getPos();
  void incPos();
  SVMDirectoryEntryRef getDirent();
};

void fillDate(SVMDateTimeRef dt, uint8_t date[2]);
void fillDirEnt(SVMDirectoryEntryRef dir, uint8_t* loc);
void fillTime(SVMDateTimeRef dt, uint8_t time[2], unsigned char dh = 0);

#endif

