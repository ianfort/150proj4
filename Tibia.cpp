#include "Tibia.h"

Tibia::Tibia(TVMThreadEntry e, void* p)
{
  entry = e;
  param = p;
}//constructor


TVMThreadEntry Tibia::getEntry()
{
  return entry;
}//TVMThreadEntry Tibia::getEntry()


void* Tibia::getParam()
{
  return param;
}//void* Tibia::getParam()


