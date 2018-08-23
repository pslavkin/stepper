#ifndef __COMMANDS_H__
#define __COMMANDS_H__


extern void Init_Uart0(void);
extern int Cmd_Welcome ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Login   ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Help    ( struct tcp_pcb* tpcb ,int argc ,char *argv[] );
extern int Cmd_Exit    ( struct tcp_pcb* tpcb, int argc, char *argv[] );


extern int Cmd_Back2Login     ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Back2Main      ( struct tcp_pcb* tpcb, int argc, char *argv[] );

extern int Cmd_Main2Ip        ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Main2Motor     ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Main2System    ( struct tcp_pcb* tpcb, int argc, char *argv[] );

extern int Cmd_Mac            ( struct tcp_pcb* tpcb ,int argc ,char *argv[] );
extern int Cmd_Ip             ( struct tcp_pcb* tpcb ,int argc ,char *argv[] );
extern int Cmd_Mask           ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Gateway        ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Dhcp           ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Config_Port    ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Link_State     ( struct tcp_pcb* tpcb, int argc, char *argv[] );

extern int Cmd_TaskList  ( struct tcp_pcb* tpcb ,int argc ,char *argv[] );
extern int Cmd_Uptime    ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Wdog_Tout ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Reboot    ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Pwd       ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Id        ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Show_Id   ( struct tcp_pcb* tpcb, int argc, char *argv[] );

extern int Cmd_Spi_Init       ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Spi_Run        ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Spi_Step       ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Spi_Move       ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Spi_Goto       ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Spi_Goto_Dir   ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Spi_Goto_Until ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Spi_Home       ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Spi_Go_Mark    ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Spi_Reset_Pos  ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Spi_Reset      ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Spi_Soft_Stop  ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Spi_Hard_Stop  ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Spi_Soft_Hiz   ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Spi_Hard_Hiz   ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Spi_Status     ( struct tcp_pcb* tpcb, int argc, char *argv[] );

extern int Cmd_Speed     ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Acc       ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Dec       ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Max_Speed ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Min_Speed ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Wait      ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Nowait    ( struct tcp_pcb* tpcb, int argc, char *argv[] );

extern int Cmd_Spi_Set_Param ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Spi_Get_Param ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Toogle_Pulses ( struct tcp_pcb* tpcb, int argc, char *argv[] );

extern int Cmd_Main2Esp(struct tcp_pcb* tpcb, int argc, char *argv[]);
extern int Cmd_Pipe_Esp(struct tcp_pcb* tpcb, int argc, char *argv[]);
extern int Cmd_Esp_Bind(struct tcp_pcb* tpcb, int argc, char *argv[]);

void User_Commands_Task       ( void* nil                                    );
extern void DisplayIPAddress  ( struct tcp_pcb* tpcb,uint32_t ui32Addr       );

#endif // __COMMANDS_H__
