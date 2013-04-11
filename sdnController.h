/*
Author: Muruganantham Raju
Copyright: This code is part of on going research work at USC. 
           Please consult mraju@usc.edu, if you want to use
		   full/portion of this implementation work.

This file declares flowGenerator component 
*/

#ifndef SDNCONTROLLER_H
#define SDNCONTROLLER_H

#include<stdio.h>
#include "generic_queue.h"

#define CONTRLPROCESSINGTIME 1

/* 
Structure to record controller properties. 
Need to expand list with all important properties -
recorded in application policy & switch spec doc
*/
typedef struct controllerProps
{
	unsigned int controllerModel;             // NOX,POX,FLOODLIGHT etc
	unsigned int ctrlChannelRate;  // control channel rate
}ControllerProps;

class sdnController
{
	public:
		sdnController(ControllerProps myprops,
					Queue *cntrlChannelQueue,
					Queue *cntrlToSwitchQueue)
		{
			myproperties = myprops;
			switchToControllerQueue = cntrlChannelQueue; //shared Queue switch<->controller
			controllerToSwitchQueue = cntrlToSwitchQueue; //shared Queue controller<->Switch
			numLargeFlowsExamined = 0;
			numSmallFlowsExamined = 0;
			totalNumFlowsExamined = 0;
			totalNumPktsExamined = 0;
			totalNumFlowsProcessed = 0;
			numBytesExamined = 0;
			numBytesProcessed = 0;
		} 

		int run(unsigned int currentTime); 

		void getControllerStatistics();

	private:
		ControllerProps myproperties;
		Queue *switchToControllerQueue;
		Queue *controllerToSwitchQueue;
		unsigned int numLargeFlowsExamined;
		unsigned int numSmallFlowsExamined;
		unsigned int totalNumFlowsExamined;
		unsigned int totalNumPktsExamined;
		unsigned int totalNumFlowsProcessed; //PROCESS_FLOW type packets
		unsigned int numBytesExamined;
		unsigned int numBytesProcessed;			
};

#endif

