#include "event_queue.h"
//#define DEBUG

/*
Func: append inserts event in sorted time order
Args: void *data 
Return 1 on success or 0 on failure
*/
int EventQueue::append(void *data)
{
	Node *newNode = new Node();
	if (!newNode)
	{
		printf("Error EventQueue Unable to allocate a new node\n");
		return 0;
	}
	newNode->data = data;
	newNode->next = NULL;
	Event *newEvent = (Event *) data;

	// If this is the first node, head & tail points this node
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

	
	if (!head || !tail)
	{
		printf("Error either head or tail empty\n");
		return 0;
	}

	// Insert at tail if this new event time > tail event time
	Event *currentEvent = (Event *) tail->data;
	if (currentEvent->eventTime < newEvent->eventTime)
	{
		tail->next = newNode;
		tail = newNode;
		currentLength++;
		#ifdef DEBUG
		printf("A new node added to the queue\n");
		getQueueStatistics();
		#endif
		return 1;
	}

	Node *currentNode = head;
	Node *prevNode = head;
	
	
	while (currentNode)
	{
		currentEvent = (Event *) currentNode->data;

		// Find the insertion point of new event
		if (currentEvent->eventTime < newEvent->eventTime)
		{
			prevNode = currentNode;
			if (!currentNode->next)
			{
				// Insert at tail
				currentNode->next = newNode;
				tail = newNode;
				currentLength++;
				#ifdef DEBUG
				printf("A new node added to the queue\n");
				getQueueStatistics();
				#endif
				return 1;
			}
			// Move to next node
			currentNode = currentNode->next;
		} else if (currentEvent->eventTime == newEvent->eventTime)
		{
			if (!currentNode->next)
			{
				// Insert at tail
				currentNode->next = newNode;
				tail = newNode;
				currentLength++;
				#ifdef DEBUG
				printf("A new node added to the queue\n");
				getQueueStatistics();
				#endif
				return 1;
			} else
			{	
				 Event *nextEvent = (Event *) (currentNode->next)->data;
				
				 // Find the last equal event if there are consecutive 
				 // events of same time stamps -> E1(t1)->E2(t1)->E3(t1)
				 // newEvent E4(t1) to be inserted after E3(t1)
				 if (nextEvent->eventTime == newEvent->eventTime)
				 {
					 currentNode = currentNode->next;
				 } else
				 {
					// Insert in between the nodes
					newNode->next = currentNode->next;
					currentNode->next = newNode;
					currentLength++;
					#ifdef DEBUG
					printf("A new node added to the queue\n");
					getQueueStatistics();
					#endif
					return 1;
				 }
			}
			
		} else
		{
			if (prevNode == head)
			{
				// Insert before the head
				newNode->next = currentNode;
				head = newNode;
			} else
			{
				// Insert before a node
				prevNode->next = newNode;
				newNode->next = currentNode;
			}

			currentLength++;
			#ifdef DEBUG
			printf("A new node added to the queue\n");
			getQueueStatistics();
			#endif
			return 1;
		}
		
	}
	return 0;
}

/*
Func: print the current events in event queue
Return 1 on success or 0 on failure
*/
int EventQueue::printScheduledEvents()
{
	Node *currentNode = head;
	Event *currentEvent;

	// Traverse the entire queue
	while (currentNode)
	{
		currentEvent = (Event *) currentNode->data;
		
		if (currentEvent)
		{
			switch (currentEvent->eventType)
			{
			case flowArrivalEvent:
				printf("Event: Arrivalevent Time:%u\n",currentEvent->eventTime);
				break;
			case switchProcessingEvent:
				printf("Event: switchEvent Time:%u\n",currentEvent->eventTime);
				break;
			case controllerProcessingEvent:
				printf("Event: Controllerevent Time:%u\n",currentEvent->eventTime);
				break;
			}
		} else
		{
			printf("Error - Null event added in event queue\n");
			return 0;
		}
		
		// Move to next event
		currentNode = currentNode->next;
	}

	return 1;
}

/*
Func: remove an event from head of queue
Return the event on success or NULL on failure
*/
void* EventQueue::remove()
{
	Node *tmpNode = NULL;
	void *data;

	// If no head & tail, error
	if (!head && !tail)
	{
		printf("Queue is empty\n");
		return NULL;
	}

	// If only one element, empty head & tail
	if (head == tail)
	{
		tmpNode = head;
		data = tmpNode->data;
		delete tmpNode;
		tmpNode = NULL;
		head = NULL;
		tail = NULL;
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
