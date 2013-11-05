#ifndef _USER_H_
#define _USER_H_

#include "defines.h"

RC SysCall( SysCallType type, uval32 arg0, uval32 arg1, uval32 arg2);

RC CreateThread (uval32 pc, uval32 sp, int priority);
RC DestroyThread(ThreadId tid);
RC Yield();
RC Suspend();
RC ResumeThread (ThreadId tid);
RC ChangeThreadPriority(ThreadId tid, int newPriority);

void mymain(void);

#endif

