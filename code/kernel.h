#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "defines.h"
#include "list.h"

typedef struct type_STACK Stack;

struct type_STACK 
{ 
  uval8 stack[STACKSIZE]; 
};

extern TD *Active, Kernel;
extern LL *ReadyQ, *BlockedQ, *FreeQ;

void InitReadyQ();
void InitBlockedQ();
void InitFreeQ();

void ScheduleNextThread();

RC K_CreateThread( uval32 pc, uval32 sp, uval32 priority );
RC K_DestroyThread( uval32 tid);
RC K_Yield();
RC K_Suspend();
RC K_ResumeThread(uval32 tid);
RC K_ChangeThreadPriority(uval32 tid, uval32 priority);

void Idle(void);
void InitKernel(void);  

void K_SysCall( SysCallType type, uval32 arg0, uval32 arg1, uval32 arg2);
extern void SysCallHandler(SysCallType type, uval32 arg0, uval32 arg1, uval32 arg2);
#endif
