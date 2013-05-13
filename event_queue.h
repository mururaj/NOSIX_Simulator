#ifndef EVENTQUEUE_H
#define EVENTQUEUE_H

#include "generic_queue.h"

// As of now there are 3 event types
enum eventNames { flowArrivalEvent, switchProcessingEvent, controllerProcessingEvent };

// Node structure the event
typedef struct event
{
	unsigned int eventTime;
	eventNames eventType;	
}Event;

class EventQueue : public Queue
{
	public:
		EventQueue() : Queue()
		{
		}

		EventQueue(unsigned int qId, unsigned int length =0)
		{
			queueId = qId;
			maxLength = length;
			currentLength = 0; // Q is empty initially
			head = NULL;
			tail = NULL;
		}

		int append(void *);

		void* remove();

		int printScheduledEvents();

};
#endif //EVENTQUEUE_H
