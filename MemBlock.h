#ifndef MEMBLOCK_H
#define MEMBLOCK_H

#include <stdint.h>
#include <cstddef>

class MemBlock
{
  uint8_t *start;
  unsigned int size;
public:
  MemBlock();
  MemBlock(uint8_t* strt, unsigned int sz);

  uint8_t* getStart();
  unsigned int getSize();
};

#endif
