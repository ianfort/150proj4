#include "Mutex.h"

TVMMutexID Mutex::idCounter = 0;


Mutex::Mutex()
{
  QTex = new queue<Thread*>;
  owner = NULL;
  id = idCounter;
  idCounter++;
}//Mutex::Mutex()


int Mutex::acquire(Thread* thrd, TVMTick timeout)
{
  if (owner)
  {
    if (!isInQueue(*(thrd->getIDRef())) && timeout != VM_TIMEOUT_IMMEDIATE)
    {
      QTex->push(thrd);
      return ACQUIRE_WAIT;
    }
    return ACQUIRE_UNNECESSARY;
  }
  owner = thrd;
  return ACQUIRE_SUCCESS;
}//int Mutex::acquire(Thread* thrd, TVMTick timeout)


bool Mutex::getAvailable()
{
  if (owner)
  {
    return true;
  }
  return false;
}//bool Mutex::getAvailable()


TVMMutexID Mutex::getID()
{
  return id;
}//TVMMutexID Mutex::getID()


Thread* Mutex::getOwner()
{
  return owner;
}//Thread* Mutex::getOwner()


bool Mutex::isInQueue(TVMThreadID id)
{
  Thread* temp;
  bool found = false;
  int sz = QTex->size();
  for (int i = 0; i < sz; i++)
  {
    temp = QTex->front();
    QTex->pop();
    if (*(temp->getIDRef()) == id)
      found = true;
    QTex->push(temp);
  }//cycle the wait queue completely to linear search for a thread's presence
  return found;
}//bool isInQueue(TVMThreadID id)


void Mutex::release()
{
  if (!QTex->empty())
  {
    owner = QTex->front();
    owner->stopWaiting();
    QTex->pop();
  }
  else 
  {
    owner = NULL;
  }
}//void Mutex::release()

void Mutex::waitTimeout(Thread* thrd)
{
  Thread* temp;
  int sz = QTex->size();
  for (int i = 0; i < sz; i++)
  {
    temp = QTex->front();
    QTex->pop();
    if (temp != thrd)
      QTex->push(temp);
  }//cycle the wait queue completely to find and flush the timeout'd thread from the wait queue
  thrd->stopWaiting();
}//void Mutex::waitTimeout(Thread* thrd)


