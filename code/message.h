#ifndef _MESSAGE_H_
#define _MESSAGE_H_

typedef struct type_mailbox Mailbox;
typedef struct type_msg Msg;

struct type_mailbox
{
	Msg* head;
};

struct type_msg
{
	ThreadId senderId;
	ThreadId receiverId;
	int message;
	Msg* next; 

};

//allocates and properly initializes a list structure and returns a pointer to it or null
Mailbox* CreateMailBox();

//destroys list, whose pointer is passed in as an argument.
RC DestroyMailBox (Mailbox* list);

//dequeues the msg at the head of list and returns a pointer to it, or else null
Msg* Dequeue (ThreadId receiverId, Mailbox* list);

//Enqueues the msg according to the reciever id
RC Enqueue(Msg* msg, Mailbox* mailbox);

//prints the elements of the list
void printmailbox(Mailbox* list);

#endif