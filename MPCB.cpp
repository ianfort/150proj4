#include "MPCB.h"

unsigned int MPCB::idInc = 0;

MPCB::MPCB(uint8_t *plStrt, unsigned int plSz)
{
  id = idInc;
  start = plStrt;
  size = plSz;
  allocated = new vector<MemBlock>;
  //subBlocks = new vector<MPCB*>;

  idInc++;
/* heap = new uint8_t[hpSz];
 isFree = new bool[hpSz];
  for (unsigned int i = 0 ; i < shSz ; i++ )
  {
    isFree[i] = false;
  }
  for (unsigned int i = shSz ; i < hpSz ; i++ )
  {
    isFree[i] = true;
  }
  heapsz = hpSz;
  sharedsz = shSz;
*/
}

MPCB::~MPCB()
{
  delete allocated;
}


uint8_t* MPCB::allocate(unsigned int spaceSize)
{
  uint8_t* candidateStart = start;
  vector<MemBlock>::iterator newBlockItr;
  if (allocated->empty())
  {
    if (spaceSize <= size)
    {
      allocated->push_back(MemBlock(candidateStart, spaceSize));
      return allocated->back().getStart();
    } // If spaceSize is smaller or equal in size to the memory pool.
    return NULL;
  } // If no space allocated yet
  else
  {
    for (vector<MemBlock>::iterator itr = allocated->begin() ; itr != allocated->end() ; itr++)
    {
      if (spaceSize <= (unsigned int)((*itr).getStart() - candidateStart))
      {
        newBlockItr = allocated->insert(itr, MemBlock(candidateStart, spaceSize));
        return (*newBlockItr).getStart();
      }
      candidateStart = (*itr).getStart() + (*itr).getSize();
    }
  } // If there is memory already allocated
  if ( spaceSize <= (unsigned int)((start + size) - candidateStart) )
  {
    allocated->push_back(MemBlock(candidateStart, spaceSize));
    return allocated->back().getStart();
  }
  return NULL;
}


bool MPCB::deallocate(uint8_t* strt)
{
  for (vector<MemBlock>::iterator itr = allocated->begin() ; itr != allocated->end() ; itr++)
  {
    if ((*itr).getStart() == strt)
    {
      allocated->erase(itr);
      return true;
    }
  }
  return false;
} // Note: Does not delete memory MPCB. MPCB cleanup handled in VirtualMachine.cpp

/*
MPCB* MPCB::findSubBlock(unsigned int ident)
{
  for (vector<MPCB*>::iterator itr = subBlocks->begin() ; itr != subBlocks->end() ; itr++)
  {
    if ((*itr)->getID() == ident)
    {
      return *itr;
    }
  }
  return NULL;
}


void MPCB::insertSubBlock(MPCB* m)
{
  subBlocks->push_back(m);
}


void MPCB::removeSubBlock(unsigned int ident)
{
  for (vector<MPCB*>::iterator itr = subBlocks->begin() ; itr != subBlocks->end() ; itr++)
  {
    if ((*itr)->getID() == ident)
    {
      subBlocks->erase(itr);
      break;
    }
  }
}
*/

unsigned int MPCB::getID()
{
  return id;
}


uint8_t* MPCB::getStart()
{
  return start;
}


unsigned int MPCB::getSize()
{
  return size;
}


bool MPCB::fullyFree()
{
  return allocated->empty();
}


unsigned int MPCB::countFree()
{
  unsigned int numAllocated = 0;
  for (vector<MemBlock>::iterator itr = allocated->begin() ; itr != allocated->end() ; itr++)
  {
    numAllocated += (*itr).getSize();
  }

  return size - numAllocated;
}

