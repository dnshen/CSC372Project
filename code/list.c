#include "defines.h"
#include "list.h"
#include "main.h"

#include <stdlib.h>

TD *CreateTD(ThreadId tid)
{
  TD *thread = (TD *)malloc(sizeof(TD));

  if(thread != NULL) {
    thread->link = NULL;
    thread->tid = tid;
    thread->priority = MIN_PRIORITY;
    thread->waittime = 0;
    thread->inlist = NULL;
    thread->returnCode = 0;

    thread->regs.pc = 0;
    thread->regs.sp = 0;
    thread->regs.sr = 0;
  } else {
    myprint("Failed to allocate new thread\n");
  }

  return thread;
}

void InitTD(TD *td, uval32 pc, uval32 sp, uval32 priority) 
{ 
  if(td != NULL) {
	td->tid = td->tid;
    td->regs.pc  = pc; 
    td->regs.sp = sp; 
    td->regs.sr  = DEFAULT_THREAD_SR; 
    td->priority = priority;
	if (td->tid != KERNEL_TID) {
		asm volatile("ldw r10, %0" : : "m" (sp));
		asm volatile("ldw r9, %0" : : "m" (pc));
		asm volatile("stw r9, 108(r10)");
	}
  } else {
    myprint("Tried to initialize NULL pointer\n");
  }
}

//allocates and proerly initializes a list structure and returns a pointer to it or null
LL* CreateList(ListType type){
	LL* list = (LL*)malloc(sizeof(LL));
	list->type = type;
	list->head = NULL;
	return list;
}

//destroys list, whose pointer is passed in as an argument.
RC DestroyList (LL *list){
	if (list == NULL) return -1;

	while (list->head != NULL) {
		TD* temp = list->head->link;
		free(list->head);
		list->head = temp;
	}
	free(list);
	return RC_SUCCESS;
}

//dequeues the TD at the head of list and returns a pointer to it, or else null
TD* DequeueHead (LL *list){
	TD* td = list->head;
	list->head = td->link;
	td->link = NULL;
	td->inlist = NULL;
	return td;
}

//if list is a priority list, then enqueues td in its proper location. 
RC PriorityEnqueue(TD *td, LL *list){
	if (list->type != L_PRIORITY) return RC_FAILED;

	if (list->head == NULL) {
		list->head = td;
	} else {
		TD* t = list->head;
		if (t->priority > td->priority) {
			td->link = t;
			list->head = td;
		} else {
			while (t->link != NULL && t->link->priority <= td->priority) {
				t = t->link;
			}
			td->link = t->link;
			t->link = td;
		}
	}
	td->inlist = list;
	return RC_SUCCESS;
}

//enqueues td at the head of list if list is a LIFO list.
RC EnqueueAtHead(TD *td, LL *list){
	if (list->type != L_LIFO) return RC_FAILED;

	td->link = list->head;
	td->inlist = list;
	list->head = td;
	return RC_SUCCESS;
}

//if list is a waiting list, then inserts td in its correct position assuming it should wait for waittime. The waittime values of the other elements in the list should be properly adjusted. 
RC WaitlistEnqueue(TD *td, int waittime, LL *list){
	if (list->type != L_WAITING) return RC_FAILED;

	TD* t = list->head;
	if (t == NULL || waittime - t->waittime < 0) {
		list->head = td;
	} else {
		while (t->link != NULL && waittime - t->waittime >= t->link->waittime ){
			waittime -= t->waittime;
			t = t->link;
		}
		waittime -= t->waittime;
		td->link = t->link;
		t->link = td;
	}
	td->waittime = waittime;	
	td->inlist = list;

	t = td->link;	
	while (t != NULL){
		t->waittime -= td->waittime;
		t = t->link;
	}	
	return RC_SUCCESS;
}

//searches list for a TD with a process id tid, and returns a pointer to it or null otherwise
TD* FindTD(ThreadId tid, LL *list){
	TD* td = list->head;
	while (td != NULL){
		if (td->tid == tid) {return td;}
		td = td->link;
	}
	return NULL;
}

//dequeues td from whatever list it might be in, if it is in one
//NOTE: whatever that gets dequeued is not freed
RC DequeueTD(TD *td ){
	if (td->inlist == NULL) return RC_FAILED;

	LL* list = td->inlist;
	TD* t = list->head;

	if (t == td) {
		list->head = td->link;
	} else {
		while (t->link != NULL && t->link != td) {
			t = t->link;
		}
		t->link = td->link;
	}

	if (list->type == L_WAITING) {
		t = td->link;
		while (t != NULL) {
			t->waittime += td->waittime;
			t = t->link;
		}
	}

	td->inlist = NULL;
	td->link = NULL;
	return RC_SUCCESS;
}

//Print the elements of the list
#ifndef NATIVE
void printList(LL* list) {
	TD* currentTD = list->head;
	printf("printing list\n");
	while (currentTD != NULL) {
		printf("tid: %d\t", currentTD->tid);
		if (list->type == L_WAITING) {
			printf("wait time: %d\t", currentTD->waittime);
		} else if (list->type == L_PRIORITY) {
			printf("priority: %d\t", currentTD->priority);
		}
		printf("\n");
		currentTD = currentTD->link;
	}
}
#endif
