#include <stdio.h>/*{{{*/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "driverlib/sysctl.h"
#include "driverlib/emac.h"
#include "driverlib/flash.h"
#include "inc/hw_memmap.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "utils/cmdline.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "utils/lwiplib.h"
#include "state_machine.h"
#include "commands.h"
#include "telnet.h"
#include "opt.h"
#include "wdog.h"
#include "usr_flash.h"
#include "schedule.h"
#include "spi_phisical.h"
#include "gcode.h"
#include "third_party/lwip-1.4.1/src/include/ipv4/lwip/ip_addr.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"/*}}}*/
//--------------------------------------------------------------------------------
tCmdLineEntry* g_psCmdTable;

tCmdLineEntry Login_Cmd_Table[] =
{
    { "login"  ,Cmd_Login   ,": login" }                                ,
    { "id"     ,Cmd_Show_Id ,": show id name" }                         ,
    { "exit"   ,Cmd_Exit    ,": exit and close connection" }            ,
    { "?"      ,Cmd_Help    ,": help" }                                 ,
    { 0        ,0           ,0 }
};
tCmdLineEntry Main_Cmd_Table[] =
{
    { "net"    ,Cmd_Main2Ip     ,": network options" } ,
    { "motor"  ,Cmd_Main2Motor  ,": motor driver" }    ,
    { "system" ,Cmd_Main2System ,": system options" }  ,
    { "esp"    ,Cmd_Main2Esp    ,": esp options" }     ,
    { "?"      ,Cmd_Help        ,": help" }            ,
    { "<"      ,Cmd_Back2Login  ,": back" }            ,
    { 0        ,0               ,0 }
};
tCmdLineEntry Ip_Cmd_Table[] =
{
    { "mac"  ,Cmd_Mac         ,": show MAC address" }                                 ,
    { "ip"   ,Cmd_Ip          ,": show and/or save ip" }                              ,
    { "mask" ,Cmd_Mask        ,": show and/or save mask" }                            ,
    { "gw"   ,Cmd_Gateway     ,": show and/or save gateway" }                         ,
    { "dhcp" ,Cmd_Dhcp        ,": show and/or save dhcp (0=disable 1=enable)" }       ,
    { "cp"   ,Cmd_Config_Port ,": show and/or save config tcp port [default 49152]" } ,
    { "link" ,Cmd_Link_State  ,": show link state" } ,
    { "?"    ,Cmd_Help        ,": help" }                                             ,
    { "<"    ,Cmd_Back2Main   ,": back" }                                             ,
    { 0      ,0               ,0 }
};
tCmdLineEntry Motor_Cmd_Table[] =
{
    { "init"    ,Cmd_Spi_Init       ,": init regs" }                                                                          ,
    { "run"     ,Cmd_Spi_Run        ,": Sets the target speed and the motor direction" }                                      ,
    { "step"    ,Cmd_Spi_Step       ,": Puts the device in Step-clock" }                                                      ,
    { "move"    ,Cmd_Spi_Move       ,": Makes N_STEP (micro)steps in DIR direction (Not performable when motor is running)" } ,
    { "goto"    ,Cmd_Spi_Goto       ,": Brings motor in ABS_POS position (minimum path)" }                                    ,
    { "gotod"   ,Cmd_Spi_Goto_Dir   ,": Brings motor in ABS_POS position forcing DIR direction" }                             ,
    { "gountil" ,Cmd_Spi_Goto_Until ,": Performs a motion in DIR direction with speed SPD until SW is closed" }               ,
    { "home"    ,Cmd_Spi_Home       ,": Brings the motor in HOME position" }                                                  ,
    { "mark"    ,Cmd_Spi_Go_Mark    ,": Brings the motor in MARK position" }                                                  ,
    { "rstpos"  ,Cmd_Spi_Reset_Pos  ,": Resets the ABS_POS register (sets HOME position)" }                                   ,
    { "rst"     ,Cmd_Spi_Reset      ,": Device is reset to power-up conditions" }                                             ,
    { "stop"    ,Cmd_Spi_Soft_Stop  ,": Stops motor with a deceleration phase" }                                              ,
    { "hstop"   ,Cmd_Spi_Hard_Stop  ,": Stops motor immediately" }                                                            ,
    { "hiz"     ,Cmd_Spi_Soft_Hiz   ,": Puts the bridges in high impedance status after a deceleration phase" }               ,
    { "hhiz"    ,Cmd_Spi_Hard_Hiz   ,": Puts the bridges in high impedance status immediately" }                              ,
    { "stat"    ,Cmd_Spi_Status     ,": Returns the status register value" }                                                  ,
    { "sp"      ,Cmd_Spi_Set_Param  ,": Set param comand ej: sp 5 1234" }                                                     ,
    { "gp"      ,Cmd_Spi_Get_Param  ,": Get param comand ej: gp 5" }                                                          ,
    { "pulse"   ,Cmd_Toogle_Pulses  ,": Toogle pulses con direccino ej.pulse 100 1" }                                         ,
    { "speed"   ,Cmd_Speed      ,": actual speed" }                                                                      ,
    { "acc"     ,Cmd_Acc            ,": Acceleration" }                                                                       ,
    { "dec"     ,Cmd_Dec            ,": Decceleration" }                                                                      ,
    { "maxv"    ,Cmd_Max_Speed      ,": Maximum speed" }                                                                      ,
    { "minv"    ,Cmd_Min_Speed      ,": Minimim speed" }                                                                      ,
    { "wait"    ,Cmd_Wait           ,": wait" }                                                                               ,
    { "nowait"  ,Cmd_Nowait         ,": no wait" }                                                                            ,
    { "?"       ,Cmd_Help           ,": help" }                                                                               ,
    { "<"       ,Cmd_Back2Main      ,": back" }                                                                               ,
    { 0         ,0                  ,0 }
};
tCmdLineEntry System_Cmd_Table[] =
{
    { "id"     ,Cmd_Id        ,": show and/or save id" }       ,
    { "pwd"    ,Cmd_Pwd       ,": show and/or save password" } ,
    { "reboot" ,Cmd_Reboot    ,": reboot" }                    ,
    { "task"   ,Cmd_TaskList  ,": rsv" }                       ,
    { "uptime" ,Cmd_Uptime    ,": uptime [secs]" }             ,
    { "wdog"   ,Cmd_Wdog_Tout ,": wdog tout [secs] (0 disable)" }          ,
    { "?"      ,Cmd_Help      ,": help" }                      ,
    { "<"      ,Cmd_Back2Main ,": back" }                      ,
    { 0        ,0             ,0 }
};
tCmdLineEntry Esp_Cmd_Table[] =
{
    { "r" ,Cmd_Pipe_Esp  ,": retransmit to esp" } ,
    { "b" ,Cmd_Esp_Bind  ,": esp bind port" } ,
    { "?" ,Cmd_Help      ,": help" }              ,
    { "<" ,Cmd_Back2Main ,": back" }              ,
    { 0   ,0             ,0 }
};
//----------------------WELCOME----------------------------------------------------------
int Cmd_Welcome(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
   g_psCmdTable=Motor_Cmd_Table;
   UART_ETHprintf(tpcb,"\033[2J\033[H+++ PaP V0.1 +++\r\ntp final protocolos - cese2018\r\n");
   return 0;
}/*}}}*/
int Cmd_Help(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
    tCmdLineEntry *pEntry;
    UART_ETHprintf(tpcb,"\r\navailable commands\r\n------------------\r\n");
    pEntry = g_psCmdTable;
    for(;pEntry->pcCmd;pEntry++)
        UART_ETHprintf(tpcb,"%15s%s\r\n", pEntry->pcCmd, pEntry->pcHelp);
    return 0;
}/*}}}*/
//---------------------MAIN-----------------------------------------------------------
int Cmd_Login(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
   if(argc>1) {
      if(ustrcmp(argv[1],Usr_Flash_Params.Pwd)==0) {
         UART_ETHprintf(DEBUG_MSG,"success\r\n");
         g_psCmdTable=Main_Cmd_Table;
         Cmd_Help(DEBUG_MSG,argc,argv);
      }
      else
         UART_ETHprintf(tpcb,"invalid\r\n");
   }
   else
      UART_ETHprintf(tpcb,"no pwd\r\n");
   return 0;
}/*}}}*/
int Cmd_Exit(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
   Telnet_Close(tpcb);
   return 0;
}/*}}}*/
int Cmd_Main2Ip(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
   g_psCmdTable=Ip_Cmd_Table;
   return 0;
}/*}}}*/
int Cmd_Main2Motor(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
   g_psCmdTable=Motor_Cmd_Table;
   return 0;
}/*}}}*/
int Cmd_Main2System(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
   g_psCmdTable=System_Cmd_Table;
   return 0;
}/*}}}*/
int Cmd_Main2Esp(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
   g_psCmdTable=Esp_Cmd_Table;
   return 0;
}/*}}}*/
int Cmd_Back2Main(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
   g_psCmdTable=Main_Cmd_Table;
   return 0;
}/*}}}*/
int Cmd_Back2Login(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
   g_psCmdTable=Login_Cmd_Table;
   return 0;
}/*}}}*/
//--------------------NET-----------------------------------------------------------------------------
int Cmd_Mac(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
   uint8_t Actual_Mac[6];
   uint8_t* Mac;
   if(argc==7) {
      uint8_t i;
      Mac=Usr_Flash_Params.Mac_Addr;
      for(i=0;i<6;i++)
         Mac[i]=hextoc(argv[i+1]);
      Save_Usr_Flash();
      UART_ETHprintf(tpcb,"new ");
   }
   else {
      Mac=Actual_Mac;
      lwIPLocalMACGet ( Mac );
   }
   UART_ETHprintf(tpcb,"mac: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                        Mac[0], Mac[1], Mac[2], Mac[3], Mac[4], Mac[5]);
   return 0;
}/*}}}*/
int Cmd_Ip(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
   if(argc==1)
      DisplayIPAddress(tpcb,lwIPLocalIPAddrGet());
   else {
      ip_addr_t New_Ip;
      UART_ETHprintf(tpcb,"new ip:");
      if(ipaddr_aton(argv[1],&New_Ip) == 1) {
         DisplayIPAddress(tpcb,New_Ip.addr);
         Usr_Flash_Params.Ip_Addr=htonl(New_Ip.addr);
         Save_Usr_Flash();
      }
      else {
         UART_ETHprintf(tpcb,"invalid ip:%s\r\n",argv[1]);
         DisplayIPAddress(tpcb,New_Ip.addr);
      }
   }
   return 0;
}/*}}}*/
int Cmd_Mask(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
   if(argc==1)
      DisplayIPAddress(tpcb,lwIPLocalNetMaskGet());
   else {
      ip_addr_t New_Ip;
      UART_ETHprintf(tpcb,"new mask:");
      if(ipaddr_aton(argv[1],&New_Ip) == 1) {
         DisplayIPAddress(tpcb,New_Ip.addr);
         Usr_Flash_Params.Mask_Addr=htonl(New_Ip.addr);
         Save_Usr_Flash();
      }
      else
         UART_ETHprintf(tpcb,"invalid\r\n");
   }
   return 0;
}/*}}}*/
int Cmd_Gateway(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
   if(argc==1)
      DisplayIPAddress(tpcb,lwIPLocalGWAddrGet());
   else {
      ip_addr_t New_Ip;
      UART_ETHprintf(tpcb,"new gateway:");
      if(ipaddr_aton(argv[1],&New_Ip) == 1) {
         DisplayIPAddress(tpcb,New_Ip.addr);
         Usr_Flash_Params.Gateway_Addr=htonl(New_Ip.addr);
         Save_Usr_Flash();
      }
      else
         UART_ETHprintf(tpcb,"invalid\r\n");
   }
   return 0;
}/*}}}*/
int Cmd_Dhcp(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
   if(argc==1)
      UART_ETHprintf(tpcb,"%d\r\n",Usr_Flash_Params.Dhcp_Enable);
   else {
      uint16_t New=atoi(argv[1]);
      UART_ETHprintf(tpcb,"new dhcp=%d \r\n",New);
      Usr_Flash_Params.Dhcp_Enable=New;
      Save_Usr_Flash();
   }
   return 0;
}/*}}}*/
int Cmd_Config_Port(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
   if(argc==1)
      UART_ETHprintf(tpcb,"%d\r\n",Usr_Flash_Params.Config_Port);
   else {
      uint16_t New_Port=atoi(argv[1]);
      UART_ETHprintf(tpcb,"new port=%d\r\n",New_Port);
      Usr_Flash_Params.Config_Port=New_Port;
      Save_Usr_Flash();
   }
   return 0;
}/*}}}*/
void DisplayIPAddress(struct tcp_pcb* tpcb,uint32_t ui32Addr)
{/*{{{*/
    UART_ETHprintf(tpcb, "%d.%d.%d.%d\r\n", ui32Addr & 0xff, (ui32Addr >> 8) & 0xff,
            (ui32Addr >> 16) & 0xff, (ui32Addr >> 24) & 0xff);
}/*}}}*/
//---------------SYSTEM-----------------------------------------------------------
int Cmd_TaskList(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
  // if(argc== 2  && ustrcmp("tareas",argv[1])==0) 
   {
      char* Buff=(char*)pvPortMalloc(UART_TX_BUFFER_SIZE);
      UART_ETHprintf(tpcb,"\r\n");
      vTaskList( Buff );
      UART_ETHprintf(tpcb,Buff);
      vPortFree(Buff);
   }
   return 0;
}/*}}}*/
int Cmd_Uptime(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
   UART_ETHprintf(tpcb,"%d\r\n",Read_Uptime());
   return 0;
}/*}}}*/
int Cmd_Wdog_Tout(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
   if(argc==1)
      UART_ETHprintf(tpcb,"%d secs\r\n",Usr_Flash_Params.Wdog);
   else {
      uint16_t New_Time=atoi(argv[1]);
      if(New_Time>=120 || New_Time==0) {
         UART_ETHprintf(tpcb,"new wdog=%d secs\r\n",New_Time);
         Usr_Flash_Params.Wdog=New_Time;
         Save_Usr_Flash();
      }
      else
         UART_ETHprintf(tpcb,"wdog too short (<120 secs) %d secs\r\n",New_Time);
   }
   return 0;
}/*}}}*/
int Cmd_Link_State(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
   UART_ETHprintf(tpcb,"link %s\r\n",EMACPHYLinkUp()?"up":"down");
   return 0;
}/*}}}*/
int Cmd_Reboot(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
   Telnet_Close(tpcb);
   New_None_Periodic_Func_Schedule(20,Soft_Reset); //tengo que rebootear dede fuerea de Rcv_fn
   return 0;
}/*}}}*/
int Cmd_Show_Id(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
   UART_ETHprintf(tpcb,"%s\r\n", Usr_Flash_Params.Id);
   return 0;
}/*}}}*/
int Cmd_Id(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
   if(argc==1)
      UART_ETHprintf(tpcb,"%s\r\n", Usr_Flash_Params.Id);
   else {
      UART_ETHprintf(tpcb,"new id:%s\r\n",argv[1]);
      ustrncpy(Usr_Flash_Params.Id,argv[1],sizeof(Usr_Flash_Params.Id)-1);
      Save_Usr_Flash();
   }
   return 0;
}/*}}}*/
int Cmd_Pwd(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
   if(argc==1)
      UART_ETHprintf(tpcb,"%s\r\n", Usr_Flash_Params.Pwd);
   else {
      UART_ETHprintf(tpcb,"new pwd:%s \r\n",argv[1]);
      ustrncpy(Usr_Flash_Params.Pwd,argv[1],sizeof(Usr_Flash_Params.Pwd)-1);
      Save_Usr_Flash();
   }
   return 0;
}/*}}}*/
//-------------MOTOR--------------------------------------------------------------
int Cmd_Spi_Get_Param(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
   if(argc>1) {
      uint32_t Ans=0;
      switch (atoi(argv[1])) {
            case 1:
               Ans=Get_Reg1(atoi(argv[2]));
               break;
            case 2:
               Ans=Get_Reg2(atoi(argv[2]));
               break;
            case 3:
               Ans=Get_Reg3(atoi(argv[2]));
               break;
            default:
               break;
      }
      UART_ETHprintf(tpcb,"Reg: %d = %d \r\n",atoi(argv[2]),Ans);
   }
   return 0;
}/*}}}*/
int Cmd_Spi_Set_Param(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
   if(argc>1) 
      switch (atoi(argv[1])) {
            case 1:
               Set_Reg1(atoi(argv[2]),atoi(argv[3]));
               break;
            case 2:
               Set_Reg2(atoi(argv[2]),atoi(argv[3]));
               break;
            case 3:
               Set_Reg3(atoi(argv[2]),atoi(argv[3]));
               break;
            default:
               break;
      }
   return 0;
}/*}}}*/
int Cmd_Spi_Init(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
//   UART_ETHprintf(tpcb,"init\r\n");
   Init_Powerstep(tpcb);
   return 0;
}/*}}}*/
int Cmd_Spi_Run(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
   if(argc>1)
      Send_App3 (Run_Dir_Cmd, atoi(argv[1]), atoi(argv[2]));
   return 0;
}/*}}}*/
int Cmd_Spi_Step       ( struct tcp_pcb* tpcb, int argc, char *argv[] )
{/*{{{*/
   if(argc>1)
      Send_App0 (Step_Clk_Cmd, atoi(argv[1]));
   return 0;
}/*}}}*/
int Cmd_Spi_Move       ( struct tcp_pcb* tpcb, int argc, char *argv[] ) 
{/*{{{*/
   if(argc>1)
      Send_App3 (Move_Cmd, atoi(argv[1]), atoi(argv[2]));
   return 0;
}/*}}}*/
int Cmd_Spi_Goto       ( struct tcp_pcb* tpcb, int argc, char *argv[] ) 
{/*{{{*/
   if(argc>1)
      Send_App3 (Goto_Cmd, 0, atoi(argv[1]));
   return 0;
}/*}}}*/
int Cmd_Spi_Goto_Dir   ( struct tcp_pcb* tpcb, int argc, char *argv[] )
{/*{{{*/
   if(argc>1)
      Send_App3 (Goto_Dir_Cmd, atoi(argv[1]), atoi(argv[2]));
   return 0;
}/*}}}*/
int Cmd_Spi_Goto_Until ( struct tcp_pcb* tpcb, int argc, char *argv[] )
{/*{{{*/
   if(argc>1)
      Send_App3 (Go_Until_Cmd, atoi(argv[1]), atoi(argv[2]));
   return 0;
}/*}}}*/
int Cmd_Spi_Home       ( struct tcp_pcb* tpcb, int argc, char *argv[] )
{/*{{{*/
   Send_App0 (Go_Home_Cmd,0);
   return 0;
}/*}}}*/
int Cmd_Spi_Go_Mark    ( struct tcp_pcb* tpcb, int argc, char *argv[] )
{/*{{{*/
   Send_App0 (Go_Mark_Cmd, 0);
   return 0;
}/*}}}*/
int Cmd_Spi_Reset_Pos  ( struct tcp_pcb* tpcb, int argc, char *argv[] )
{/*{{{*/
   Send_App0 (Reset_Pos_Cmd, 0);
   return 0;
}/*}}}*/
int Cmd_Spi_Reset      ( struct tcp_pcb* tpcb, int argc, char *argv[] )
{/*{{{*/
   Send_App0 (Reset_Device_Cmd, 0);
   return 0;
}/*}}}*/
int Cmd_Spi_Soft_Stop  ( struct tcp_pcb* tpcb, int argc, char *argv[] )
{/*{{{*/
   Send_App0 (Soft_Stop_Cmd, 0);
   return 0;
}/*}}}*/
int Cmd_Spi_Hard_Stop  ( struct tcp_pcb* tpcb, int argc, char *argv[] )
{/*{{{*/
   Send_App0 (Hard_Stop_Cmd, 0);
   return 0;
}/*}}}*/
int Cmd_Spi_Soft_Hiz   ( struct tcp_pcb* tpcb, int argc, char *argv[] )
{/*{{{*/
   Send_App0 (Soft_Hi_Z_Cmd, 0);
   return 0;
}/*}}}*/
int Cmd_Spi_Hard_Hiz   ( struct tcp_pcb* tpcb, int argc, char *argv[] )
{/*{{{*/
   Send_App0 (Hard_Hi_Z_Cmd, 0);
   return 0;
}/*}}}*/
int Cmd_Spi_Status     ( struct tcp_pcb* tpcb, int argc, char *argv[] )
{/*{{{*/
   UART_ETHprintf(tpcb,"status= 0x%06x\r\n",Get_App3(Get_Status_Cmd));
   return 0;
}/*}}}*/
int Cmd_Toogle_Pulses(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
   if(argc>1) {
      uint32_t Pulses = atoi(argv[1]);
      UART_ETHprintf(DEBUG_MSG,"begin pulses:%d\r\n",Pulses);
      Toogle_Pulses(Pulses);
      UART_ETHprintf(DEBUG_MSG,"end pulses\r\n");
   }
   return 0;
}/*}}}*/
int Cmd_Speed     ( struct tcp_pcb* tpcb, int argc, char *argv[] )
{/*{{{*/
   float V=Get_Reg3 (Speed_Reg)/67.108864;
   UART_ETHprintf(tpcb,"step/seg= %f\r\n",V);
   return 0;
}/*}}}*/
int Cmd_Max_Speed     ( struct tcp_pcb* tpcb, int argc, char *argv[] )
{/*{{{*/
   if(argc>1) {
      float V=(atol(argv[1]))*0.065535;
      UART_ETHprintf(DEBUG_MSG,"step/seg= %f\r\n",V);
      Set_Reg2 (Max_Speed_Reg, (uint32_t)V);
   }
   else {
      float V=Get_Reg2 (Max_Speed_Reg)/0.065535;
      UART_ETHprintf(tpcb,"step/seg= %f\r\n",V);
   }
   return 0;
}/*}}}*/
int Cmd_Min_Speed     ( struct tcp_pcb* tpcb, int argc, char *argv[] )
{/*{{{*/
   if(argc>1) {
      float V=(atol(argv[1]))*4.194304;
      UART_ETHprintf(DEBUG_MSG,"step/seg= %f\r\n",V);
      Set_Reg2 (Min_Speed_Reg, (uint32_t)V);
   }
   else {
      float V=Get_Reg2 (Min_Speed_Reg)/4.194304;
      UART_ETHprintf(tpcb,"step/seg= %f\r\n",V);
   }
   return 0;
}/*}}}*/
int Cmd_Acc     ( struct tcp_pcb* tpcb, int argc, char *argv[] )
{/*{{{*/
   if(argc>1) {
      float V=(atol(argv[1]))*0.068719476736;
      UART_ETHprintf(DEBUG_MSG,"[step/seg]2= %f\r\n",V);
      Set_Reg2 (Acc_Reg, (uint32_t)V);
   }
   else {
      float V=Get_Reg2 (Acc_Reg)/0.068719476736;
      UART_ETHprintf(tpcb,"[step/seg]2= %f\r\n",V);
   }
   return 0;
}/*}}}*/
int Cmd_Dec     ( struct tcp_pcb* tpcb, int argc, char *argv[] )
{/*{{{*/
   if(argc>1) {
      float V=(atol(argv[1]))*0.068719476736;
      UART_ETHprintf(DEBUG_MSG,"[step/seg]2= %f\r\n",V);
      Set_Reg2 (Dec_Reg, (uint32_t)V);
   }
   else {
      float V=Get_Reg2 (Dec_Reg)/0.068719476736;
      UART_ETHprintf(tpcb,"[step/seg]2= %f\r\n",V);
   }
   return 0;
}/*}}}*/
int Cmd_Wait     ( struct tcp_pcb* tpcb, int argc, char *argv[] )
{/*{{{*/
   Set_Wait_Busy();
   return 0;
}/*}}}*/
int Cmd_Nowait     ( struct tcp_pcb* tpcb, int argc, char *argv[] )
{/*{{{*/
   Unset_Wait_Busy();
   return 0;
}/*}}}*/
//---------------ESP-----------------------------------------------------------
int Cmd_Pipe_Esp(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
   if(argc>1) {
      UART_ETHprintf(CONFIG_ESP_MSG,"%s\r\n",argv[1]);
   }
   return 0;
}/*}}}*/
int Cmd_Esp_Bind(struct tcp_pcb* tpcb, int argc, char *argv[])
{/*{{{*/
   if(argc>1) {
      UART_ETHprintf(CONFIG_ESP_MSG,"AT+CIPMUX=1\r\n");
      vTaskDelay(pdMS_TO_TICKS(1000));
      UART_ETHprintf(CONFIG_ESP_MSG,"AT+CIPSERVER=1,%d\r\n",atoi(argv[1]));
   }
   return 0;
}/*}}}*/
//-----------------CMD PROCESS--------------------------------------------------
char Buff_Cmd[APP_INPUT_BUF_SIZE];
void Init_Uart0(void)
{
   ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
   ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
   ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
   ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
//   UARTEchoSet(false);

   UARTStdioConfig ( 0, 115200, configCPU_CLOCK_HZ);
   g_psCmdTable=Motor_Cmd_Table;
}

void User_Commands_Task(void* nil)
{
   Cmd_Welcome(UART_MSG,0,NULL);
   Cmd_Help(UART_MSG,0,NULL);
   uint32_t Uart_Id=0;

   while(1) {
      while(xSemaphoreTake(Uart_Studio_Semphr,portMAX_DELAY)!=pdTRUE)
         ;
      {
         struct Gcode_Queue_Struct D;
         UARTgets ( (char*)D.Buff ,APP_INPUT_BUF_SIZE );
         D.tpcb=UART_MSG;
         D.Id=Uart_Id;
         Uart_Id++;
         while(xQueueSend(Gcode_Queue,&D,portMAX_DELAY)!=pdTRUE)
            ;
         Print_Slide(&D);
      }
   }
}
