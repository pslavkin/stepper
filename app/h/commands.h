#ifndef __COMMANDS_H__
#define __COMMANDS_H__

extern int Cmd_help              ( struct tcp_pcb* tpcb ,int argc ,char *argv[] );
extern int Cmd_Mac               ( struct tcp_pcb* tpcb ,int argc ,char *argv[] );
extern int Cmd_Ip                ( struct tcp_pcb* tpcb ,int argc ,char *argv[] );
extern int Cmd_Send2Eth          ( struct tcp_pcb* tpcb ,int argc ,char *argv[] );
extern int Cmd_TaskList          ( struct tcp_pcb* tpcb ,int argc ,char *argv[] );
extern int Cmd_Links_State       ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Spi_App_Cmd       ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Spi_Get_Param_Cmd ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern void UpdateMACAddr        ( struct tcp_pcb* tpcb                         );
extern void User_Commands_Task   ( void* nil                                    );
extern void DisplayIPAddress     ( struct tcp_pcb* tpcb,uint32_t ui32Addr       );


#endif // __COMMANDS_H__
