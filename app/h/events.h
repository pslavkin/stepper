#ifndef EVENTS
#define EVENTS
#include "state_machine.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
//-----------------------------------------------------------
#define MAX_EVENTS      50//40 //40//40//80 //25 //ojo que en linux se mandan paquetes largos y el parser mete todo a eventos... por ejemplo el cambio de ip's es A192.168.002.010 255.255.255.000 192.168.002.001\r\n = 50 EVENTOS!!! mas alguno que ande por alli!!...

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


