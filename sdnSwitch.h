#ifndef SDNSWITCH_H
#define SDNSWITCH_H

#include <stdio.h>
#include "generic_queue.h"

#define SWITCHPROCESSTIME 1
#define NUMBYTESPERMB 1048576
// 105000 -> Worst case the queue can hold all small flows of total size 1Gb
#define WAITFLOWQUEUEMAXSIZE 105000
// conservatively declaring max size to hold all small flows
#define ACTIVEFLOWQUEUEMAXSIZE 105000

// Need to replace with queue names
#define TMPQUEUE 5
#define WAITFLOWQ 6
#define ACTIVEQ 7

/* 
Structure to record switch properties. 
Need to expand list with all important properties -
recorded in application policy & switch spec doc
*/
typedef struct switchProps
{
	unsigned int switchModel;             // switch model number
	unsigned int forwardingRate;   // Forwarding rate in Gbps
	unsigned int ctrlChannelRate;  // control channel rate
	unsigned int numFlowTables;    // Number of flow tables
	unsigned int perTableNumFlowEntries; // Number of flow entries per table
	unsigned int numFlowsInstalled; // Number of flow entries installed at present
	unsigned int numFlowEntriesBySmall; // Number of flow table entries occupied by small flows
	unsigned int numFlowEntriesByLarge; // Number of flow table entries occupued by large flows
}SwitchProps;


/*
sdnSwitch class represents switch component.
The properties of the switch and pointer to flow -
arrival queue must be passed for every switch object
*/
class sdnSwitch
{
	public:
	sdnSwitch()
	{
	}

	sdnSwitch(SwitchProps myprops,Queue *flowArrQueue,
			 Queue *cntrlChannelQueue,Queue *cntrlToSwitchQueue)
	{
		flowArrivalQueue = flowArrQueue;      // shared Queue flowGen<->Switch
		switchToControllerQueue = cntrlChannelQueue; //shared Queue switch<->controller
		waitingFlowsQueue = new Queue(WAITFLOWQ,
								WAITFLOWQUEUEMAXSIZE);
		activeFlowsQueue = new Queue(ACTIVEQ,
								ACTIVEFLOWQUEUEMAXSIZE);
		controllerToSwitchQueue = cntrlToSwitchQueue; //shared Queue controller<->Switch
		tmpIncomingFlowQueue = new Queue(TMPQUEUE,
								WAITFLOWQUEUEMAXSIZE); // This queue may not be required. 
		myproperties = myprops; 
		numBytesForwarded = 0;
		numBytesDropped = 0;
		numFlowsForwarded = 0;
		numLargeFlowsForwarded = 0;
		numSmallFlowsForwarded = 0;
		numPktsForwarded = 0;
		numFlowsDropped = 0;
		numSmallFlowsDropped = 0;
		numLargeFlowsDropped = 0;
	}
	
	int run(unsigned int currentTime);

	int run3(unsigned int currentTime);

	void fastPath(unsigned int flowSize);

	void getDataPlaneStatistics();

	unsigned int getCtrlChanlRateInBytes();
	
    unsigned int getfwdRateInBytes();

	unsigned int findSmallFlowEntry();

	unsigned int receiveIncomingFlows();

	unsigned int receiveControllerMsgs();

	unsigned int forwardFlows();

	unsigned int sendControllerMsgs();

	~sdnSwitch()
	{
		delete waitingFlowsQueue;
		delete activeFlowsQueue;
		delete tmpIncomingFlowQueue;
	}

	private:
		Queue *flowArrivalQueue;
		Queue *switchToControllerQueue;
		Queue *waitingFlowsQueue;
		Queue *activeFlowsQueue;
		Queue *controllerToSwitchQueue;
		Queue *tmpIncomingFlowQueue;
		SwitchProps myproperties;
		unsigned int numBytesForwarded;
		unsigned int numBytesDropped;
		unsigned int numFlowsForwarded;
		unsigned int numLargeFlowsForwarded;
		unsigned int numSmallFlowsForwarded;
		unsigned int numPktsForwarded;
		unsigned int numFlowsDropped;
		unsigned int numSmallFlowsDropped;
		unsigned int numLargeFlowsDropped;
};

#endif //SDNSWITCH_H
