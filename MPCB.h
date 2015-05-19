#ifndef MPCB_H
#define MPCB_H

#include <vector>
#include <stdint.h>
#include <cstddef>
#include <cassert>
#include <iostream>

#include "MemBlock.h"

using namespace std;

class MPCB
{
  static unsigned int idInc;
  unsigned int id;
  uint8_t *start;
  unsigned int size;
  vector<MemBlock> *allocated;
public:
  MPCB(uint8_t *plStrt, unsigned int plSz);
  ~MPCB();
  uint8_t* allocate(unsigned int size);
  bool deallocate(uint8_t* strt);
  unsigned int getID();
  uint8_t* getStart();
  unsigned int getSize();
  bool fullyFree();
  unsigned int countFree();
};

#endif
