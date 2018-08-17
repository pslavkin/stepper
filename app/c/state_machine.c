#include <stdbool.h>
#include <stdint.h>
#include "driverlib/sysctl.h"
#include "utils/uartstdio.h"
#include "inc/hw_types.h"
#include "inc/hw_nvic.h"
#include "state_machine.h"
#include "events.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

const State* ActualState;
Events Event;

//-----------------------------------------------------------------------
void           Set_State  ( const State* New_State,const State** Machine ) { *Machine=New_State                       ;}
const State**  Empty_Sm   ( void                                         ) { return (const State**)Empty_State_Machine;}
void           Rien       ( void                                         ) { }
void Delay_Useg(uint32_t d)
{
   MAP_SysCtlDelay((configCPU_CLOCK_HZ/3000000)*d);
}
//-----------------------------------------------------------------------
uint16_t       Actual_Event ( void ) { return Event.Event  ;}
const State**  Actual_Sm    ( void ) { return Event.Machine;}
void           Soft_Reset   ( void ) {
 HWREG(NVIC_APINT) = NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ;
}
//-----------------------------------------------------------------------
void State_Machine(void* nil)               //esta funcion ejecuta la maquina de estados donde el evento viene en la variable Event... que se decidio que no sea por parametro para permitir la recursividad infinita...  
{
   while(1) {
      while(xQueueReceive(Events_Queue,&Event,portMAX_DELAY) == pdFALSE)
         ;
      if(Event.Machine!=Empty_State_Machine) {
         ActualState = *(Event.Machine);
         for(;ActualState->Event!=ANY_Event && ActualState->Event!=Event.Event;ActualState++)
            ;
         *Event.Machine=ActualState->Next_State;
         ActualState->Func();
      }
   }
}

//------------------------------------------------------------------------------------------
