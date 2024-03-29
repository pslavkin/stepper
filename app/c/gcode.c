#include <stdint.h>
#include "utils/cmdline.h"
#include "opt.h"
#include "commands.h"
#include "gcode.h"
#include "utils/uartstdio.h"

QueueHandle_t Gcode_Queue;
struct Gcode_Queue_Struct Actual_Cmd;

void Gcode_Parser(void* nil)
{
   Gcode_Queue= xQueueCreate(GCODE_QUEUE_SIZE,sizeof(struct Gcode_Queue_Struct));
   struct Gcode_Queue_Struct Cmd;
   while(1) {
      xQueueReceive(Gcode_Queue,&Actual_Cmd,portMAX_DELAY);
      Cmd=Actual_Cmd;
      CmdLineProcess ( (char* )Cmd.Buff,Cmd.tpcb);
   }
}

