#ifndef  TELNET
#define  TELNET

extern void       Init_Telnet  ( void );
extern void       Telnet_Close ( struct tcp_pcb *tpcb);
extern void Gcode_Parser(void* nil);

#endif

