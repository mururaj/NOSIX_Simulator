#include "system.h"

#define MAXSTEPS 300

// Need to replace later with queue name as args
#define EVENTSCHEDULER 1
#define FLOWARRIVALQ 2
#define SWTICHTOCONTROLLER 3
#define CONTROLLERTOSWITCH 4

 EventQueue *eventScheduler = new EventQueue(EVENTSCHEDULER);

void getFlowInfo(Flow *currentFlow)
{
	switch(currentFlow->flowType) // small, medium or large flow
	{
		case small:
			printf("Small flow\n");
			break;
		case medium:
			printf("Medium flow\n");
			break;
		case large:
			printf("Large flow\n");
			break;
	}

	switch(currentFlow->flowTag)
	{
		case begin:
			printf("Begin flow\n");
		    break;
		case continuing:
			printf("Continuing flow\n");
			break;
		case end:
			printf("End flow\n");
			break;
		case fullflow:
			printf("Full flow\n");
			break;
	}

	printf("flowArrivalTime %u\n",currentFlow->flowArrivalTime);
	if (currentFlow->flowMatching)
	{
		printf("Matching flow\n");
	} else
	{
		printf("Unmatching flow\n");
	}
	printf("Num packets in this flow %u\n",currentFlow->numPackets);
	printf("Flow number %u\n",currentFlow->flowNumber);	
}

int main()
{
	// Create flow arrival queue
	Queue flowArrivalQ = Queue(FLOWARRIVALQ);

	// Create switch to controller queue
	Queue controlChannelQ = Queue(SWTICHTOCONTROLLER);

	// Create controller to switch message queue
	Queue controlToSwitchQ = Queue(CONTROLLERTOSWITCH);

	#ifdef DEBUG
		eventScheduler->getQueueStatistics();
		flowArrivalQ.getQueueStatistics();
		controlChannelQ.getQueueStatistics();
		controlToSwitchQ.getQueueStatistics();
	#endif

	// Probably need to design application policy with following parameters
	flowGenProps flowGen1Properties;
	flowGen1Properties.largeFlowPercentage = 20;
	flowGen1Properties.smallFlowPercentage = 80;
	flowGen1Properties.mediumFlowPercentage = 0;
	flowGen1Properties.flowArrivalInterval = 1;
	flowGen1Properties.totalNumberOfFlows = 3500;
	
	SwitchProps switch1properties;

	// Initialize the properties of the switch
	switch1properties.switchModel = 1;
	switch1properties.forwardingRate = 1; //1Gbps
	switch1properties.numFlowTables = 1; // only 1 flow table
	switch1properties.perTableNumFlowEntries = 500; // 500 flow entries
	switch1properties.ctrlChannelRate = 10; //10Mbps
	switch1properties.numFlowsInstalled = 0; // Initially no flow pre installed
	switch1properties.numFlowEntriesBySmall = 0; // Initially no small flow pre installed
	switch1properties.numFlowEntriesByLarge = 0; // Initially no large flow pre installed

	ControllerProps controller1properties;

	// Initialize the properties of the controller
	controller1properties.controllerModel = 1;
	controller1properties.ctrlChannelRate = 10; //10Mbps

	// Instantiate new flow generator component passing address of 
	// flow arrival queue & largeFlowPercentage set to 100%
	flowGenerator flowGen1 = flowGenerator(flowGen1Properties,
		                    &flowArrivalQ,switch1properties.forwardingRate);

	// Instantiate new switch component passing properties of 
	// this specific switch & address of flow arrival queue
	sdnSwitch sw1 = sdnSwitch(switch1properties,&flowArrivalQ,
							  &controlChannelQ,&controlToSwitchQ);

	// Instantiate new controller component passing properties of
	// this specific controller & address of shared queues
	sdnController cntrl1 = sdnController(controller1properties,
					&controlChannelQ,&controlToSwitchQ);

	// Schedule initial tasks
	// Schedule next flow arrival event
	unsigned int beginClock = 1;
	
	Event *newflowArrivalEvent = new Event();
	newflowArrivalEvent->eventTime = beginClock;
	newflowArrivalEvent->eventType = flowArrivalEvent;
	eventScheduler->append((void *) newflowArrivalEvent);

    
	Event *newflowProcessingEvent = new Event();
	newflowProcessingEvent->eventTime = beginClock;
	newflowProcessingEvent->eventType = switchProcessingEvent;
	eventScheduler->append((void *) newflowProcessingEvent);

	Event *contrlProcessingEvent = new Event();
	contrlProcessingEvent->eventTime = beginClock;
	contrlProcessingEvent->eventType = controllerProcessingEvent;
	eventScheduler->append((void *) contrlProcessingEvent);

	unsigned int simulationDuration = MAXSTEPS;
	
	// Simulator runs through FlowGenerator -> Switch components
	for (unsigned int i=1;i<=simulationDuration;i++)
	{
		Event *currentEvent =(Event *)eventScheduler->remove();

		if (currentEvent)
		{
			switch (currentEvent->eventType)
			{
			case flowArrivalEvent:
				printf("Processing arrivalevent\n");
				flowGen1.run(currentEvent->eventTime);
				break;
			case switchProcessingEvent:
				printf("Processing switchingevent\n");
				sw1.run3(currentEvent->eventTime);
				break;
			case controllerProcessingEvent:
				printf("Processing controllerevent\n");
				cntrl1.run(currentEvent->eventTime);
			}
		
			// Free the event
			delete(currentEvent);

			#ifdef DEBUG
				printf("Simulator Clock: %u\n",i);
				if (!eventScheduler->printScheduledEvents())
				{
					printf("Error unable to print the event\n");
				}
			#endif
		} else
		{
			printf("Error no event in event queue\n");
			break;
		}
	}
	
	return 0;
}
