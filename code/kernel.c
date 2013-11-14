#include "defines.h"
#include "list.h"
#include "kernel.h"
#include "main.h"

#include <stdlib.h>
#include <assert.h>

TD *Active, Kernel;
LL *ReadyQ, *BlockedQ, *FreeQ;
Stack KernelStack;

void InitReadyQ() {
	ReadyQ = CreateList(L_PRIORITY);
}

void InitBlockedQ() {
	BlockedQ = CreateList(L_PRIORITY);
}

void InitFreeQ() {
	FreeQ = CreateList(L_PRIORITY);
	int tid = USER_TID_START;
	for (tid; tid <= USER_TID_END; tid++) {
		TD *thread = CreateTD(tid);
		PriorityEnqueue (thread, FreeQ);
	}
}

void InitKernel(void) {
  Active = CreateTD(KERNEL_TID);
  InitTD(Active, 0, 0, 1);  //Will be set with proper return registers on context switch

  InitReadyQ();
  InitBlockedQ();
  InitFreeQ();

#ifdef NATIVE
  InitTD(&Kernel, (uval32) SysCallHandler, (uval32) &(KernelStack.stack[STACKSIZE]), 0);
  Kernel.regs.sr = DEFAULT_KERNEL_SR;
#endif /* NATIVE */
}



void K_SysCall( SysCallType type, uval32 arg0, uval32 arg1, uval32 arg2) 
{ 
#ifdef NATIVE
  asm(".align 4; .global SysCallHandler; SysCallHandler:");
  uval32 sysMode = SYS_EXIT;
#endif

  RC returnCode ; 
  switch( type ) {
    case SYS_CREATE: 
      returnCode = K_CreateThread( arg0, arg1, arg2 ) ; 
    break ;
    
    case SYS_DESTROY:
      returnCode = K_DestroyThread( arg0 );
    break;
    
    case SYS_YIELD:
      returnCode = K_Yield();
    break;
    
    case SYS_SUSPEND:
      returnCode = K_Suspend();
    break;
    
    case SYS_RESUME:
      returnCode = K_ResumeThread( arg0 );
    break;
    
    case SYS_CHANGE_PRI:
      returnCode = K_ChangeThreadPriority( arg0, arg1 );
    break;
        
  default:
    myprint("Invalid SysCall type\n");
    returnCode = RC_FAILED;
    break;
  } 
  
  myprintdebug("Scheduling next thread\n");
  myprintdebug("readyQ\n");
  printList(ReadyQ);
  myprintdebug("blockedQ\n");
  printList(BlockedQ);

  ScheduleNextThread();
  
#ifdef NATIVE
  asm volatile("ldw r8, %0" : : "m" (sysMode): "r8");
  asm( "trap" );
#endif /* NATIVE */
}

void ScheduleNextThread() {
  if (ReadyQ->head == NULL) {
	myprintdebug("Ready queue empty\n");
	return;
  }
  if (Active != NULL) {
	if (Active->priority > ReadyQ->head->priority) {
		myprintdebug("switching to higher pri thread\n");
		if (Active->tid != KERNEL_TID) {
			myprintdebug("moving active to ready q\n");
			PriorityEnqueue(Active, ReadyQ); 
		}
		Active = NULL;
	  }
  }
  
  if (Active == NULL || Active->tid == KERNEL_TID) {
    myprintdebug("setting new active thread\n");
    Active = DequeueHead(ReadyQ);
  }
  
  if (Active->tid == 2) {
	myprintdebug("setting active to test\n");
  }
   myprint("done scheduling\n");
}


RC K_CreateThread(uval32 pc, uval32 sp, uval32 priority) { 
  TD *thread = DequeueHead (FreeQ);
  if (thread == NULL) {
    myprint("No TD Available \n");
    return RESOURCE_ERROR;
  } else if (priority < 0 || priority > MIN_PRIORITY) {
    myprint("Priority out of range \n");
    return PRIORITY_ERROR;
  } else {
    myprint("CreateThread \n");
    InitTD(thread, pc, sp, priority);
    PriorityEnqueue (thread, BlockedQ);
    return RC_SUCCESS;
  }

//  RC sysReturn = RC_SUCCESS;
//  myprint("CreateThread ");
//  return sysReturn;
}

RC K_DestroyThread(uval32 tid){
  if (tid == 0 || tid == Active->tid) {
    PriorityEnqueue (Active, FreeQ);
    Active = NULL;
    myprint("active thread destroyed \n");
    return RC_SUCCESS;
  } 
  
  TD* thread = FindTD (tid, ReadyQ);
  if (thread == NULL) thread = FindTD (tid, BlockedQ);
  if (thread == NULL) {
    myprint("thread is already dead \n");
    return TID_ERROR;
  } else {
      DequeueTD(thread);
      PriorityEnqueue(thread, FreeQ);
      myprint("thread destroyed \n");
      return RC_SUCCESS;
  }
}

RC K_Yield(){
  PriorityEnqueue (Active, ReadyQ);
  Active = NULL;
  myprint("active thread yielded\n");
  return RC_SUCCESS;
}

RC K_Suspend(){
  PriorityEnqueue (Active, BlockedQ);
  Active = NULL;
  myprint("active thread suspended\n");
  return RC_SUCCESS;
}

RC K_ResumeThread(uval32 tid){
  if (tid < USER_TID_START || tid > USER_TID_END) {
    myprint("thread id not available\n");
    return TID_ERROR;
  }
  
  TD* thread = FindTD(tid, BlockedQ);
  if (thread == NULL) {
    myprint("thread is not blocked\n");
    return NOT_BLOCKED;
  } else {
    DequeueTD(thread);
    PriorityEnqueue(thread, ReadyQ);
    myprint("thread resumed\n");
    return RC_SUCCESS;
  }
}

RC K_ChangeThreadPriority(uval32 tid, uval32 priority){
   if (tid < USER_TID_START || tid > USER_TID_END) {
    myprint("thread id not available\n");
    return TID_ERROR;
  } else if (priority < 0 || priority > MIN_PRIORITY) {
    myprint("Priority out of range \n");
    return PRIORITY_ERROR;
  }
  
  TD* thread = NULL;
  if (Active->tid == tid) {
    thread = Active;
	thread->priority = priority;
  } else {
    TD* thread = FindTD(tid, ReadyQ);
	if (thread == NULL) {
      thread = FindTD(tid, BlockedQ);
	  DequeueTD(thread);
	  thread->priority = priority;
	  PriorityEnqueue(thread, BlockedQ);
    } else {
	  DequeueTD(thread);
	  thread->priority = priority;
	  PriorityEnqueue(thread, ReadyQ);
	}
  }

  myprint("Priority changed \n");
  return RC_SUCCESS;
}

void Idle() 
{ 
  /*
  int i; 
  while( 1 ) 
    { 
      myprint( "CPU is idle\n" ); 
      for( i = 0; i < MAX_THREADS; i++ ) 
	{ 
	} 
      Yield(); 
    } 
  */
} 
