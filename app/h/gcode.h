#ifndef  GCODE
#define  GCODE

#define GCODE_QUEUE_SIZE 20

struct Gcode_Queue_Struct
{
   uint8_t Buff[APP_INPUT_BUF_SIZE];
   struct tcp_pcb* tpcb;
   uint32_t Id;
};
extern QueueHandle_t Gcode_Queue;
extern void          Gcode_Parser(void* nil);
extern struct Gcode_Queue_Struct Actual_Cmd;

#endif

