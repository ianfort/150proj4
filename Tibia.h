#ifndef TIBIA_H
#define TIBIA_H

#include "VirtualMachine.h"

class Tibia
{
  TVMThreadEntry entry;
  void* param;
public:
  Tibia(TVMThreadEntry e, void* p);
  TVMThreadEntry getEntry();
  void* getParam();
};//every skeleton needs a tibia!


#endif

