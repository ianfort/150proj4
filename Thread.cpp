#include "Thread.h"

TVMThreadID Thread::nextID = 0;

Thread::Thread()
{
  id = nextID;
  nextID++;
  stackBase = NULL;
  heldMutex = new vector<Mutex*>;
  waiting = NULL;
}//default/empty constructor

Thread::Thread(const TVMThreadPriority &pri, const TVMThreadState &st, TVMThreadIDRef tid, uint8_t *sb,
               TVMMemorySize ss, const ThreadEntry &entryFunc, void *p)
{
  id = nextID;
  nextID++;
  heldMutex = new vector<Mutex*>;
  waiting = NULL;
  priority = pri;
  state = st;
  *tid = id;
  stackBase = sb;
  stackSize = ss;
  entry = entryFunc;
  param = p;
  ticks = -1;
  cd = -5;
}//constructor


Thread::~Thread()
{
  if (stackBase)
    delete stackBase;
  delete heldMutex;
}//Default destructor


int Thread::acquireMutex(Mutex* mtx, TVMTick timeout)
{
  waiting = NULL;
  if (!findMutex(mtx->getID()))
  {
    if (mtx->acquire(this, timeout) == ACQUIRE_SUCCESS)
    {
      heldMutex->push_back(mtx);
      return ACQUIRE_SUCCESS;
    }
    waiting = mtx;
    return ACQUIRE_WAIT;
  } // !findMutex(mtx->getID())
  return ACQUIRE_UNNECESSARY;
}//bool Thread::acquireMutex(Mutex* mtx)


void Thread::decrementTicks()
{
  ticks--;
  if (ticks == 0 && state == VM_THREAD_STATE_WAITING)
  {
    state = VM_THREAD_STATE_READY;
    if (waiting)
    {
      waiting->waitTimeout(this);
    }//stop waiting on mutex due to TIMEOUT
    readyQ[priority]->push(this);
  }//if no need to be asleep
  if (ticks < 0)
  {
    ticks = -5;
  }
}//void Thread::decrementTicks()


Mutex* Thread::findMutex(TVMMutexID id)
{
  for (vector<Mutex*>::iterator itr = heldMutex->begin(); itr != heldMutex->end(); itr++)
  {
    if ((*itr)->getID() == id)
    {
      return *itr;
    }//return ptr to mutex
  }//linear search threough mutexes
  return NULL;
}//Mutex* Thread::findMutex(TVMMutexID id)


volatile int Thread::getcd()
{
  return cd;
}//volatile int Thread::getcd()


SMachineContext* Thread::getContextRef()
{
  return &context;
}//Thread::SMachineContext* getContextRef()


TVMThreadEntry Thread::getEntry()
{
  return entry;
}//TVMThreadEntry Thread::getEntry()


TVMThreadIDRef Thread::getIDRef()
{
  return &id;
}//TVMThreadID Thread::getIDRef()


TVMThreadPriority Thread::getPriority()
{
  return priority;
}//TVMThreadPriority Thread::getPriority()


volatile TVMThreadState Thread::getState()
{
  return state;
}//TVMThreadState Thread::getState()


volatile int Thread::getTicks()
{
  return ticks;
}//volatile int Thread::getTicks()


void Thread::releaseAllMutex()
{
  Mutex* mtx;
  while (!heldMutex->empty())
  {
    mtx = heldMutex->back();
    mtx->release();
    heldMutex->pop_back();
  }
}//void Thread::releaseAllMutex()


bool Thread::releaseMutex(TVMMutexID id)
{
  for (vector<Mutex*>::iterator itr = heldMutex->begin() ; itr != heldMutex->end() ; itr++)
  {
    if ((*itr)->getID() == id)
    {
      (*itr)->release();
      heldMutex->erase(itr);
      return true;
    }//if the mutex is found, release it
  }//linear search for mutex
  return false;
}//bool Thread::releaseMutex(TVMMutexID id)


void Thread::setcd(volatile int calldata)
{
  cd = calldata;
}//void Thread::setcd(volatile int calldata)


void Thread::setContext(SMachineContext c)
{
  context = c;
}//void Thread::setContext(SMachineContext c)


void Thread::setID(TVMThreadID newID)
{
  id = newID;
}//Thread::TVMThreadID setID(TVMThreadID newID)


void Thread::setPriority(TVMThreadPriority pri)
{
  priority = pri;
}//void Thread::setPriority(TVMThreadPriority pri)


void Thread::setState(TVMThreadState newstate)
{
  state = newstate;
}//void Thread::setState(TVMThreadState newstate)


void Thread::setTicks(volatile int newticks)
{
  ticks = newticks;
}//void Thread::setTicks(int newticks)


void Thread::stopWaiting()
{
  waiting = NULL;
  state = VM_THREAD_STATE_READY;  
  readyQ[priority]->push(this);
}//void Thread::stopWaiting()


