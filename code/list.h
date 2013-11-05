#ifndef _LIST_H_
#define _LIST_H_

#include "defines.h"

typedef enum { UNDEF, L_PRIORITY, L_LIFO, L_WAITING} ListType ;

#define MIN_PRIORITY 100

typedef struct type_LL LL;
typedef struct type_TD TD;
typedef struct type_REGS Registers;

extern TD* Active;

struct type_REGS
{
  uval32 sp;
  uval32 pc;
  uval32 sr;
};

struct type_LL
{
  TD *head;
  ListType type;
};

struct type_TD
{
  TD * link;
  ThreadId tid;
  Registers regs;
  int priority;
  int waittime;
  RC returnCode;
  LL * inlist;
};

TD *CreateTD(ThreadId tid);
void InitTD(TD *td, uval32 pc, uval32 sp, uval32 priority);

//allocates and properly initializes a list structure and returns a pointer to it or null
LL* CreateList(ListType type);

//destroys list, whose pointer is passed in as an argument.
RC DestroyList (LL *list);

//dequeues the TD at the head of list and returns a pointer to it, or else null
TD* DequeueHead (LL *list);

//if list is a priority list, then enqueues td in its proper location. 
//if same pri, placed after
RC PriorityEnqueue(TD *td, LL *list);

//enqueues td at the head of list if list is a LIFO list.
RC EnqueueAtHead(TD *td, LL *list);

//if list is a waiting list, then inserts td in its correct position assuming it should wait for waittime. The waittime values of the other elements in the list should be properly adjusted.
//If same wait time, placed after
RC WaitlistEnqueue(TD *td, int waittime, LL *list);

TD* FindTD(ThreadId tid, LL *list);

//Dequeues td from whatever list it might be in, if it is in one
//NOTE: Dequeue does not free pointer
RC DequeueTD(TD *td );

#ifndef NATIVE
//prints the elements of the list
void printList(LL* list);
#endif
#endif
