#include "Thread.h"
#include "Tibia.h"
#include "Mutex.h"
#include "MPCB.h"
#include "FATData.h"
#include "Dir.h"
#include <algorithm>


using namespace std;

extern "C" TVMMainEntry VMLoadModule(const char *module);
extern "C" void VMUnloadModule(void);
void fileCallback(void* calldata, int result);
void timerISR(void*);
void scheduler();
void skeleton(void* tibia);
void idle(void*);
Mutex* findMutex(TVMMutexID id);
MPCB* findMemPool(TVMMemoryPoolID id);

vector<Thread*> *threads;
Thread *tr, *mainThread, *pt;
queue<Thread*> *readyQ[NUM_RQS];
sigset_t sigs;
vector<Mutex*> *mutexes;
uint8_t *heap;
vector<MPCB*> *pools;
uint8_t *VMPoolStart;
void* sharebase;
TVMMemoryPoolID shareid, heapid;
FATData* VMFAT;
string curPath;
vector<Dir*> *dirs;

const TVMMemoryPoolID VM_MEMORY_POOL_ID_SYSTEM = 0;

TVMStatus VMStart(int tickms, TVMMemorySize heapsize, int machinetickms, TVMMemorySize sharedsize, const char *mount,
int argc, char *argv[])
{
  TVMThreadID idletid;
  TVMMemorySize share = (sharedsize+0xFFF)&(~0xFFF);
  curPath = "/";

  TVMMainEntry mainFunc = VMLoadModule(argv[0]);
  if (!mainFunc)
  {
    return VM_STATUS_FAILURE;
  }//if mainFunc doesn't load, kill yourself

  threads = new vector<Thread*>;
  mutexes = new vector<Mutex*>;
  for (int i = 0; i < NUM_RQS; i++)
  {
    readyQ[i] = new queue<Thread*>;
  }//allocate memory for ready queues

  // Create heap.
  heap = new uint8_t[heapsize];
  pools = new vector<MPCB*>;
  VMMemoryPoolCreate(heap, heapsize, &heapid);

  VMFAT = new FATData(mount);
  dirs = new vector<Dir*>;

  mainThread = new Thread;
  mainThread->setPriority(VM_THREAD_PRIORITY_NORMAL);
  mainThread->setState(VM_THREAD_STATE_RUNNING);
  threads->push_back(mainThread);
  tr = mainThread;
  VMThreadCreate(idle, NULL, 0x100000, VM_THREAD_PRIORITY_NIL, &idletid);
  VMThreadActivate(idletid);
  if (!(sharebase = MachineInitialize(machinetickms, share)))
    return 1; //screw cleanup. Kill yourself if machine initialize fails.
  VMMemoryPoolCreate(sharebase, share, &shareid);
  MachineRequestAlarm((tickms*1000), timerISR, NULL);
  MachineEnableSignals();

  mainFunc(argc, argv);
  VMUnloadModule();
  MachineTerminate();
  for (vector<Thread*>::iterator itr = (threads->begin()+1); itr != threads->end(); itr++)
  {
    VMThreadTerminate(*((*itr)->getIDRef()));
    delete *itr;
  }//for all threads
  delete threads;
  for (vector<Mutex*>::iterator itr = mutexes->begin(); itr != mutexes->end(); itr++)
  {
    VMMutexDelete((*itr)->getID());
  }//for all threads
  delete mutexes;
  for (vector<MPCB*>::iterator itr = pools->begin() ; itr != pools->end() ; itr++)
  {
    delete *itr;
  }
  delete pools;

  return VM_STATUS_SUCCESS;
} //VMStart

//***************************************************************************//
// START VMFILE FUNCTIONS                                                    //
//***************************************************************************//
TVMStatus VMFileOpen(const char *filename, int flags, int mode, int *filedescriptor)
{
  MachineSuspendSignals(&sigs);
  tr->setcd(-18); //impossible to have a negative file descriptor
  if (filename == NULL || filedescriptor == NULL)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_PARAMETER;
  }//need to have a filename and a place to put the FD
  MachineFileOpen(filename, flags, mode, fileCallback, (void*)tr);
  tr->setState(VM_THREAD_STATE_WAITING);
  scheduler();
  if(tr->getcd() < 0)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_FAILURE;
  }//if invalid FD
  *filedescriptor = tr->getcd();
  MachineResumeSignals(&sigs);
  return VM_STATUS_SUCCESS;
} // VMFileOpen


TVMStatus VMFileClose(int filedescriptor)
{
  MachineSuspendSignals(&sigs);
  tr->setcd(1);
  // Make thread wait here.
  MachineFileClose(filedescriptor, fileCallback, (void*)tr);
  tr->setState(VM_THREAD_STATE_WAITING);
  scheduler();
  if (tr->getcd() < 0)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_FAILURE;
  }//negative result is a failure
  MachineResumeSignals(&sigs);
  return VM_STATUS_SUCCESS;
}//TVMStatus VMFileClose(int filedescriptor)


TVMStatus VMFileWrite(int filedescriptor, void *data, int *length)
{
  MachineSuspendSignals(&sigs);
  tr->setcd(-739);
  int lenleft = *length;
  int byteswritten = 0;
  char* localdata = new char[*length + 1];
  strcpy(localdata, (char*)data);
  char *writeloc;
  TVMStatus test = VM_STATUS_FAILURE;
  if (!data || !length)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_PARAMETER;
  }//not allowed to have NULL pointers for where we put the data
  while (test != VM_STATUS_SUCCESS)
  {
    MachineResumeSignals(&sigs);
    test = VMMemoryPoolAllocate(shareid, min(*length, 512), (void**)&writeloc);
    MachineSuspendSignals(&sigs);
    scheduler();//try to allocate until it works
  }
  for (int i = 0; lenleft >= 0 ; i++, lenleft -= 512)
  {
    memcpy(writeloc, &localdata[i*512], min(lenleft, 512));
    MachineFileWrite(filedescriptor, writeloc, min(lenleft, 512), fileCallback, (void*)tr);
    tr->setState(VM_THREAD_STATE_WAITING);
    scheduler();
    byteswritten += tr->getcd();
  }//cycle as needed to print everything in blocks of 512 bytes at a time
  delete localdata;
  VMMemoryPoolDeallocate(shareid, writeloc);
  tr->setcd(byteswritten);
  if(tr->getcd() < 0)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_FAILURE;
  }//if a negative number of bytes written
  MachineResumeSignals(&sigs);
  return VM_STATUS_SUCCESS;
} // VMFileWrite


TVMStatus VMFileSeek(int filedescriptor, int offset, int whence, int *newoffset)
{
  MachineSuspendSignals(&sigs);
  tr->setcd(-728);
  MachineFileSeek(filedescriptor, offset, whence, fileCallback, (void*)tr);
  tr->setState(VM_THREAD_STATE_WAITING);
  scheduler();
  if (newoffset)
  {
    *newoffset = tr->getcd();
  }//return newoffset
  if (tr->getcd() < 0)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_FAILURE;
  }//check for failure
  MachineResumeSignals(&sigs);
  return VM_STATUS_SUCCESS;
}//TVMStatus VMFileseek(int filedescriptor, int offset, int whence, int *newoffset)


TVMStatus VMFileRead(int filedescriptor, void *data, int *length)
{
  MachineSuspendSignals(&sigs);
  tr->setcd(-728);
  char* retval = new char[*length + 1];
  int lenleft = *length;
  int bytesread = 0;
  char *readloc;
  TVMStatus test = VM_STATUS_FAILURE;
  retval[*length] = '\0';
  if (!data || !length)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_PARAMETER;
  }//not allowed to be NULL pointers
  while (test != VM_STATUS_SUCCESS)
  {
    MachineResumeSignals(&sigs);
    test = VMMemoryPoolAllocate(shareid, min(*length, 512), (void**)&readloc);
    MachineSuspendSignals(&sigs);
    scheduler();//try to allocate until it works
  }
  for (int i = 0; lenleft >= 0 ; i++, lenleft -= 512)
  {
    MachineFileRead(filedescriptor, readloc, min(lenleft, 512), fileCallback, (void*)tr);
    tr->setState(VM_THREAD_STATE_WAITING);
    scheduler();
    bytesread += tr->getcd();
    memcpy(&retval[i*512], readloc, min(lenleft, 512));
  }
  tr->setcd(bytesread);
  *length = tr->getcd();
  if (tr->getcd() < 0)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_FAILURE;
  }//if the calldata was invalid
  strcpy((char*)data, retval);
  delete[] retval;
  MachineResumeSignals(&sigs);
  return VM_STATUS_SUCCESS;
}//TVMStatus VMFileseek(int filedescriptor, int offset, int whence, int *newoffset)
//***************************************************************************//
// END VMFILE FUNCTIONS                                                   //
//***************************************************************************//
//***************************************************************************//
// START VMMUTEX FUNCTIONS                                                   //
//***************************************************************************//
TVMStatus VMMutexCreate(TVMMutexIDRef mutexref)
{
  MachineSuspendSignals(&sigs);
  if (!mutexref)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_PARAMETER;
  }
  Mutex* mtx = new Mutex;
  *mutexref = mtx->getID();
  mutexes->push_back(mtx);
  MachineResumeSignals(&sigs);
  return VM_STATUS_SUCCESS;
}//TVMSTATUS VMMutexCreate(TVMMutexIDRef mutexref)


TVMStatus VMMutexAcquire(TVMMutexID mutex, TVMTick timeout)
{
  MachineSuspendSignals(&sigs);
  Mutex* mtx = findMutex(mutex);
  int acquireState;
  if (!mtx)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_ID;
  }
  else if (timeout == VM_TIMEOUT_IMMEDIATE)
  {
    acquireState = tr->acquireMutex(mtx, timeout);
    if (acquireState != ACQUIRE_SUCCESS)
    {
      MachineResumeSignals(&sigs);
      return VM_STATUS_FAILURE;
    }
    if (acquireState == ACQUIRE_UNNECESSARY)
    {
      MachineResumeSignals(&sigs);
      return VM_STATUS_ERROR_INVALID_ID;
    }
    MachineResumeSignals(&sigs);
    return VM_STATUS_SUCCESS;
  }
  else
  {
    if (tr->acquireMutex(mtx, timeout) != ACQUIRE_SUCCESS)
    {
      tr->setTicks(timeout);
      tr->setState(VM_THREAD_STATE_WAITING);
      scheduler();
    }
  }//works for both finite and infinite timeouts
  if (!tr->findMutex(mutex))
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_FAILURE;
  }//case: timeout failure
  MachineResumeSignals(&sigs);
  return VM_STATUS_SUCCESS;
}//TVMStatus VMMutexAcquire(TVMMutexID mutex, TVMTick timeout)


TVMStatus VMMutexDelete(TVMMutexID mutex)
{
  MachineSuspendSignals(&sigs);
  vector<Mutex*>::iterator mtx;
  Mutex* tmp;
  for (vector<Mutex*>::iterator itr = mutexes->begin(); itr != mutexes->end(); itr++ )
  {
    if ((*itr)->getID() == mutex)
    {
      mtx = itr;
    }
  }
  if (mtx == mutexes->end())
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_ID;
  }
  tmp = *mtx;
  if (tmp->getOwner())
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_STATE;
  }
  mutexes->erase(mtx);
  delete tmp;
  MachineResumeSignals(&sigs);
  return VM_STATUS_SUCCESS;
}


TVMStatus VMMutexQuery(TVMMutexID mutex, TVMThreadIDRef ownerref)
{
  MachineSuspendSignals(&sigs);
  Mutex* mtx;
  if (!ownerref)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_PARAMETER;
  }
  mtx = findMutex(mutex);
  if ( !mtx )
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_ID;
  }
  ownerref = mtx->getOwner()->getIDRef();
  MachineResumeSignals(&sigs);
  return VM_STATUS_SUCCESS;
}


TVMStatus VMMutexRelease(TVMMutexID mutex)
{
  MachineSuspendSignals(&sigs);
  if (!findMutex(mutex))
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_ID;
  }
  if (!tr->findMutex(mutex))
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_STATE;
  }
  tr->releaseMutex(mutex);
  scheduler();
  MachineResumeSignals(&sigs);
  return VM_STATUS_SUCCESS;
}//TVMStatus VMMutexRelease(TVMMutexID mutex)
//***************************************************************************//
// END VMMUTEX FUNCTIONS                                                     //
//***************************************************************************//
//***************************************************************************//
// START VMTHREAD FUNCTIONS                                                  //
//***************************************************************************//
TVMStatus VMThreadSleep(TVMTick tick)
{
  MachineSuspendSignals(&sigs);
  if (tick == VM_TIMEOUT_IMMEDIATE)
  {
    tr->setState(VM_THREAD_STATE_READY);
    readyQ[tr->getPriority()]->push(tr);
    scheduler();
  }// the current process yields the remainder of its processing quantum to the next ready process of equal priority.
  else if (tick == VM_TIMEOUT_INFINITE)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_PARAMETER;
  }//invalid param
  else
  {
    tr->setTicks(tick);
    tr->setState(VM_THREAD_STATE_WAITING);
    scheduler();
  } //does nothing while the number of ticks that have passed since entering this function is less than the number to sleep on
  MachineResumeSignals(&sigs);
  return VM_STATUS_SUCCESS;
}//TVMStatus VMThreadSleep(TVMTick tick)


TVMStatus VMThreadID(TVMThreadIDRef threadref)
{
  MachineSuspendSignals(&sigs);
  if (!threadref)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_PARAMETER;
  }//if threadref is a null pointer
  threadref = tr->getIDRef();
  MachineResumeSignals(&sigs);
  return VM_STATUS_SUCCESS;
}//TVMStatus VMThreadID(TVMThreadIDRef threadref)


TVMStatus VMThreadState(TVMThreadID thread, TVMThreadStateRef state)
{
  MachineSuspendSignals(&sigs);
  bool found = false;
  if (!state)
  {
    return VM_STATUS_ERROR_INVALID_PARAMETER;
  }//if stateref is a null pointer
  for (vector<Thread*>::iterator itr = threads->begin(); itr != threads->end(); itr++)
  {
    if (*((*itr)->getIDRef()) == thread)
    {
      *state = (*itr)->getState();
      found = true;
    }//found correct thread
  }//linear search through threads vector for the thread ID in question
  if (!found)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_ID;
  }//if thread wasn't a valid ID
  MachineResumeSignals(&sigs);
  return VM_STATUS_SUCCESS;
}//TVMStatus VMThreadState(TVMThreadID thread, TVMThreadStateRef state)


TVMStatus VMThreadActivate(TVMThreadID thread)
{
  MachineSuspendSignals(&sigs);
  bool found = false;
  Thread* nt = NULL;
  for (vector<Thread*>::iterator itr = threads->begin(); itr != threads->end(); itr++)
  {
    if (*((*itr)->getIDRef()) == thread)
    {
      nt = *itr;
      found = true;
    }//found correct thread
  }//linear search through threads vector for the thread ID in question
  if (!found)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_ID;
  }//if thread ID doesn't exist
  if (nt->getState() != VM_THREAD_STATE_DEAD)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_STATE;
  }//if thread isn't dead
  nt->setState(VM_THREAD_STATE_READY);
  readyQ[nt->getPriority()]->push(nt);
  MachineResumeSignals(&sigs);
  return VM_STATUS_SUCCESS;
}//TVMStatus VMThreadActivate(TVMThreadID thread)


TVMStatus VMThreadCreate(TVMThreadEntry entry, void *param, TVMMemorySize memsize, TVMThreadPriority prio, TVMThreadIDRef tid)
{
  MachineSuspendSignals(&sigs);
  SMachineContext context;
  TVMStatus test;
  uint8_t *mem;
  if (!entry || !tid)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_PARAMETER;
  }//INVALID PARAMS, must not be NULL
  while (test != VM_STATUS_SUCCESS)
  {
    MachineResumeSignals(&sigs);
    test = VMMemoryPoolAllocate(VM_MEMORY_POOL_ID_SYSTEM, memsize, (void**)&mem);
    MachineSuspendSignals(&sigs);
    scheduler();
  }//try to allocate until it works
  Thread* t = new Thread(prio, VM_THREAD_STATE_DEAD, tid, mem, memsize, entry, param);
  threads->push_back(t);
  Tibia *tibia = new Tibia(entry, param);
  MachineContextCreate(&context, skeleton, (void*)tibia, (void*)mem, memsize);
  t->setContext(context);
  MachineResumeSignals(&sigs);
  return VM_STATUS_SUCCESS;
}//TVMStatus VMThreadCreate(TVMThreadEntry entry, void *param, TVMMemorySize memsize, TVMThreadPriority prio, TVMThreadIDRef tid)


TVMStatus VMThreadDelete(TVMThreadID thread)
{
  MachineSuspendSignals(&sigs);
  Thread *del = NULL;
  vector<Thread*>::iterator killme;
  for (vector<Thread*>::iterator itr = threads->begin(); itr != threads->end(); itr++)
  {
    if (*((*itr)->getIDRef()) == thread)
    {
      del = *itr;
      killme = itr;
    }//save the correct itr for later use
  }//search through threads to find correct Thread* to delete
  if (!del)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_ID;
  }//thread ID not found
  if (del->getState() != VM_THREAD_STATE_DEAD)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_STATE;
  }//requested thread not dead
  threads->erase(killme);
  delete del;
  MachineResumeSignals(&sigs);
  return VM_STATUS_SUCCESS;
}//TVMStatus VMThreadDelete(TVMThreadID thread)


TVMStatus VMThreadTerminate(TVMThreadID thread)
{
  MachineSuspendSignals(&sigs);
  Thread *term = NULL, *temp;
  int target;
  for (vector<Thread*>::iterator itr = threads->begin(); itr != threads->end(); itr++)
  {
    if (*((*itr)->getIDRef()) == thread)
      term = *itr;
  }//search through threads to find correct Thread* to update
  if (!term)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_ID;
  }//term not found in threads
  if (term->getState() == VM_THREAD_STATE_DEAD)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_STATE;
  }//requested thread already dead
  term->setState(VM_THREAD_STATE_DEAD);
  term->releaseAllMutex();
  target = readyQ[term->getPriority()]->size();
  for (int i = 0; i < target; i++)
  {
    temp = readyQ[term->getPriority()]->front();
    readyQ[term->getPriority()]->pop();
    if (temp != term)
      readyQ[term->getPriority()]->push(temp);
  }//cycle the relevant ready queue completely to find and flush the terminated thread from the ready queue
  scheduler();
  MachineResumeSignals(&sigs);
  return VM_STATUS_SUCCESS;
}//TVMStatus VMThreadTerminate(TVMThreadID thread)
//***************************************************************************//
// END VMTHREAD FUNCTIONS                                                    //
//***************************************************************************//
//***************************************************************************//
// START MEMORY POOL FUNCTIONS                                               //
//***************************************************************************//
TVMStatus VMMemoryPoolAllocate(TVMMemoryPoolID memory, TVMMemorySize size, void **pointer)
{
  MachineSuspendSignals(&sigs);
  MPCB* foundPool;
  uint8_t* allocatedStart;
  TVMMemorySize sz = (size+0x3F)&(~0x3F);
  if (!pointer || size == 0)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_PARAMETER;
  }
  foundPool = findMemPool(memory);
  if (!foundPool)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_PARAMETER;
  }
  allocatedStart = foundPool->allocate(sz);
  if (!allocatedStart)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INSUFFICIENT_RESOURCES;
  }
  *((uint8_t**)pointer) = allocatedStart;
  MachineResumeSignals(&sigs);
  return VM_STATUS_SUCCESS;
}//TVMStatus VMMemoryPoolAllocate(TVMMemoryPoolID memory, TVMMemorySize size, void **pointer)


TVMStatus VMMemoryPoolDeallocate(TVMMemoryPoolID memory, void *pointer)
{
  MachineSuspendSignals(&sigs);
  MPCB* foundPool;
  if (!pointer)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_PARAMETER;
  }
  foundPool = findMemPool(memory);
  if (!foundPool)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_PARAMETER;
  }
  if (!foundPool->deallocate((uint8_t*)pointer))
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_PARAMETER;
  }
  MachineResumeSignals(&sigs);
  return VM_STATUS_SUCCESS;
}//TVMStatus VMMemoryPoolDeallocate(TVMMemoryPoolID memory, void *pointer)


TVMStatus VMMemoryPoolCreate(void *base, TVMMemorySize size, TVMMemoryPoolIDRef memory)
{
  MachineSuspendSignals(&sigs);
  if (!base || !memory)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_PARAMETER;
  }
  pools->push_back(new MPCB((uint8_t*)base, size));
  *memory = pools->back()->getID(); 
  MachineResumeSignals(&sigs);
  return VM_STATUS_SUCCESS;
}//TVMStatus VMMemoryPoolCreate(void *base, TVMMemorySize size, TVMMemoryPoolIDRef memory)


TVMStatus VMMemoryPoolDelete(TVMMemoryPoolID memory)
{
  MachineSuspendSignals(&sigs);
  for (vector<MPCB*>::iterator itr = pools->begin() ; itr != pools->end() ; itr++)
  {
    if ((*itr)->getID() == memory)
    {
      if (!(*itr)->fullyFree())
      {
        MachineResumeSignals(&sigs);
        return VM_STATUS_ERROR_INVALID_STATE;
      }
      delete (*itr);
      pools->erase(itr);
      MachineResumeSignals(&sigs);
      return VM_STATUS_SUCCESS;
    }
  }
  MachineResumeSignals(&sigs);
  return VM_STATUS_ERROR_INVALID_PARAMETER;
} // TVMStatus VMMemoryPoolDelete(TVMMemoryPoolID memory)


TVMStatus VMMemoryPoolQuery(TVMMemoryPoolID memory, TVMMemorySizeRef bytesleft)
{
  MachineSuspendSignals(&sigs);

  MPCB* foundPool;

  if (!bytesleft)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_PARAMETER;
  }

  foundPool = findMemPool(memory);
  if (!foundPool)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_PARAMETER;
  }

  *bytesleft = foundPool->countFree();
  MachineResumeSignals(&sigs);
  return VM_STATUS_SUCCESS;
}
//***************************************************************************//
// END MEMORY POOL FUNCTIONS                                                 //
//***************************************************************************//
//***************************************************************************//
// START FAT DIRECTORY FUNCTIONS                                             //
//***************************************************************************//
TVMStatus VMDirectoryOpen(const char *dirname, int *dirdescriptor)
{
  MachineSuspendSignals(&sigs);
  if (!dirname || !dirdescriptor)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_PARAMETER;
  }//if pointers are NULL
  //dirname is absolute path of directory
  //dirdescriptor: index of directory in data (0 for root)
  Dir *dir = new Dir(dirname, dirdescriptor, VMFAT);
  dirs->push_back(dir);
  MachineResumeSignals(&sigs);
  return VM_STATUS_SUCCESS;
}//TVMStatus VMDirectoryOpen(const char *dirname, int *dirdescriptor)


TVMStatus VMDirectoryClose(int dirdescriptor)
{
  MachineSuspendSignals(&sigs);
  Dir *dir;
  for (vector<Dir*>::iterator itr = dirs->begin(); itr != dirs->end(); itr++)
  {
    if ((*itr)->getDirdesc() == dirdescriptor)
    {
      dir = *itr;
      dirs->erase(itr);
      delete dir;
      break;
    }
  }
  MachineResumeSignals(&sigs);
  return VM_STATUS_SUCCESS;
}//TVMStatus VMDirectoryClose(int dirdescriptor)


TVMStatus VMDirectoryRead(int dirdescriptor, SVMDirectoryEntryRef dirent)
{
  MachineSuspendSignals(&sigs);
  MachineResumeSignals(&sigs);
  return VM_STATUS_SUCCESS;
}//TVMStatus VMDirectoryRead(int dirdescriptor, SVMDirectoryEntryRef dirent)


TVMStatus VMDirectoryRewind(int dirdescriptor)
{
  MachineSuspendSignals(&sigs);
  MachineResumeSignals(&sigs);
  return VM_STATUS_SUCCESS;
}//TVMStatus VMDirectoryRewind(int dirdescriptor)


TVMStatus VMDirectoryCurrent(char *abspath)
{
  MachineSuspendSignals(&sigs);
  if (!abspath)
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_ERROR_INVALID_PARAMETER;
  }//if abspath is a NULL pointer
  strcpy(abspath, curPath.c_str());
  MachineResumeSignals(&sigs);
  return VM_STATUS_SUCCESS;
}//TVMStatus VMDirectoryCurrent(char *abspath)


TVMStatus VMDirectoryChange(const char *path)
{
  MachineSuspendSignals(&sigs);
  if (!strcmp(path, ".") || !strcmp(path, "/") || !strcmp(path, "./"))
  {
    MachineResumeSignals(&sigs);
    return VM_STATUS_SUCCESS;
  }
  MachineResumeSignals(&sigs);
  return VM_STATUS_FAILURE;
}//TVMStatus VMDirectoryChange(const char *path)


TVMStatus VMDirectoryCreate(const char *dirname)
{//EXTRA CREDIT
  MachineSuspendSignals(&sigs);
  MachineResumeSignals(&sigs);
  return VM_STATUS_SUCCESS;
}//TVMStatus VMDirectoryCreate(const char *dirname)


TVMStatus VMDirectoryUnlink(const char *path)
{//EXTRA CREDIT
  MachineSuspendSignals(&sigs);
  MachineResumeSignals(&sigs);
  return VM_STATUS_SUCCESS;
}//TVMStatus VMDirectoryUnlink(const char *path)
//***************************************************************************//
//END FAT DIRECTORY FUNCTIONS                                                //
//***************************************************************************//
//***************************************************************************//
// START UTILITY FUNCTIONS                                                   //
//***************************************************************************//
void fileCallback(void* calldata, int result)
{
  Thread* cbt = (Thread*)calldata;
  cbt->setcd(result);
  while (cbt->getState() != VM_THREAD_STATE_WAITING);
  cbt->setState(VM_THREAD_STATE_READY);
  readyQ[cbt->getPriority()]->push(cbt);
}//void fileCallback(void* calldata, int result)


void timerISR(void*)
{
  MachineSuspendSignals(&sigs);
  for (vector<Thread*>::iterator itr = threads->begin(); itr != threads->end(); itr++)
  {
    (*itr)->decrementTicks();
  }//add one tick passed to every thread
  MachineResumeSignals(&sigs);
  scheduler();
}//Timer ISR: Do Everything!


void scheduler()
{
  pt = tr;
  if (pt->getState() == VM_THREAD_STATE_RUNNING)
  {
    pt->setState(VM_THREAD_STATE_READY);
    readyQ[pt->getPriority()]->push(pt);
  }//return non-blocked old thread to ready queues
  if (!readyQ[VM_THREAD_PRIORITY_HIGH]->empty())
  {
    tr = readyQ[VM_THREAD_PRIORITY_HIGH]->front();
    readyQ[VM_THREAD_PRIORITY_HIGH]->pop();
  }//if there's anything in hi-pri, run it
  else if (!readyQ[VM_THREAD_PRIORITY_NORMAL]->empty())
  {
    tr = readyQ[VM_THREAD_PRIORITY_NORMAL]->front();
    readyQ[VM_THREAD_PRIORITY_NORMAL]->pop();
  }//if there's anything in med-pri, run it
  else if (!readyQ[VM_THREAD_PRIORITY_LOW]->empty())
  {
    tr = readyQ[VM_THREAD_PRIORITY_LOW]->front();
    readyQ[VM_THREAD_PRIORITY_LOW]->pop();
  }//if there's anything in low-pri, run it
  else
  {
    tr = readyQ[VM_THREAD_PRIORITY_NIL]->front();
    readyQ[VM_THREAD_PRIORITY_NIL]->pop();
  }//if there's nothing in any of the RQs, spin with the idle process
  tr->setState(VM_THREAD_STATE_RUNNING);
  MachineResumeSignals(&sigs);
  MachineContextSwitch(pt->getContextRef(), tr->getContextRef());
}//void scheduler()


void skeleton(void *tibia)
{
  TVMThreadEntry func = ((Tibia*)tibia)->getEntry();
  func(((Tibia*)tibia)->getParam());
  delete (Tibia*)tibia;
  VMThreadTerminate(*(tr->getIDRef()));
}//3spoopy5me


void idle(void*)
{
  while(1);
}//idles forever!


Mutex* findMutex(TVMMutexID id)
{
  for (vector<Mutex*>::iterator itr = mutexes->begin() ; itr != mutexes->end() ; itr++)
  {
    if ( (*itr)->getID() == id )
    {
      return *itr;
    }
  }
  return NULL;
}//Mutex* findMutex(TVMMutexID id)


MPCB* findMemPool(TVMMemoryPoolID id)
{
  for (vector<MPCB*>::iterator itr = pools->begin() ; itr != pools->end() ; itr++)
  {
    if ((*itr)->getID() == id)
    {
      return (*itr);
    }
  }
  return NULL;
}//MPCB* findMemPool(TVMMemoryPoolID id)


//***************************************************************************//
// END UTILITY FUNCTIONS                                                     //
//***************************************************************************//

