#ifndef FLOWGEN_H
#define FLOWGEN_H

#include <stdio.h>


// Define packet size (in bytes) of flows
#define SMALL_FLOW_PACKET_SIZE   512 // Size of DNS query packet
#define MEDIUM_FLOW_PACKET_SIZE  1024  // Size of medium flow packet
#define LARGE_FLOW_PACKET_SIZE  1500 // Max size of ethernet frame

// Define Number of packets per flow
#define NUM_PACKETS_SMALL_FLOW    20  //10240 bytes - 20 DNS query packets - 10kb total size
#define NUM_PACKETS_MEDIUM_FLOW 1000 // 1024000 bytes - 1MB total size
#define NUM_PACKETS_LARGE_FLOW 70000 // 105000000 bytes - 100MB total size

#define NUMBYTESPERGB 1073741824

#define TOTALNUMFLOWS 5000

#include "generic_queue.h"
#include "event_queue.h"

enum flowTypes { small=1, medium, large };
enum flowTags { begin=1, continuing, end, fullflow };
enum flowActions { PACKET_IN=1, PACKET_OUT, FLOW_INSTALL, PROCESS_PACKET, NIL};

//Structure to record the flow characteristics
typedef struct flow
{
	flowTypes flowType; // small, medium or large flow
	unsigned int flowArrivalTime; // Time at which the flow is inserted into queue
	bool flowMatching; // Whether this flow matches entries in flow table
	flowTags flowTag; // Designate begining and end of the flow
	unsigned int numPackets; // Number of packets included in this flow
	unsigned int flowNumber; // An identification number of this flow
	flowActions flowAction; // Specify the action associated with the flow
}Flow;

//Structure to record flowGen(app) properties. 
typedef struct flowGenProps
{
		unsigned int largeFlowPercentage;
		unsigned int mediumFlowPercentage;
		unsigned int smallFlowPercentage;
		unsigned int flowArrivalInterval;
		unsigned int numSmallFlowsGenerated;
		unsigned int numLargeFlowsGenerated;
		unsigned int numMediumFlowsGenerated;
		unsigned int totalNumberOfFlows; // total flows to be generated by flow generator per unit time
} FlowGenProps;


/*
flowGenerator class represents the flow generation component.
The flow arrival queue and % of large, small & medium traffic
must be passed for every flowGenerator object. 
*/
class flowGenerator
{
	public:
		
		flowGenerator(FlowGenProps myProps,Queue *flowArrQueue,
		            unsigned int switchFwdRate) : flowArrivalQueue(flowArrQueue),
			 switchForwardingRate(switchFwdRate)			 			
		{
			
			myproperties = myProps;
			myproperties.numSmallFlowsGenerated = 0; //explicity initialized 
			myproperties.numMediumFlowsGenerated = 0; //explicity initialized
			myproperties.numLargeFlowsGenerated = 0; //explicity initialized
			numFlowsGenerated = 0; //total num of flows generated so far
			
			numSmallFlows = myproperties.totalNumberOfFlows *
											myproperties.smallFlowPercentage / 100;
			numLargeFlows = myproperties.totalNumberOfFlows * 
											myproperties.largeFlowPercentage / 100;	 
			
			// Check if evenly dividing BW is surplus of small flow share
			if ( getfwdRateInBytes()/myproperties.totalNumberOfFlows > 
				NUM_PACKETS_SMALL_FLOW * SMALL_FLOW_PACKET_SIZE)
			{
				perSmallFlowShare = NUM_PACKETS_SMALL_FLOW * SMALL_FLOW_PACKET_SIZE;
				if (numLargeFlows == 0)
				{
					perLargeFlowShare = 0;
				} else
				{
					perLargeFlowShare = ( getfwdRateInBytes() - 
									(numSmallFlows * perSmallFlowShare) ) / numLargeFlows;
				}
			} else
			{
			   perSmallFlowShare = getfwdRateInBytes()/myproperties.totalNumberOfFlows;
			   perLargeFlowShare = getfwdRateInBytes()/myproperties.totalNumberOfFlows;
			}
			
			// Initialize flow tracker array
			for (unsigned int i=0;i<TOTALNUMFLOWS;i++)
			{
				flowTracker[i]= 0;
			}		
		 
		}


		unsigned int getfwdRateInBytes()
		{
		     unsigned int fwdRateInBytes = switchForwardingRate * NUMBYTESPERGB;
			 return fwdRateInBytes;
		}
		
		int run(unsigned int currentTime);

		void getFlowGeneratorStatistics();

		int generateFlowsByByteSplit(unsigned int currentTime);

		int generateFlowsByNumFlowSplit(unsigned int currentTime);

		int generateFlowsByTotalNumFlowSplit(unsigned int currentTime);

		int generateMediumFlow(unsigned int currentTime);

		int generateSmallFlow(unsigned int currentTime, char whichMethod);

		int generateLargeFlow(unsigned int currentTime, char whichMethod);

	private:
		FlowGenProps myproperties;
		Queue *flowArrivalQueue;		
		unsigned int switchForwardingRate;
		unsigned int numSmallFlows;
		unsigned int numMediumFlows;
		unsigned int numLargeFlows;
		unsigned int numFlowsGenerated;
		unsigned int perLargeFlowShare;
		unsigned int perSmallFlowShare;
		unsigned int flowTracker[TOTALNUMFLOWS];		
};

#endif //FLOWGEN_H
