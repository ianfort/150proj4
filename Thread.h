#ifndef THREAD_H
#define THREAD_H

#define ACQUIRE_SUCCESS 0
#define ACQUIRE_WAIT 1
#define ACQUIRE_UNNECESSARY (-1)

#include <unistd.h>
#include <vector>
#include "VirtualMachine.h"
#include "Machine.h"
#include <iostream>
#include <queue>
#include <cstring>
#include "Mutex.h"

using namespace std;


#define NUM_RQS                                 4
#define VM_THREAD_PRIORITY_NIL                  ((TVMThreadPriority)0x00)

class Thread;
extern queue<Thread*> *readyQ[NUM_RQS];

typedef void (*ThreadEntry)(void *param);

class Mutex;

class Thread
{
  static TVMThreadID nextID; //increment every time a thread is created. Decrement never.
  SMachineContext context;
  TVMThreadPriority priority;
  volatile TVMThreadState state;
  TVMThreadID id;
  volatile int ticks;
  uint8_t *stackBase;
  TVMMemorySize stackSize;
  TVMThreadEntry entry;
  void *param;
  volatile int cd; //calldata
  vector<Mutex*> *heldMutex;
  Mutex* waiting;
public:
  Thread();
  Thread(const TVMThreadPriority &pri, const TVMThreadState &st, TVMThreadIDRef tid, uint8_t *sb, TVMMemorySize ss, const ThreadEntry &entryFunc, void *p);
  ~Thread();
  int acquireMutex(Mutex* mtx, TVMTick timeout); // m
  void decrementTicks();
  Mutex* findMutex(TVMMutexID id); // m
  volatile int getcd();
  SMachineContext* getContextRef();
  TVMThreadEntry getEntry();
  TVMThreadIDRef getIDRef();
  TVMThreadPriority getPriority();
  volatile TVMThreadState getState();
  volatile int getTicks();
  void releaseAllMutex();
  bool releaseMutex(TVMMutexID); // m
  void setcd(volatile int calldata);
  void setContext(SMachineContext c);
  void setID(TVMThreadID newID);
  void setPriority(TVMThreadPriority pri);
  void setState(TVMThreadState newstate);
  void setTicks(volatile int newticks);
  void stopWaiting();
};


#endif

