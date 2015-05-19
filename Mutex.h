#ifndef MUTEX_H
#define MUTEX_H


#include "Thread.h"
#include "VirtualMachine.h"
#include <queue>

using namespace std;

// typedef unsigned int TVMMutexID, *TVMMutexIDRef;

class Thread;

class Mutex
{
  static TVMMutexID idCounter;
  TVMMutexID id; // the id of this mutex
//  bool available; // Whether the mutex is availabe
  queue<Thread*> *QTex;
  Thread* owner;
  // queue<Thread*> *qtex; // Waiting queue for threads trying to acquire this mutex
public:
  Mutex();
  int acquire(Thread* thrd, TVMTick timeout);
  bool getAvailable();
  TVMMutexID getID();
  Thread* getOwner();
  bool isInQueue(TVMThreadID id);
  void release();
  void waitTimeout(Thread* thrd);
};

#endif
