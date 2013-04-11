#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdio.h>
#include "generic_queue.h"
#include "event_queue.h"
#include "flowGenerator.h"
#include "sdnSwitch.h"
#include "sdnController.h"

//#define DEBUG

// Create global event scheduler 
extern EventQueue *eventScheduler;

extern void getFlowInfo(Flow *currentFlow);

#endif //SYSTEM_H
