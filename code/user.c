#include "defines.h"
#include "list.h"
#include "user.h"
#include "main.h"

#ifndef NATIVE

#include "kernel.h" 

#endif /* NATIVE */

#include <stdlib.h>
#include <assert.h>

RC SysCall(SysCallType type, uval32 arg0, uval32 arg1, uval32 arg2) 
{
  uval32 returnCode;

#ifdef NATIVE  
  uval32 sysMode = SYS_ENTER;  
  asm volatile("ldw r8, %0\n\t"
	       "ldw r4, %1\n\t" 
	       "ldw r5, %2\n\t"
	       "ldw r6, %3\n\t"
	       "ldw r7, %4\n\t" 
	       "trap"
	       : : "m" (sysMode), "m" (type), "m" (arg0), "m" (arg1), "m" (arg2)
	       : "r4", "r5", "r6", "r7", "r8");  
#else /* NATIVE */
  K_SysCall(type, arg0, arg1, arg2);
#endif /* NATIVE */
  
  //returnCode = RC_SUCCESS; //Change this code to take the actual return value
  return returnCode; 
} 

RC CreateThread (uval32 pc, uval32 sp, int priority){
	return SysCall (SYS_CREATE, pc, sp, priority);
}

RC DestroyThread(ThreadId tid) {
	return SysCall (SYS_DESTROY, tid, 0, 0);
}

RC Yield(){
	return SysCall (SYS_YIELD, 0, 0, 0);
}
RC Suspend() {
	return SysCall (SYS_SUSPEND, 0, 0, 0);
}

RC ResumeThread (ThreadId tid) {
	return SysCall (SYS_RESUME, tid, 0, 0);
}

RC ChangeThreadPriority(ThreadId tid, int newPriority) {
	return SysCall (SYS_CHANGE_PRI, tid, newPriority, 0);
}

void test() {
	myprint("in thread 1\n");
	myprint("t1 change thread 1 pri to 1\n");
	ChangeThreadPriority(2, 1);
	myprint("t1 resume t2 (pri 1)\n");
	ResumeThread(3);	
	myprint("t1 change t2 pri 0\n");		
	ChangeThreadPriority(3, 0);
	myprint("muahahaha still running~~ not dead\n");		
}

void test2() {
	myprint("in thread 2\n");
	myprint("t2 resume thread 1\n");
	ResumeThread(2);
	myprint("t2 suspend\n");	
	Suspend();	
	myprint("t2 destroy t1\n");
	DestroyThread(2);		
	myprint("t2 destroy self\n");
	DestroyThread(3);		
}

void idleThread() {
	myprint("resume thread 2\n");
	ResumeThread(3);
	while (1) {
		myprint("cpu idle\n");
		Yield();	
	}
}

void mymain() 
{ 
  RC ret;

  myprint("My main\n");
  void* stack = malloc(MIN_USER_STACK_SIZE);
  void* stack2 = malloc(MIN_USER_STACK_SIZE);
  void* stack3 = malloc(MIN_USER_STACK_SIZE);
  myprint("thread 1 created priority 2, start on blocked\n");
  ret = CreateThread((uval32)test, (uval32)stack, 2); 
  myprint("thread 2 created priority 1, start on blocked\n");
  ret = CreateThread((uval32)test2, (uval32)stack2, 1);
  
  ret = CreateThread((uval32)idleThread, (uval32)stack3, MIN_PRIORITY);
  myprint("all initialized start threads\n");
  ResumeThread(4);

  
  free(stack);
  free(stack2);
  
  myprint("DONE\n");

  while(1);
}
