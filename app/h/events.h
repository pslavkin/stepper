#ifndef EVENTS
#define EVENTS
#include "state_machine.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
//-----------------------------------------------------------

typedef struct Events {
    uint16_t      Event;
    const State** Machine;
}Events;

extern QueueHandle_t Events_Queue;
//-----------------------------------------------------------
void    Init_Events  ( void                                 );
void    Send_Event   ( uint16_t Event,const State** Machine );
void    Insert_Event ( uint16_t Event,const State** Machine );
//-------------------------------------------------------------
#endif


