#include "defines.h"
#include "list.h"
#include "main.h"
#include "message.h"

#include <stdlib.h>

Mailbox* CreateMailBox(){
	Mailbox* list = (Mailbox*)malloc(sizeof(Mailbox));
	list->head = NULL;
	return list;
}

//destroys list, whose pointer is passed in as an argument.
RC DestroyMailBox (Mailbox* list){
	if (list == NULL) return -1;

	while (list->head != NULL) {
		Msg* temp = list->head->next;
		free(list->head);
		list->head = temp;
	}
	free(list);
	return RC_SUCCESS;
}

//dequeues the msg at the head of list and returns a pointer to it, or else null
Msg* Dequeue (ThreadId receiverId, Mailbox* list) {
	if (list->head == NULL) {
		return NULL;
	} else {
		Msg* t = list->head;
		if (t->receiverId == receiverId) {
			list->head = t->next;
			return t;
		} else {
			t = t->next;
			while (t != NULL && t->receiverId != receiverId) {
				t = t->next;
			}
			Msg* temp = t;
			t = t->next;
			return temp;
		}
	}
}

//Enqueues the msg according to the reciever id
RC Enqueue(Msg* msg, Mailbox* list) {
	if (list->head == NULL) {
		list->head =msg;
	} else {
		Msg* t = list->head;
		if (t->receiverId >msg->receiverId) {
		msg->next = t;
			list->head =msg;
		} else {
			while (t->next != NULL && t->next->receiverId <=msg->receiverId) {
				t = t->next;
			}
		msg->next = t->next;
			t->next =msg;
		}
	}
	return RC_SUCCESS;
}

//prints the elements of the list
void printmailbox(Mailbox* list) {
	#ifdef DEBUG
		Msg* currentMsg = list->head;
		printf("printing mailbox\n");
		while (currentMsg != NULL) {
			printf("receiverId: %d\t", currentMsg->receiverId);
			printf("senderId: %d\t", currentMsg->senderId);
			printf("msg: %d\t", currentMsg->msg);			
			printf("\n");
			currentMsg = currentMsg->next;
		}
	#endif
}