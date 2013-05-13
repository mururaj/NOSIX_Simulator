#include "sdnController.h"
#include "system.h"

int sdnController::run(unsigned int currentTime)
{
	Flow *switchToContrlMsg;
	bool processedMsgs = false;
	numLargeFlowsExamined = 0;
	numSmallFlowsExamined = 0;
	totalNumFlowsExamined = 0;
	totalNumPktsExamined = 0;
	totalNumFlowsProcessed = 0;
	numBytesExamined = 0;
	numBytesProcessed = 0;

	while ((switchToContrlMsg=(Flow *)switchToControllerQueue->remove()))
	{
		processedMsgs = true;

		// As of now controller push the message back to Ctrl<->Switch queue
		// We need to come up with specific msg pkt implementation to handle
		// packet_out msgs etc

		// Update controller counters
		if (switchToContrlMsg->flowType == large)
		{
			numLargeFlowsExamined++;			
		} else if (switchToContrlMsg->flowType == small)
		{
			numSmallFlowsExamined++;
		}
		
		totalNumFlowsExamined++;

		// If new packet_in, then install the flow
		if (switchToContrlMsg->flowAction == PACKET_IN)
		{
			switchToContrlMsg->flowAction = FLOW_INSTALL;
			++totalNumPktsExamined;

			// Check whether packet_in is for large/small flow
			if (switchToContrlMsg->flowType == large)
			{
				numBytesExamined += (LARGE_FLOW_PACKET_SIZE);
			} else if (switchToContrlMsg->flowType == small)
			{
				numBytesExamined += (SMALL_FLOW_PACKET_SIZE);
			}

		} else if (switchToContrlMsg->flowAction == PROCESS_PACKET)
		{
			switchToContrlMsg->flowAction = PACKET_OUT;
			totalNumPktsExamined += switchToContrlMsg->numPackets;
			totalNumFlowsProcessed++;

			// Check whether process_packet is for large/small flow
			if (switchToContrlMsg->flowType == large)
			{
				numBytesExamined += (switchToContrlMsg->numPackets * 
					                 LARGE_FLOW_PACKET_SIZE);
				numBytesProcessed += (switchToContrlMsg->numPackets * 
					                 LARGE_FLOW_PACKET_SIZE);
			} else if (switchToContrlMsg->flowType == small)
			{
				numBytesExamined += (switchToContrlMsg->numPackets *
					                 SMALL_FLOW_PACKET_SIZE);
				numBytesProcessed += (switchToContrlMsg->numPackets *
					                 SMALL_FLOW_PACKET_SIZE);
			}
		}

		controllerToSwitchQueue->append((void*)switchToContrlMsg);
	}

	printf("Time %u: Processing by controller\n",currentTime);
	getControllerStatistics();

	// Schedule next processing event
	Event *contrlProcessingEvent = new Event();
	contrlProcessingEvent->eventType = controllerProcessingEvent;
	if (!processedMsgs)
	{
		#ifdef DEBUG
			printf("Time %u: No new messges to process\n",currentTime);
		#endif
		contrlProcessingEvent->eventTime = ++currentTime;

	} else
	{
		contrlProcessingEvent->eventTime = currentTime + CONTRLPROCESSINGTIME;
	}
	eventScheduler->append((void *) contrlProcessingEvent);

	return 1;
}

/*
Func getControllerStatitics: print the current statistics of controller
*/
void sdnController::getControllerStatistics()
{
	printf("Number of large flows examined %u \n",numLargeFlowsExamined);
	printf("Number of small flows examined %u \n",numSmallFlowsExamined);
	printf("Total number of flows examined %u \n",totalNumFlowsExamined);
	printf("Total number of packets examined %u \n",totalNumPktsExamined);
	printf("Total number of flows processed %u \n",totalNumFlowsProcessed);
	printf("Amount of bytes examined %u \n",numBytesExamined);
	printf("Amount of bytes processed %u \n",numBytesProcessed);
}
