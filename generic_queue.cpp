#include "generic_queue.h"
//#define DEBUG

/*
Func: append inserts item into queue
Args: void *data 
Return 1 on success or 0 on failure
*/
int Queue::append(void *data)
{
	/* We don't put a bound on queue size.
	   This may be set based on resource constraint.
	if (currentLength >= maxLength)
	{
		return 0;
	} */

	Node *newNode = new Node();
	if (!newNode)
	{
		printf("Error unable to allocate a new node append\n");
		return 0;
	}
	newNode->data = data;
	newNode->next = NULL;

	// If queue is empty, set head & tail
	if (!head && !tail)
	{
		head = newNode;
		tail = newNode;
		currentLength++;
		#ifdef DEBUG
		printf("A new node added to the queue\n");
		getQueueStatistics();
		#endif
		return 1;
	}

	// Append to end of queue & update tail
	tail->next = newNode;
	tail = newNode;
	currentLength++;

	#ifdef DEBUG
		printf("A new node added to the queue\n");
		getQueueStatistics();
	#endif

	return 1;
}

/*
Func: remove an item from head of queue
Return the item on success or NULL on failure
*/
void* Queue::remove()
{
	Node *tmpNode = NULL;
	void *data;

	// If queue is empty,return NULL
	if (!head && !tail)
	{
		printf("Queue is empty\n");
		return NULL;
	}

	// If only one node in queue, empty the queue
	if (head == tail)
	{
		tmpNode = head;
		head = NULL;
		tail = NULL;
		data = tmpNode->data;
		delete(tmpNode);
		tmpNode = NULL;
		currentLength--;
		#ifdef DEBUG
		printf("A node removed from the queue\n");
		getQueueStatistics();
		#endif
		return data;
	}

	if (!head || !tail)
	{
		return NULL;
	}

	// Update queue head
	tmpNode = head;
	head = head->next;
	data = tmpNode->data;
	delete(tmpNode);
	tmpNode = NULL;
	currentLength--;

	#ifdef DEBUG
		printf("A node removed from the queue\n");
		getQueueStatistics();
	#endif
	return data;
}

/*
Func: getId returns the ID number associated with queue
*/
unsigned int Queue::getId()
{
	return queueId;
}

/*
Func: getQueueStatistics prints information about the queue
Prints queue Id, current length and address of head and tail pointers
*/
void Queue::getQueueStatistics()
{
	printf("Queue Name %u\n",queueId);
	printf("Queue current length %u\n",currentLength);
	if (head != NULL && tail !=NULL )
	{
		printf("Queue head pointer %p\n",head);
		printf("Queue tail pointer %p\n",tail);
	}

}

/*
Func: getData return the data of the head node
Returns the data if head found or NULL if queue empty
*/
void* Queue::getData()
{
	if (head)
	{
		return head->data;
	} else
	{
		return NULL;
	}
}
