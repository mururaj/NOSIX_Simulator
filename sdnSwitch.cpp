#include "sdnSwitch.h"
#include "flowGenerator.h"
#include "system.h"

#include <tr1/unordered_map>
#include <iostream>
using namespace std::tr1;

unordered_map <unsigned int,unsigned int> hashSet;

unsigned int firstSmallFlowNum = 0;
unsigned int lastSmallFlowNum = 0;

//#define DEBUG
#define STRAWMAN
//#define NOSIX

#ifdef STRAWMAN
int sdnSwitch::run3(unsigned int currentTime)
{
	bool processedFlows = false;
		
	numBytesForwarded = 0;
	numFlowsForwarded = 0;
	numBytesDropped = 0;
	numLargeFlowsForwarded = 0;
	numSmallFlowsForwarded = 0;
	numPktsForwarded = 0; 
	numFlowsDropped = 0;
	numSmallFlowsDropped = 0;
	numLargeFlowsDropped = 0;
	firstSmallFlowNum = 0;
	lastSmallFlowNum = 0;
	
	#ifdef DEBUG
	flowArrivalQueue->getQueueStatistics();
	#endif
	
	if (!receiveIncomingFlows())
	{
		printf("There are no flows to receive\n");
	} else
	{
		#ifdef DEBUG
			printf("Flows are received \n");
		#endif
	}

	if (!receiveControllerMsgs())
	{
		#ifdef DEBUG
			printf("No message to process from controller\n");
		#endif
		
	} else
	{
		processedFlows = true;
	}

	if (!forwardFlows())
	{
		#ifdef DEBUG
			printf("No message to forward\n");
		#endif
		
	} else
	{
		processedFlows = true;
	}

	if (!sendControllerMsgs())
	{
		#ifdef DEBUG
			printf("Unable to send message to controller\n");
		#endif
	}
	
	printf("Time %u: Processing a flow\n",currentTime);
	getDataPlaneStatistics();
	
	// Schedule next processing event
	Event *newflowProcessingEvent = new Event();
	newflowProcessingEvent->eventType = switchProcessingEvent;
	if (!processedFlows)
	{
		#ifdef DEBUG
			printf("Time %u: No flow processed\n",currentTime);
		#endif
		newflowProcessingEvent->eventTime = ++currentTime;

	} else
	{
		newflowProcessingEvent->eventTime = currentTime + SWITCHPROCESSTIME;
	}
	eventScheduler->append((void *) newflowProcessingEvent);

	return 1;
}

/*
Func sendControllerMsgs: Examine incoming flow & prepare msgs for controller
-> Check if the incoming flow is matching or not
-> If matching, move the flow to list of active flows
-> If not matching, check if flow table has empty entries
-> If flow table has space, then install the flow & send packet_in
-> If flow table has no space, then send process_packet message  
*/
unsigned int sdnSwitch::sendControllerMsgs()
{
	
	unsigned int  controlChanlRate = getCtrlChanlRateInBytes();

	Flow *waitingFlow;
	while ((waitingFlow = (Flow *) tmpIncomingFlowQueue->remove()))
	{
			/* Pre declaring now. Stub to check execution without controller
			if (!hashSet[waitingFlow->flowNumber])
			{
				hashSet[waitingFlow->flowNumber] = 1;
				myproperties.numFlowsInstalled++;

				#ifdef DEBUG
					printf("Number of installed flows %u\n",myproperties.numFlowsInstalled);
				#endif

			}*/		

			// Check if this flow matches flow table entry
			if (!hashSet[waitingFlow->flowNumber] || waitingFlow->flowTag == begin 
				|| waitingFlow->flowTag == fullflow)
			{  
				// handle flow table missing action
				
				#ifdef DEBUG
					printf("Flow %u is not matching flow table\n",waitingFlow->flowNumber);
				#endif

				// Check if flow table is full
				if (myproperties.numFlowsInstalled < myproperties.perTableNumFlowEntries)
				{
					
					// Send first packet of the flow to control channel
					// sending packet_in asynchronous message to controller
					if (waitingFlow->flowType == large)
					{
						if ((controlChanlRate/LARGE_FLOW_PACKET_SIZE) < 1)
						{
							#ifdef DEBUG
								printf("Dropping large flow %u\n",waitingFlow->flowNumber);
							#endif
							// code to drop the flow
							numBytesDropped += (waitingFlow->numPackets * 
												LARGE_FLOW_PACKET_SIZE);
							delete waitingFlow;
							waitingFlow = NULL;
							numFlowsDropped++;
							numLargeFlowsDropped++;
						} else
						{
							#ifdef DEBUG
								printf("FT is not full. Sending packetin for large flow %u\n",
												waitingFlow->flowNumber);
							#endif
							
							// Update control channel rate
							controlChanlRate -=LARGE_FLOW_PACKET_SIZE;
							
							// Update total number of installed flow entries
							myproperties.numFlowsInstalled++;
							myproperties.numFlowEntriesByLarge++;

							waitingFlow->flowAction = PACKET_IN;

							// Add this flow to control channel
							switchToControllerQueue->append((void *)waitingFlow);

							// Also add copy of this flow to waiting flows
							waitingFlowsQueue->append((void *)waitingFlow);
						}
					} else if (waitingFlow->flowType == small)
					{
						if ((controlChanlRate/SMALL_FLOW_PACKET_SIZE) < 1)
						{
							#ifdef DEBUG
								printf("Dropping small flow %u\n",waitingFlow->flowNumber);
							#endif
							// code to drop the flow
							numBytesDropped += (waitingFlow->numPackets * 
												SMALL_FLOW_PACKET_SIZE);
							delete waitingFlow;
							waitingFlow = NULL;
							numFlowsDropped++;
							numSmallFlowsDropped++;
						} else
						{							
							#ifdef DEBUG
								printf("FT is not full. Sending packetin for small flow %u\n",
											waitingFlow->flowNumber);
							#endif

							if (firstSmallFlowNum == 0)
							{
								firstSmallFlowNum = waitingFlow->flowNumber;
							} else if (waitingFlow->flowNumber < firstSmallFlowNum)
							{
								firstSmallFlowNum = waitingFlow->flowNumber;
							}

							if (lastSmallFlowNum == 0)
							{
								lastSmallFlowNum = waitingFlow->flowNumber;
							} else if (waitingFlow->flowNumber > lastSmallFlowNum)
							{
								lastSmallFlowNum = waitingFlow->flowNumber;
							}
							
							// Update control channel rate
							controlChanlRate -=SMALL_FLOW_PACKET_SIZE;

							// Update total number of installed flow entries
							myproperties.numFlowsInstalled++;
							myproperties.numFlowEntriesBySmall++;

							waitingFlow->flowAction = PACKET_IN;

							// Add this flow to control channel
							switchToControllerQueue->append((void *)waitingFlow);

							// Also add copy of this flow to waiting flows
							waitingFlowsQueue->append((void *)waitingFlow);
						}	
					}
				} else
				{
					// handle flow table full case
					
					// Send all the packets of this flow to control channel
					// sending process_packet asynchronous message to controller
					if (waitingFlow->flowType == large)
					{
						// code to evict a small flow						
					  
					  if ((controlChanlRate/
							(LARGE_FLOW_PACKET_SIZE * waitingFlow->numPackets)) < 1)
						{
							#ifdef DEBUG
								printf("Dropping large flow %u\n",waitingFlow->flowNumber);
							#endif
							// code to drop the flow
							numBytesDropped += (waitingFlow->numPackets * 
												LARGE_FLOW_PACKET_SIZE);
							delete waitingFlow;
							waitingFlow = NULL;
							numFlowsDropped++;
							numLargeFlowsDropped++;
						} else
						{
							#ifdef DEBUG
								printf("Flow table is full. Sending packetout\n");
							#endif
							
							// Update control channel rate
							controlChanlRate -= (LARGE_FLOW_PACKET_SIZE * waitingFlow->numPackets);

							waitingFlow->flowAction = PROCESS_PACKET;

							// Add this flow to control channel
							switchToControllerQueue->append((void *)waitingFlow);

							// Also add copy of this flow to waiting flows
							waitingFlowsQueue->append((void *)waitingFlow);
						}
					} else if (waitingFlow->flowType == small)
					{
						if ((controlChanlRate/
							(SMALL_FLOW_PACKET_SIZE * waitingFlow->numPackets)) < 1)
						{
							#ifdef DEBUG
								printf("Dropping small flow %u\n",waitingFlow->flowNumber);
							#endif
							// code to drop the flow
							numBytesDropped += (waitingFlow->numPackets * 
												SMALL_FLOW_PACKET_SIZE);
							delete waitingFlow;
							waitingFlow = NULL;
							numFlowsDropped++;
							numSmallFlowsDropped++;
						} else
						{
							#ifdef DEBUG
								printf("Flow table is full. Sending packetout\n");
							#endif
							// Update control channel rate
							controlChanlRate -= (SMALL_FLOW_PACKET_SIZE * waitingFlow->numPackets);

							waitingFlow->flowAction = PROCESS_PACKET;

							// Add this flow to control channel
							switchToControllerQueue->append((void *)waitingFlow);

							// Also add copy of this flow to waiting flows
							waitingFlowsQueue->append((void *)waitingFlow);
						}
					}
				}

			} else
			{
				// handle flow table matching action

				#ifdef DEBUG
					printf("Flow %u is matching flow table\n",waitingFlow->flowNumber);
				#endif

				waitingFlow->flowMatching = true;

				// Move the flow to active flows
				// Note no need to send matching flows to controller
				activeFlowsQueue->append((void *)waitingFlow);

				// update num bytes to be transferred

				if (waitingFlow->flowType == large)
				{
					hashSet[waitingFlow->flowNumber] = hashSet[waitingFlow->flowNumber] +
											( waitingFlow->numPackets * LARGE_FLOW_PACKET_SIZE);
				} else if (waitingFlow->flowType == small)
				{
					hashSet[waitingFlow->flowNumber] = hashSet[waitingFlow->flowNumber] +
											( waitingFlow->numPackets * SMALL_FLOW_PACKET_SIZE);
				}											
			}						
	}		

	return 1;
}

// Receive the messages from controller and place
// the flows in proper queue
unsigned int sdnSwitch::receiveControllerMsgs()
{
	Flow *msgFromController;
	bool processedFlows = false;

	// receive messages from controller and take appropriate action
	while ((msgFromController = (Flow *)controllerToSwitchQueue->remove()))
	{
		processedFlows = true;

		// Process the waiting flows & move them to active flows
		Flow *activeFlow = (Flow *)waitingFlowsQueue->remove();

		if (!activeFlow)
		{
			printf("Error no active flow to add to active flow queue\n");
			return 0;
		}
			#ifdef DEBUG
			if (msgFromController->flowNumber == activeFlow->flowNumber)
			{
				printf("Controller msg & waiting flow number matches for %u\n",
													activeFlow->flowNumber);
			} else
			{
				printf("Controller msg %u & waiting flow number %u are not matching\n",
										msgFromController->flowNumber,activeFlow->flowNumber);
			}
			#endif

		// Interpret the controller messages
		if (msgFromController->flowAction == FLOW_INSTALL)
			{
				if (hashSet[activeFlow->flowNumber])
				{ 
					printf("Error this flow %u is already installed in flow table\n",
						activeFlow->flowNumber);
					return 0;
				}
				
				// Install the flow 
				if (activeFlow->flowType == large)
				{
					hashSet[activeFlow->flowNumber] = activeFlow->numPackets * 
															LARGE_FLOW_PACKET_SIZE;
				} else if (activeFlow->flowType == small)
				{
					// Install flow table entry
					hashSet[activeFlow->flowNumber] = activeFlow->numPackets * 
															SMALL_FLOW_PACKET_SIZE;		
				}

					#ifdef DEBUG
						printf("Number of installed flows %u\n",myproperties.numFlowsInstalled);
					#endif

				// This flow is now a matching flow
				activeFlow->flowMatching = true;

				// Set the action 
				activeFlow->flowAction = FLOW_INSTALL;					
				
				// Add this flow to active flow queue
				activeFlowsQueue->append((void *)activeFlow);

			} else if (msgFromController->flowAction == PACKET_OUT)
			{
					//This flow is not a matching flow
					activeFlow->flowMatching = false;

					//Set the action
					activeFlow->flowAction = PACKET_OUT;

					// Add this flow to active flow queue
					activeFlowsQueue->append((void *)activeFlow);
			}					
		}
	
	if (!processedFlows)
	{
		printf("No controller messages\n");
	}

	return 1;
}

#endif // end of STRAWMAN

/*
Func fastPath: Represents flow entries are matching TCAM -
and hence will be forwarded to output interface. 
Args: 
"unsinged int flowSize" Size of Flow to be forwarded
*/
void sdnSwitch::fastPath(unsigned int flowSize)
{
		numBytesForwarded += flowSize;
		numFlowsForwarded++;	
}


void sdnSwitch::getDataPlaneStatistics()
{
	printf("Number of bytes forwarded %u\n",numBytesForwarded);
	printf("Number of bytes dropped %u\n",numBytesDropped);
	printf("Number of flows forwarded %u\n",numFlowsForwarded);
	printf("Number of large flows forwarded %u\n",numLargeFlowsForwarded);
	printf("Number of small flows forwarded %u\n",numSmallFlowsForwarded);
	printf("Number of packets forwarded %u\n",numPktsForwarded);
	printf("Number of flows dropped %u\n",numFlowsDropped);
	printf("Number of small flows dropped %u \n",numSmallFlowsDropped);
	printf("Number of large flows dropped %u \n",numLargeFlowsDropped);
	printf("Number of flows installed %u\n",myproperties.numFlowsInstalled);
	printf("Number of flow table entries filled by small flows %u\n",myproperties.numFlowEntriesBySmall);
	printf("Number of flow table entries filled by large flows %u\n",myproperties.numFlowEntriesByLarge);
}


unsigned int sdnSwitch::getfwdRateInBytes()
{
    unsigned int fwdRateInBytes = myproperties.forwardingRate * NUMBYTESPERGB;
    return fwdRateInBytes;
}


unsigned int sdnSwitch::getCtrlChanlRateInBytes()
{
	unsigned int controlChannelRateInBytes = myproperties.ctrlChannelRate * NUMBYTESPERMB;
	return controlChannelRateInBytes;
}

unsigned int sdnSwitch::findSmallFlowEntry()
{
	if (firstSmallFlowNum == 0 || lastSmallFlowNum == 0 
		|| firstSmallFlowNum > lastSmallFlowNum)
	{
		// error
		return 0;
	}
	for (unsigned int i=firstSmallFlowNum;i<=lastSmallFlowNum;i++)
	{
		if (hashSet[i])
		{
			return i;
		}
	}
	
	return 0;
}

// Following function receives the incoming flows from flow-
// arrivalQ and move it to tmp Q
unsigned int sdnSwitch::receiveIncomingFlows()
{
	Flow *newFlow = NULL;
	bool flowReceived = false;

	// receive all the incoming flows
	while ((newFlow = (Flow *)flowArrivalQueue->remove()))
	{
		// Assume all flows are non-matching
		if (!newFlow->flowMatching)
		{
			flowReceived = true;
			// Add all incoming flows to a temp queue. 
			// This Q is not required. To be fixed.
			tmpIncomingFlowQueue->append((void *)newFlow);			
		}
	}

	if (flowReceived)
	{
		return 1;
	} else
	{
		return 0;
	}
}

/*
Func forwardFlows: Performs switch forwarding operation
-> Forwards flow at switch forwarding rate
-> Update the relevant counters for statistics
-> If the flow is the end of this flow, then
   remove it from flow table
*/
unsigned int sdnSwitch::forwardFlows()
{
	Flow *activeFlow;
	unsigned int nextFlowPktSize = 0; // size of the next flow
	unsigned int  switchFwdRate = getfwdRateInBytes();
	bool processedFlows = false;
	
	// Get size of next flow packets in active flows queue
	if ((activeFlow = (Flow *)activeFlowsQueue->getData()))
	{
		if (activeFlow->flowType == large)
		{
			nextFlowPktSize = activeFlow->numPackets * LARGE_FLOW_PACKET_SIZE;
		} else if (activeFlow->flowType == small)
		{
			nextFlowPktSize = activeFlow->numPackets * SMALL_FLOW_PACKET_SIZE;
		}
	} else
	{
		printf("No flows in active flow Queue\n");
	}

	// forward the active flows
	// check if remaining size is large enough to transfer next flow packet
	while (	(switchFwdRate - numBytesForwarded) > (nextFlowPktSize) )
	{
		 // check if no more flow in active queue
		 if (!(activeFlow = (Flow *)activeFlowsQueue->remove()))
		 {
			 printf("No flows in active flow Queue to forward\n");
			 break;
		 }

		  processedFlows = true;

		// forward the flow and update the DP counters
		if (activeFlow->flowType == large)
		{
			numLargeFlowsForwarded++;
			numBytesForwarded += activeFlow->numPackets * LARGE_FLOW_PACKET_SIZE;
			numPktsForwarded += activeFlow->numPackets;
			numFlowsForwarded++;
		} else if (activeFlow->flowType == small)
		{
			numSmallFlowsForwarded++;
			numBytesForwarded += activeFlow->numPackets * SMALL_FLOW_PACKET_SIZE;
			numPktsForwarded += activeFlow->numPackets;
			numFlowsForwarded++;			
		}

		if (activeFlow->flowMatching)
		{ 
			// Uninstall this flow from flow table if this is last piece of the flow
			if (activeFlow->flowTag == fullflow || activeFlow->flowTag == end)
			{
				hashSet.erase(activeFlow->flowNumber);
				myproperties.numFlowsInstalled--;

				if (activeFlow->flowType == small && 
					myproperties.numFlowEntriesBySmall != 0)
				{
					myproperties.numFlowEntriesBySmall--;
				} else if (activeFlow->flowType == large &&
					myproperties.numFlowEntriesByLarge != 0)
				{
					myproperties.numFlowEntriesByLarge--;
				}

				#ifdef DEBUG
					printf("Uninstalled active flow %u\n",activeFlow->flowNumber);
					printf("Number of installed flows %u\n",myproperties.numFlowsInstalled);
				#endif
			}		
				
		} else
		{
			// Any additional processing if required		
		}

		// free up the flow 
		delete(activeFlow);
		activeFlow = NULL;

		// Get size of next flow packets in active flows queue
		if ((activeFlow = (Flow *)activeFlowsQueue->getData()))
		{
			if (activeFlow->flowType == large)
			{
				nextFlowPktSize = activeFlow->numPackets * LARGE_FLOW_PACKET_SIZE;
			} else if (activeFlow->flowType == small)
			{
				nextFlowPktSize = activeFlow->numPackets * SMALL_FLOW_PACKET_SIZE;
			}
		} else
		{		
			printf("No flows in active flow Queue\n");
			break; // break from while
		}
	}


	if (!processedFlows)
	{
		printf("No messages in active queue for forwarding\n");
	}

	return 1;
}

#ifdef NOSIX

int sdnSwitch::run3(unsigned int currentTime)
{
	bool processedFlows = false;
	
	
	numBytesForwarded = 0;
	numBytesDropped = 0;
	numFlowsForwarded = 0;
	numLargeFlowsForwarded = 0;
	numSmallFlowsForwarded = 0;
	numPktsForwarded = 0; 
	numFlowsDropped = 0;
	numSmallFlowsDropped = 0;
	numLargeFlowsDropped = 0;
	firstSmallFlowNum = 0;
	lastSmallFlowNum = 0;
	
	#ifdef DEBUG
	flowArrivalQueue->getQueueStatistics();
	#endif
	
	if (!receiveIncomingFlows())
	{
		printf("There are no flows to receive\n");
	} else
	{
		#ifdef DEBUG
			printf("Flows are received \n");
		#endif
	}

	if (!receiveControllerMsgs())
	{
		#ifdef DEBUG
			printf("No message to process from controller\n");
		#endif
		
	} else
	{
		processedFlows = true;
	}

	if (!forwardFlows())
	{
		#ifdef DEBUG
			printf("No message to forward\n");
		#endif
		
	} else
	{
		processedFlows = true;
	}
	
	if (!sendControllerMsgs())
	{
		#ifdef DEBUG
			printf("Unable to send messages to controller\n");
		#endif
	} 


	printf("Time %u: Processing a flow\n",currentTime);
	getDataPlaneStatistics();
	
	// Schedule next processing event
	Event *newflowProcessingEvent = new Event();
	newflowProcessingEvent->eventType = switchProcessingEvent;
	if (!processedFlows)
	{
		#ifdef DEBUG
			printf("Time %u: No flow processed\n",currentTime);
		#endif
		newflowProcessingEvent->eventTime = ++currentTime;

	} else
	{
		newflowProcessingEvent->eventTime = currentTime + SWITCHPROCESSTIME;
	}
	eventScheduler->append((void *) newflowProcessingEvent);

	return 1;
}

/*
Func sendControllerMsgs: Examine incoming flow & prepare msgs for controller
-> Check if the incoming flow is matching or not
-> If matching, move the flow to list of active flows
-> If not matching, check if flow table has empty entries
-> If flow table has space, then install the flow & send packet_in
-> If flow table has no space, then send process_packet message 
   -> Note that NOSIX performs eviction of small flows from flow table 
*/
unsigned int sdnSwitch::sendControllerMsgs()
{

	unsigned int  controlChanlRate = getCtrlChanlRateInBytes();

	Flow *waitingFlow;
	while ((waitingFlow = (Flow *) tmpIncomingFlowQueue->remove()))
	{
			/* Pre declaring now. Stub to check execution without controller
			if (!hashSet[waitingFlow->flowNumber])
			{
				hashSet[waitingFlow->flowNumber] = 1;
				myproperties.numFlowsInstalled++;

				#ifdef DEBUG
					printf("Number of installed flows %u\n",myproperties.numFlowsInstalled);
				#endif

			}*/		

			// Check if this flow matches flow table entry
			if (!hashSet[waitingFlow->flowNumber] || waitingFlow->flowTag == begin 
				|| waitingFlow->flowTag == fullflow)
			{  
				// handle flow table missing action
				
				if (hashSet[waitingFlow->flowNumber])
				{
					printf("Error -> begin/full flow already in table \n");
				}

				#ifdef DEBUG
					printf("Flow %u is not matching flow table\n",waitingFlow->flowNumber);
				#endif

				// Check if flow table is full
				if (myproperties.numFlowsInstalled < myproperties.perTableNumFlowEntries)
				{
					
					// Send first packet of the flow to control channel
					// sending packet_in asynchronous message to controller
					if (waitingFlow->flowType == large)
					{
						if ((controlChanlRate/LARGE_FLOW_PACKET_SIZE) < 1)
						{
							#ifdef DEBUG
								printf("Dropping large flow %u\n",waitingFlow->flowNumber);
							#endif

							// code to drop the flow
							numBytesDropped += (waitingFlow->numPackets * LARGE_FLOW_PACKET_SIZE);
							delete waitingFlow;
							waitingFlow = NULL;
							numFlowsDropped++;
							numLargeFlowsDropped++;
						} else
						{
							#ifdef DEBUG
								printf("FT is not full. Sending packetin for large flow %u\n",
												waitingFlow->flowNumber);
							#endif
							
							// Update control channel rate
							controlChanlRate -=LARGE_FLOW_PACKET_SIZE;
							
							// Update total number of installed flow entries
							myproperties.numFlowsInstalled++;
							myproperties.numFlowEntriesByLarge++;

							hashSet[waitingFlow->flowNumber] = waitingFlow->numPackets * 
															LARGE_FLOW_PACKET_SIZE;

							waitingFlow->flowAction = PACKET_IN;

							// Send packet_in to controller
							switchToControllerQueue->append((void *)waitingFlow);

							// also add copy of this flow to waiting flows
							waitingFlowsQueue->append((void *)waitingFlow);
						}
					} else if (waitingFlow->flowType == small)
					{
						if ((controlChanlRate/SMALL_FLOW_PACKET_SIZE) < 1)
						{
							#ifdef DEBUG
								printf("Dropping small flow %u\n",waitingFlow->flowNumber);
							#endif
							// code to drop the flow
							numBytesDropped += (waitingFlow->numPackets * SMALL_FLOW_PACKET_SIZE);
							delete waitingFlow;
							waitingFlow = NULL;
							numFlowsDropped++;
							numSmallFlowsDropped++;
						} else
						{							
							#ifdef DEBUG
								printf("FT is not full. Sending packetin for small flow %u\n",
											waitingFlow->flowNumber);
							#endif
							
							// To find the flow number of first small flow &
							// most recent small flow
							if (firstSmallFlowNum == 0)
							{
								firstSmallFlowNum = waitingFlow->flowNumber;
							} else if (waitingFlow->flowNumber < firstSmallFlowNum)
							{
								firstSmallFlowNum = waitingFlow->flowNumber;
							}

							if (lastSmallFlowNum == 0)
							{
								lastSmallFlowNum = waitingFlow->flowNumber;
							} else if (waitingFlow->flowNumber > lastSmallFlowNum)
							{
								lastSmallFlowNum = waitingFlow->flowNumber;
							}
							
							// Update control channel rate
							controlChanlRate -=SMALL_FLOW_PACKET_SIZE;

							// Update total number of installed flow entries
							myproperties.numFlowsInstalled++;
							myproperties.numFlowEntriesBySmall++;

							hashSet[waitingFlow->flowNumber] = waitingFlow->numPackets * 
															SMALL_FLOW_PACKET_SIZE;

							waitingFlow->flowAction = PACKET_IN;

							// Send packet_in messgae to controller
							switchToControllerQueue->append((void *)waitingFlow);

							// Also add copy of this flow to waiting flows
							waitingFlowsQueue->append((void *)waitingFlow);
						}	
					}
				} else
				{
					// handle flow table full case
					
					// Send all the packets of this flow to control channel
					// sending process_packet asynchronous message to controller
					if (waitingFlow->flowType == large)
					{
						// code to evict a small flow
						unsigned int smallFlowToEvict = findSmallFlowEntry();

						bool evicted = false;
						
						// Found a small flow to evict & Check if control channel has spare BW
						if (smallFlowToEvict && 
							((controlChanlRate/ 
								(LARGE_FLOW_PACKET_SIZE +
								((NUM_PACKETS_SMALL_FLOW - 1) * SMALL_FLOW_PACKET_SIZE))) > 1))
						{
											
								// Evict the small flow
								hashSet.erase(smallFlowToEvict);

								myproperties.numFlowEntriesBySmall--;

								evicted = true;

								// Install the large flow
								hashSet[waitingFlow->flowNumber] = waitingFlow->numPackets * 
															LARGE_FLOW_PACKET_SIZE;
								
								myproperties.numFlowEntriesByLarge++;

								// Update control channel rate
								controlChanlRate -= ( LARGE_FLOW_PACKET_SIZE +
											((NUM_PACKETS_SMALL_FLOW - 1) * SMALL_FLOW_PACKET_SIZE));

								
								// Flow action is set for large flow
								waitingFlow->flowAction = PACKET_IN;

								// Sent packet_in message to controller
								switchToControllerQueue->append((void *)waitingFlow);

								// Also send copy of  this flow to waiting flows
								waitingFlowsQueue->append((void *)waitingFlow);

								#ifdef DEBUG
									printf("Evicted small flow %u & Sending packetin for large flow %u\n",
												smallFlowToEvict,waitingFlow->flowNumber);
								#endif
					  } 
					  
					  if (!evicted)
					  {
						  if ((controlChanlRate/
							(LARGE_FLOW_PACKET_SIZE * waitingFlow->numPackets)) < 1)
						{
							#ifdef DEBUG
								printf("Dropping large flow %u\n",waitingFlow->flowNumber);
							#endif
							// code to drop the flow
							numBytesDropped += ( waitingFlow->numPackets * LARGE_FLOW_PACKET_SIZE );
							delete waitingFlow;
							waitingFlow = NULL;
							numFlowsDropped++;
							numLargeFlowsDropped++;
						} else
						{
							#ifdef DEBUG
								printf("Flow table is full. Sending packetout\n");
							#endif
							
							// Update control channel rate
							controlChanlRate -= (LARGE_FLOW_PACKET_SIZE * waitingFlow->numPackets);
							
							// Send process packet message to controller
							waitingFlow->flowAction = PROCESS_PACKET;
							switchToControllerQueue->append((void *)waitingFlow);

							// Add this flow to waiting flows
							waitingFlowsQueue->append((void *)waitingFlow);
						}
					  }

					} else if (waitingFlow->flowType == small)
					{
						if ((controlChanlRate/
							(SMALL_FLOW_PACKET_SIZE * waitingFlow->numPackets)) < 1)
						{
							#ifdef DEBUG
								printf("Dropping small flow %u\n",waitingFlow->flowNumber);
							#endif
							// code to drop the flow
							numBytesDropped += ( waitingFlow->numPackets * SMALL_FLOW_PACKET_SIZE );
							delete waitingFlow;
							waitingFlow = NULL;
							numFlowsDropped++;
							numSmallFlowsDropped++;
						} else
						{
							#ifdef DEBUG
								printf("Flow table is full. Sending packetout\n");
							#endif
							// Update control channel rate
							controlChanlRate -= (SMALL_FLOW_PACKET_SIZE * waitingFlow->numPackets);

							waitingFlow->flowAction = PROCESS_PACKET;

							// Send process_packet message to controller
							switchToControllerQueue->append((void *)waitingFlow);

							// Add this flow to waiting flows
							waitingFlowsQueue->append((void *)waitingFlow);
						}
					}
				}

			} else
			{
				// handle flow table matching action

				#ifdef DEBUG
					printf("Flow %u is matching flow table\n",waitingFlow->flowNumber);
				#endif

				waitingFlow->flowMatching = true;

				// Move the flow to active flows
				activeFlowsQueue->append((void *)waitingFlow);

				// update num bytes to be transferred

				if (waitingFlow->flowType == large)
				{
					hashSet[waitingFlow->flowNumber] = hashSet[waitingFlow->flowNumber] +
											( waitingFlow->numPackets * LARGE_FLOW_PACKET_SIZE);
				} else if (waitingFlow->flowType == small)
				{
					hashSet[waitingFlow->flowNumber] = hashSet[waitingFlow->flowNumber] +
											( waitingFlow->numPackets * SMALL_FLOW_PACKET_SIZE);
				}											
			}							
	}		

	return 1;
}

// Receive the messages from controller and place
// the flows in proper queue
unsigned int sdnSwitch::receiveControllerMsgs()
{
	Flow *msgFromController;
	bool processedFlows = false;

	// receive messages from controller and take appropriate action
	while ((msgFromController = (Flow *)controllerToSwitchQueue->remove()))
	{
		processedFlows = true;

		// Process the waiting flows & move them to active flows
		Flow *activeFlow = (Flow *)waitingFlowsQueue->remove();

		if (!activeFlow)
		{
			printf("Error no active flow to add to active flow queue\n");
			return 0;
		}
			#ifdef DEBUG
			if (msgFromController->flowNumber == activeFlow->flowNumber)
			{
				printf("Controller msg & waiting flow number matches for %u\n",
													activeFlow->flowNumber);
			} else
			{
				printf("Controller msg %u & waiting flow number %u are not matching\n",
										msgFromController->flowNumber,activeFlow->flowNumber);
			}
			#endif

		// Interpret the controller messages
		if (msgFromController->flowAction == FLOW_INSTALL)
			{
				bool evictedCheck = false;

				// Check if the flow was evicted & to be treated as packet_out
					if (!hashSet[activeFlow->flowNumber] &&
						activeFlow->flowType == small)
					{
						#ifdef DEBUG
							printf("An evicted flow %u treat as packet_out\n",
										activeFlow->flowNumber);
						#endif

						// Set this flow as non-matching flow
						activeFlow->flowMatching = false;

						// Set the action as packetout 
						activeFlow->flowAction = PACKET_OUT;

						evictedCheck = true;
						
						// Add this flow to active flow queue
						activeFlowsQueue->append((void *)activeFlow);

					}

				if (!evictedCheck)
				{
					// If flow not already installed, then error
					if (!hashSet[activeFlow->flowNumber])
					{ 
						printf("Error this flow %u is not yet installed in flow table\n",
						activeFlow->flowNumber);					
					}
				
					#ifdef DEBUG
						printf("Number of installed flows %u\n",myproperties.numFlowsInstalled);
					#endif

				// This flow is now a matching flow
				activeFlow->flowMatching = true;

				// Set the action 
				activeFlow->flowAction = FLOW_INSTALL;					
				
				// Add this flow to active flow queue
				activeFlowsQueue->append((void *)activeFlow);
				}


			} else if (msgFromController->flowAction == PACKET_OUT)
			{
					//This flow is not a matching flow
					activeFlow->flowMatching = false;

					//Set the action
					activeFlow->flowAction = PACKET_OUT;

					// Add this flow to active flow queue
					activeFlowsQueue->append((void *)activeFlow);
			}					
		}
	
	if (!processedFlows)
	{
		printf("No controller messages\n");
	}

	return 1;
}

#endif // end of NOSIX
