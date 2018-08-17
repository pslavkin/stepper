#include "events.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

//-----------------------------------------------------------------
QueueHandle_t Events_Queue;
void Init_Events(void)
{
   Events_Queue=xQueueCreate ( MAX_EVENTS,sizeof(Events ));
}
//-----------------------------------------------------------------
void Send_Event(uint16_t Event,const State** Machine)   //escribe un nuevo dato a la cola...
{
   Events E={Event,Machine};
   xQueueSend(Events_Queue,&E,portMAX_DELAY);
}
//-------------------------------------------------------------------------------------
void Insert_Event(uint16_t Event,const State** Machine) // escribe un nuevo dato a la cola...
{
   Events E={Event,Machine};
   xQueueSendToFront(Events_Queue,&E,portMAX_DELAY);
}

