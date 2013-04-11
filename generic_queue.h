/*
Author: Muruganantham Raju
Copyright: This code is part of on going research work at USC. 
           Please consult mraju@usc.edu, if you want to use
		   full/portion of this implementation work.

Declaration of communication FIFO Queues 
*/


#ifndef QUEUE_H
#define QUEUE_H
#include <stdio.h>
#include <string>
#define QMAXSIZE 500

// Node structure of the queue element
typedef struct node
{
	void *data;
	struct node *next;
}Node;

class Queue
{
	public:
	int append(void *);
	void* remove();
	unsigned int getId();
	void* getData();
	
	Queue() 
	{		
	}

	/* constructor initializes queue name and max length */
	Queue(unsigned int qId, unsigned int length = QMAXSIZE)
	{
		queueId = qId;
		maxLength = length;
		currentLength = 0; // Q is empty initially
		head = NULL;
		tail = NULL;
	}
	
	void getQueueStatistics();

	protected:
		unsigned int queueId; //User defined name for the queue
		unsigned int maxLength;	 //Max number of elements in the queue
		unsigned int currentLength; //Indicates the current num of elements in queue
		Node *head; //Points to head of the queue
		Node *tail; //Points to tail of the queue
};

#endif // QUEUE_H
