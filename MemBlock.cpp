#include "MemBlock.h"

MemBlock::MemBlock()
{
  start = NULL;
  size = 0;
}


MemBlock::MemBlock(uint8_t* strt, unsigned int sz)
{
  start = strt;
  size = sz;
}


uint8_t* MemBlock::getStart()
{
  return start;
}


unsigned int MemBlock::getSize()
{
  return size;
}
