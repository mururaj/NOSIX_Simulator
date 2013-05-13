#include "flowGenerator.h"
#include "stdlib.h"
#include "system.h"

//#undef DEBUG

/*
Func run: Performs flow generator operations
Note: As of now flow generator creates only largeflows
Args:
"unsigned int currentTime" - current simulation time
Returns 1 on success, 0 otherwise
*/
int flowGenerator::run(unsigned int currentTime)
{
	
	// generateFlowsByByteSplit(currentTime);
	// generateFlowsByNumFlowSplit(currentTime);

	if (!generateFlowsByTotalNumFlowSplit(currentTime))
	{
		printf("Error in flow generation by total num flow split\n");
		return 0;
	}

	printf("Time %u: Generated the following flows\n",currentTime);
	getFlowGeneratorStatistics();
	
	// Schedule next flow arrival event
	Event *newflowArrivalEvent = new Event();
	newflowArrivalEvent->eventTime = currentTime + myproperties.flowArrivalInterval;
	newflowArrivalEvent->eventType = flowArrivalEvent;
	eventScheduler->append((void *) newflowArrivalEvent);
	
	// Flow generated succesfully
	return 1;
}

/*
Func generateSmallFlow: generate small flows 
Args:
"unsigned int currentTime" - current simulation time
whichMethod - 1 for generateFlowsByTotalNumFlowSplit
              0 otherwise
Returns 1 on success or 0 on failure
*/
int flowGenerator::generateSmallFlow(unsigned int currentTime,
										char whichMethod = '0')
{	
	// Create a new flow
	Flow *newFlow = new Flow();

	if (!newFlow)
	{
		// error unable to generate flow
		printf("Error: Unable to generate small flow\n");
		
		return 0;
	}

	newFlow->flowType = small;
	newFlow->flowMatching = false;
	newFlow->flowAction = NIL;

	unsigned int myflowTrackIndex = numLargeFlows + myproperties.numSmallFlowsGenerated;

	if (whichMethod == '1')
	{
		// Calculate num packets per flow
		unsigned int numPktsPerFlow = perSmallFlowShare / SMALL_FLOW_PACKET_SIZE;
		
		
		// Check if this is begining of new flow
		if (flowTracker[myflowTrackIndex] == 0)
		{
			if (numPktsPerFlow >= NUM_PACKETS_SMALL_FLOW)
			{
				newFlow->flowTag = fullflow;
				newFlow->numPackets = NUM_PACKETS_SMALL_FLOW;
				// Reset the flow
				flowTracker[myflowTrackIndex] = 0;
			} else
			{
				newFlow->flowTag = begin;
				newFlow->numPackets = numPktsPerFlow;
				flowTracker[myflowTrackIndex] +=numPktsPerFlow;
			}

		} else if (NUM_PACKETS_SMALL_FLOW - flowTracker[myflowTrackIndex] <= numPktsPerFlow)
		{
			newFlow->numPackets = NUM_PACKETS_SMALL_FLOW - flowTracker[myflowTrackIndex];
			newFlow->flowTag = end;
			// Reset the flow
			flowTracker[myflowTrackIndex] = 0;
		} else
		{
			newFlow->flowTag = continuing;
			newFlow->numPackets = numPktsPerFlow;
			flowTracker[myflowTrackIndex] +=numPktsPerFlow;
		}

	} else
	{
		newFlow->flowTag = fullflow;
		newFlow->numPackets = NUM_PACKETS_SMALL_FLOW;
	}

	// Note flownumber of small flow starts from numLargeFlows
	newFlow->flowNumber = myflowTrackIndex;

	//Record arrival time of this flow
	newFlow->flowArrivalTime = currentTime;

	//Place the flow in flow arrival queue
	if (!flowArrivalQueue->append((void *) newFlow))
	{
		printf("Error unable to append a new flow in arrival Queue \n");
		return 0;
	}

	#ifdef DEBUG
	   getFlowInfo(newFlow);
	#endif

	return 1;
		
}

/*
Func generateMediumFlow: generate medium flows 
Args:
"unsigned int currentTime" - current simulation time
Returns 1 on success or 0 on failure
Note:generateMediumFlow to be updated with whichMethod argument
*/
int flowGenerator::generateMediumFlow(unsigned int currentTime)
{	
	// Create a new flow
	Flow *newFlow = new Flow();

	if (!newFlow)
	{
		// error unable to generate flow
		printf("Error: Unable to generate medium flow\n");
		
		return 0;
	}

	newFlow->flowType = medium;
	newFlow->flowMatching = false;

	//Record arrival time of this flow
	newFlow->flowArrivalTime = currentTime;

	//Place the flow in flow arrival queue
	flowArrivalQueue->append((void *) newFlow);

	return 1;
		
}

/*
Func generateLargeFlow: generate large flows 
Args:
"unsigned int currentTime" - current simulation time
whichMethod - 1 for generateFlowsByTotalNumFlowSplit
              0 otherwise
Returns 1 on success or 0 on failure
*/
int flowGenerator::generateLargeFlow(unsigned int currentTime,
                                   char whichMethod = '0')
{	
	// Create a new flow
	Flow *newFlow = new Flow();

	if (!newFlow)
	{
		// error unable to generate flow
		printf("Error: Unable to generate large flow\n");
		return 0;
	}

	newFlow->flowType = large;
	newFlow->flowMatching = false;
	newFlow->flowAction = NIL;

	if (whichMethod == '1')
	{
		// Calculate num packets per flow
		unsigned int numPktsPerFlow = perLargeFlowShare / LARGE_FLOW_PACKET_SIZE;
		
		// Check if this is begining of new flow
		if (flowTracker[myproperties.numLargeFlowsGenerated] == 0)
		{
			if (numPktsPerFlow >= NUM_PACKETS_LARGE_FLOW)
			{
				newFlow->flowTag = fullflow;
				newFlow->numPackets = NUM_PACKETS_LARGE_FLOW;
				// Reset the flow
				flowTracker[myproperties.numLargeFlowsGenerated] = 0;
			} else
			{
				newFlow->flowTag = begin;
				newFlow->numPackets = numPktsPerFlow;
				flowTracker[myproperties.numLargeFlowsGenerated] +=numPktsPerFlow;
			}

		} else if (NUM_PACKETS_LARGE_FLOW - flowTracker[myproperties.numLargeFlowsGenerated] <= numPktsPerFlow)
		{
			newFlow->numPackets = NUM_PACKETS_LARGE_FLOW - flowTracker[myproperties.numLargeFlowsGenerated];
			newFlow->flowTag = end;
			// Reset the flow
			flowTracker[myproperties.numLargeFlowsGenerated] = 0;
		} else
		{
			newFlow->flowTag = continuing;
			newFlow->numPackets = numPktsPerFlow;
			flowTracker[myproperties.numLargeFlowsGenerated] +=numPktsPerFlow;
		}

	} else
	{
		newFlow->flowTag = fullflow;
		newFlow->numPackets = NUM_PACKETS_LARGE_FLOW;
	}

	newFlow->flowNumber = myproperties.numLargeFlowsGenerated;

	//Record arrival time of this flow
	newFlow->flowArrivalTime = currentTime;

	//Place the flow in flow arrival queue
	if (!flowArrivalQueue->append((void *) newFlow))
	{
		printf("Error unable to append a new flow in arrival Queue \n");
		return 0;
	}

	#ifdef DEBUG
	   getFlowInfo(newFlow);
	#endif
	
	return 1;
}


/*
Func generateFlowsByByteSplit: generate flows based on % of forward channel BW
Args:
"unsigned int currentTime" - current simulation time
Returns 1 on success or 0 on failure
Algorithm:
1) Say forward channel bandwidth is 1Gbps
2) % of small flows 20%
3) % of large flows 80%
4) numSmallFlows = (1Gbps * 20%)/(small flow size)
5) numLargeFlows = (1Gbps * 80%)/(large flow size)
*/
int flowGenerator::generateFlowsByByteSplit(unsigned int currentTime)
{
	bool smallFlowComplete = false;
	bool mediumFlowComplete = false;
	bool largeFlowComplete = false;

	myproperties.numSmallFlowsGenerated = 0;
	myproperties.numLargeFlowsGenerated = 0;
	myproperties.numMediumFlowsGenerated = 0;
    
	do
	{
	
	unsigned int randNum = rand() % 3 + 1;

	#ifdef DEBUG
		printf("Randnum is %u\n",randNum);
	#endif

	switch (randNum)
	{
	case small:
		if ( myproperties.numSmallFlowsGenerated < numSmallFlows )
		{
			if (!generateSmallFlow(currentTime))
			{
				return 0;
			}
			myproperties.numSmallFlowsGenerated++;
						
			#ifdef DEBUG
				printf("Num small flows generated %u\n", myproperties.numSmallFlowsGenerated);
			#endif

			break;
		} else
		{
			smallFlowComplete = true;
		}
	case medium:
		if ( myproperties.numMediumFlowsGenerated < numMediumFlows )
		{
			if (!generateMediumFlow(currentTime))
			{
				return 0;
			}
			myproperties.numMediumFlowsGenerated++;
			
			#ifdef DEBUG
				printf("Num medium flows generated %u\n", myproperties.numMediumFlowsGenerated);
			#endif
			break;
		} else
		{
			mediumFlowComplete = true;
		}
	case large:
		if ( myproperties.numLargeFlowsGenerated < numLargeFlows )
		{
			if (!generateLargeFlow(currentTime))
			{
				return 0;
			}
			myproperties.numLargeFlowsGenerated++;
			
			#ifdef DEBUG
				printf("Num large flows generated %u\n", myproperties.numLargeFlowsGenerated);
			#endif
			break;
		} else
		{
			largeFlowComplete = true;
		}
	}		
	} while(!smallFlowComplete || !mediumFlowComplete || !largeFlowComplete);

	return 1;
}

/*
Func generateFlowsByNumFlowSplit: generate flows by fixing random number range 
as per flow %
Args:
"unsigned int currentTime" - current simulation time
Returns 1 on success or 0 on failure
Algorithm:
1) Say forward channel bandwidth is 1Gbps
2) % of small flows 20%
3) % of large flows 80%
4) If random number is between 1 to 20, then generate small flows
5) If random number is between 21 to 100, then generate large flows
*/
int flowGenerator::generateFlowsByNumFlowSplit(unsigned int currentTime)
{
	bool smallFlowComplete = true;
	bool mediumFlowComplete = true;
	bool largeFlowComplete = true;

	unsigned int numBytesGenerated = 0;

	myproperties.numSmallFlowsGenerated = 0;
	myproperties.numLargeFlowsGenerated = 0;
	myproperties.numMediumFlowsGenerated = 0;
    
	do
	{
	
	unsigned int randNum = rand() % 100 + 1;

	#ifdef DEBUG
		printf("Randnum is %u\n",randNum);
	#endif

	flowTypes typeOfFlow;

	if (randNum >= 1 && randNum <=myproperties.largeFlowPercentage)
	{
		largeFlowComplete = false;
		typeOfFlow = large;		
	} else if (randNum >= myproperties.largeFlowPercentage+1 && randNum <= myproperties.largeFlowPercentage+myproperties.mediumFlowPercentage)
	{
		typeOfFlow = medium;
	} else
	{
		smallFlowComplete = false;
		typeOfFlow = small;
	}

	switch (typeOfFlow)
	{	
	// a tricky switch case wherein failure of condition check in large case
	// take control to small case. This is to generate as much data as possible
	// to fill the forwarding rate
	case large:
		if ( ((getfwdRateInBytes() - numBytesGenerated) / 
		(NUM_PACKETS_LARGE_FLOW * LARGE_FLOW_PACKET_SIZE)) >= 1 )
		{
			if (!generateLargeFlow(currentTime))
			{
				return 0;
			}
			myproperties.numLargeFlowsGenerated++;

			numBytesGenerated = numBytesGenerated + (NUM_PACKETS_LARGE_FLOW 
				* LARGE_FLOW_PACKET_SIZE);
			
			#ifdef DEBUG
				printf("Num large flows generated %u\n", myproperties.numLargeFlowsGenerated);
				printf("Num bytes generated %u\n",numBytesGenerated);
			#endif

			break;

		} else
		{
			#ifdef DEBUG
				printf("Division result %u\n",((getfwdRateInBytes() - numBytesGenerated) / 
		(NUM_PACKETS_LARGE_FLOW * LARGE_FLOW_PACKET_SIZE)));
				printf("Remaining bytes %u < large flow size\n",(getfwdRateInBytes() 
				- numBytesGenerated));
			#endif
			largeFlowComplete = true;

			break;
		}
			
	case small:
		if ( ((getfwdRateInBytes() - numBytesGenerated) / 
		(NUM_PACKETS_SMALL_FLOW * SMALL_FLOW_PACKET_SIZE)) >= 1 )
		{
			if (!generateSmallFlow(currentTime))
			{
				return 0;
			}
			myproperties.numSmallFlowsGenerated++;

			numBytesGenerated = numBytesGenerated + (NUM_PACKETS_SMALL_FLOW 
				* SMALL_FLOW_PACKET_SIZE);
						
			#ifdef DEBUG
				printf("Num small flows generated %u\n", myproperties.numSmallFlowsGenerated);
				printf("Num bytes generated %u\n",numBytesGenerated);
			#endif

			break;

		} else
		{
			#ifdef DEBUG
				printf("Remaining bytes %u < small flow size\n",(getfwdRateInBytes() 
				- numBytesGenerated));
			#endif
			// if remaining bytes is less than a small flow, declar done
			smallFlowComplete = true;
			mediumFlowComplete = true;
			largeFlowComplete = true;

			break;
		}
	
	case medium:
		if ( ((getfwdRateInBytes() - numBytesGenerated) / 
		(NUM_PACKETS_MEDIUM_FLOW * MEDIUM_FLOW_PACKET_SIZE)) >= 1 )
		{
			if (!generateMediumFlow(currentTime))
			{
				return 0;
			}
			myproperties.numMediumFlowsGenerated++;

			numBytesGenerated = numBytesGenerated + (NUM_PACKETS_MEDIUM_FLOW 
				* MEDIUM_FLOW_PACKET_SIZE);
			
			#ifdef DEBUG
				printf("Num medium flows generated %u\n", myproperties.numMediumFlowsGenerated);
			#endif

			break;

		} else
		{
			mediumFlowComplete = true;
		}
	 	
	}		
	} while(!smallFlowComplete || !largeFlowComplete);

	return 1;	
}

/*
Func generateFlowsByTotalNumFlowSplit: generate flows by fixing number of flows
based on flow %
Args:
"unsigned int currentTime" - current simulation time
Returns 1 on success or 0 on failure
Algorithm:
1) Say total number of configured flows is X
2) % of small flows 20%
3) % of large flows 80%
4) numSmallFlows = X * 20%
5) numLargeFlows = X * 80%
6) Forwarding channel bandwidh is equally shared by all the flows
*/
int flowGenerator::generateFlowsByTotalNumFlowSplit(unsigned int currentTime)
{

	myproperties.numSmallFlowsGenerated = 0;
	myproperties.numLargeFlowsGenerated = 0;
	myproperties.numMediumFlowsGenerated = 0;
	numFlowsGenerated = 0;

	// Confirm that per flow share is at least size of a packet
	if ( perLargeFlowShare < LARGE_FLOW_PACKET_SIZE && numLargeFlows != 0)
	{
		printf("Error per flow share < size of a packet\n");
		return 0;
	}

	#ifdef DEBUG
		printf("totalSmallFlowsToGenerate %u\n",numSmallFlows);
		printf("totalLargeFlowsToGenerate %u\n",numLargeFlows);
		printf("numFlowsGenerated %u\n",numFlowsGenerated);
	#endif

	bool largeFlowDone = false;
	while (numFlowsGenerated < numLargeFlows + numSmallFlows)
	{
		unsigned int randNum = rand() % 2 + 1;

		if (randNum == 1 && !largeFlowDone)
		{
			if (myproperties.numLargeFlowsGenerated < numLargeFlows)
			{
				if (!generateLargeFlow(currentTime,'1'))
				{
					printf("Error in generation of large flow\n");
					return 0;
				}
				numFlowsGenerated++;
				myproperties.numLargeFlowsGenerated++;
				
				#ifdef DEBUG
					printf("Num flows generated %u\n",numFlowsGenerated);
				#endif
			} else
			{
				largeFlowDone = true;
			}
		} else
		{
			if (myproperties.numSmallFlowsGenerated < numSmallFlows)
			{
				if (!generateSmallFlow(currentTime,'1'))
				{
					printf("Error in generation of small flow\n");
					return 0;
				}
				numFlowsGenerated++;
				myproperties.numSmallFlowsGenerated++;

				#ifdef DEBUG
					printf("Num flows generated %u\n",numFlowsGenerated);
				#endif
			}
		}

	}

	return 1;
}

/*
Func getFlowGeneratorStatistics: prints the statistics of generated flow at unit time
*/
void flowGenerator::getFlowGeneratorStatistics()
{
	printf("Number of large flows generated %u\n",myproperties.numLargeFlowsGenerated);
	printf("Number of Small flows generated %u\n",myproperties.numSmallFlowsGenerated);
	printf("Number of medium flows generated %u\n",myproperties.numMediumFlowsGenerated);
	printf("Number of flows generated %u\n",numFlowsGenerated);
	printf("At every %u time unit, flow generator push following flows\n",myproperties.flowArrivalInterval);
	printf("Number of large flows %u\n",numLargeFlows);
	printf("Number of Small flows %u\n",numSmallFlows);
	printf("Number of medium flows %u\n",numMediumFlows);
	printf("Per large flow share %u\n",perLargeFlowShare);
	printf("Per small flow share %u\n",perSmallFlowShare);
}
