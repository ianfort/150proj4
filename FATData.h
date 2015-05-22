#ifndef FATDATA_H
#define FATDATA_H

class FATData
{
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

  uint8_t* BPB;
public:
  FATData(const char* mount);
  ~FATData();

  unsigned int getBytesPerSector();
  unsigned int bytesToUnsigned(uint8_t* start, size);
};

#endif
