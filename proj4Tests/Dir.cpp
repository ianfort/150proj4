#include "Dir.h"


Dir::Dir(const char *dirname, int *dirdescriptor, FATData* VMFAT)
{
  dirent = new SVMDirectoryEntry;
  //go from dirname to pointer location -- currently doing no EC, so assuming the dir is always Root
  if (!strcmp(dirname, ".") || !strcmp(dirname, "/") || !strcmp(dirname, "./"))
  {
    *dirdescriptor = dirdesc = DIR_ROOT_INDEX;
    fillDirEnt(dirent, VMFAT->getROOT());
  }//only works for root right now because not sure how to get pointer loc from dirname
  pos = 0;
}//Dir Constructor


Dir::~Dir()
{
  delete dirent;
}//Dir::~Dir()


int Dir::getDirdesc()
{
  return dirdesc;
}//int Dir::getDirdesc()


SVMDirectoryEntryRef Dir::getDirent()
{
  return dirent;
}//SVMDirectoryEntryRef Dir::getDirent()


unsigned int Dir::getPos()
{
  return pos;
}//int Dir::getPos()


void Dir::incPos()
{
  pos++;
}//void Dir::incPos()


void Dir::rewind()
{
  pos = 0;
}//void Dir::rewind()

//***************************************************************************//
// Begin Utility Functions for Dir                                           //
//***************************************************************************//
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
  if (*(loc+DIRENT_NAME_OFFSET+8) == ' ')
    dir->DShortFileName[8] = ' ';
  else
    dir->DShortFileName[8] = '.';
  memcpy((dir->DShortFileName)+9, loc+DIRENT_NAME_OFFSET+8, 3);
  dir->DShortFileName[VM_FILE_SYSTEM_SFN_SIZE-1] = '\0';
  dir->DSize = ((*(loc + DIRENT_FILESIZE_OFFSET + 0) << (8*0)) + 
                (*(loc + DIRENT_FILESIZE_OFFSET + 1) << (8*1)) +
                (*(loc + DIRENT_FILESIZE_OFFSET + 2) << (8*2)) +
                (*(loc + DIRENT_FILESIZE_OFFSET + 3) << (8*3)));
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


