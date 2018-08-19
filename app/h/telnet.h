#ifndef  TELNET
#define  TELNET

#include "utils/ringbuf.h"
#include "gcode.h"



struct Telnet_Args
{
   tRingBufObject RB;
   uint8_t Buff[APP_INPUT_BUF_SIZE];
   uint32_t Id;
};

extern void    Print_Slide  ( struct Gcode_Queue_Struct *D );
extern void    Init_Telnet  ( void                         );
extern void    Telnet_Close ( struct tcp_pcb *tpcb         );

#endif

