#include <stdio.h>
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
#include "third_party/lwip-1.4.1/src/include/ipv4/lwip/ip_addr.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


//*****************************************************************************
//
// This is the table that holds the command names, implementing functions, and
// brief description.
//
//*****************************************************************************
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
    { "net"    ,Cmd_Main2Ip     ,": network options" }     ,
    { "motor"  ,Cmd_Main2Motor  ,": motor driver" } ,
    { "system" ,Cmd_Main2System ,": system options" }      ,
    { "?"      ,Cmd_Help        ,": help" }                ,
    { "<"      ,Cmd_Back2Login  ,": back" }                ,
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
    { "power" ,Cmd_Powerstep_Init_Cmd ,": Inicializa los registros del powerstep" }     ,
    { "sa"    ,Cmd_Spi_App_Cmd        ,": Send app comand x spi ej: sp 208 0 0 0" }     ,
    { "gp"    ,Cmd_Spi_Get_Param_Cmd  ,": Get param comand x spi ej: sp 5" }            ,
    { "pulse" ,Cmd_Toogle_Pulses_Cmd  ,": Toogle pulses con direccino ej.pulse 100 1" } ,
    { "?"     ,Cmd_Help               ,": help" }                                       ,
    { "<"     ,Cmd_Back2Main          ,": back" }                                       ,
    { 0       ,0                      ,0 }
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
//--------------------------------------------------------------------------------
int Cmd_Welcome(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   g_psCmdTable=Login_Cmd_Table;
   UART_ETHprintf(tpcb,"\033[2J\033[H+++ PaP V0.1 +++\r\ntp final protocolos - cese2018\r\n");
   return 0;
}
int Cmd_Help(struct tcp_pcb* tpcb, int argc, char *argv[])
{
    tCmdLineEntry *pEntry;
    UART_ETHprintf(tpcb,"\r\navailable commands\r\n------------------\r\n");
    pEntry = g_psCmdTable;
    for(;pEntry->pcCmd;pEntry++)
        UART_ETHprintf(tpcb,"%15s%s\r\n", pEntry->pcCmd, pEntry->pcHelp);
    return 0;
}
//--------------------------------------------------------------------------------
int Cmd_Login(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   if(argc>1) {
      if(ustrcmp(argv[1],Usr_Flash_Params.Pwd)==0) {
         UART_ETHprintf(tpcb,"success\r\n");
         g_psCmdTable=Main_Cmd_Table;
         Cmd_Help(tpcb,argc,argv);
      }
      else
         UART_ETHprintf(tpcb,"invalid\r\n");
   }
   else
      UART_ETHprintf(tpcb,"no pwd\r\n");
   return 0;
}
int Cmd_Exit(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   Telnet_Close(tpcb);
   return 0;
}
int Cmd_Main2Ip(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   g_psCmdTable=Ip_Cmd_Table;
   return 0;
}
int Cmd_Main2Motor(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   g_psCmdTable=Motor_Cmd_Table;
   return 0;
}
int Cmd_Main2System(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   g_psCmdTable=System_Cmd_Table;
   return 0;
}
int Cmd_Back2Main(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   g_psCmdTable=Main_Cmd_Table;
   return 0;
}
int Cmd_Back2Login(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   g_psCmdTable=Login_Cmd_Table;
   return 0;
}
//----------------------------------------------------------------------------------------------------
int Cmd_Mac(struct tcp_pcb* tpcb, int argc, char *argv[])
{
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
}
int Cmd_Ip(struct tcp_pcb* tpcb, int argc, char *argv[])
{
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
}
int Cmd_Mask(struct tcp_pcb* tpcb, int argc, char *argv[])
{
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
}
int Cmd_Gateway(struct tcp_pcb* tpcb, int argc, char *argv[])
{
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
}
int Cmd_Dhcp(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   if(argc==1)
      UART_ETHprintf(tpcb,"%d\r\n",Usr_Flash_Params.Dhcp_Enable);
   else {
      uint16_t New=atoi(argv[1]);
      UART_ETHprintf(tpcb,"new dhcp=%d \r\n",New);
      Usr_Flash_Params.Dhcp_Enable=New;
      Save_Usr_Flash();
   }
   return 0;
}
int Cmd_Config_Port(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   if(argc==1)
      UART_ETHprintf(tpcb,"%d\r\n",Usr_Flash_Params.Config_Port);
   else {
      uint16_t New_Port=atoi(argv[1]);
      UART_ETHprintf(tpcb,"new port=%d\r\n",New_Port);
      Usr_Flash_Params.Config_Port=New_Port;
      Save_Usr_Flash();
   }
   return 0;
}
void DisplayIPAddress(struct tcp_pcb* tpcb,uint32_t ui32Addr)
{
    UART_ETHprintf(tpcb, "%d.%d.%d.%d\r\n", ui32Addr & 0xff, (ui32Addr >> 8) & 0xff,
            (ui32Addr >> 16) & 0xff, (ui32Addr >> 24) & 0xff);
}

int Cmd_TaskList(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   if(argc==2 && ustrcmp("tareas",argv[1])==0) {
      char* Buff=(char*)pvPortMalloc(UART_TX_BUFFER_SIZE);
      vTaskList( Buff );
      UART_ETHprintf(tpcb,Buff);
      vPortFree(Buff);
   }
   return 0;
}
int Cmd_Uptime(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   UART_ETHprintf(tpcb,"%d\r\n",Read_Uptime());
   return 0;
}
int Cmd_Wdog_Tout(struct tcp_pcb* tpcb, int argc, char *argv[])
{
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
}

int Cmd_Link_State(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   UART_ETHprintf(tpcb,"link %s\r\n",EMACPHYLinkUp()?"up":"down");
   return 0;
}
int Cmd_Reboot(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   Telnet_Close(tpcb);
   New_None_Periodic_Func_Schedule(20,Soft_Reset); //tengo que rebootear dede fuerea de Rcv_fn
   return 0;
}
int Cmd_Show_Id(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   UART_ETHprintf(tpcb,"%s\r\n", Usr_Flash_Params.Id);
   return 0;
}
int Cmd_Id(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   if(argc==1)
      UART_ETHprintf(tpcb,"%s\r\n", Usr_Flash_Params.Id);
   else {
      UART_ETHprintf(tpcb,"new id:%s\r\n",argv[1]);
      ustrncpy(Usr_Flash_Params.Id,argv[1],sizeof(Usr_Flash_Params.Id)-1);
      Save_Usr_Flash();
   }
   return 0;
}
int Cmd_Pwd(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   if(argc==1)
      UART_ETHprintf(tpcb,"%s\r\n", Usr_Flash_Params.Pwd);
   else {
      UART_ETHprintf(tpcb,"new pwd:%s \r\n",argv[1]);
      ustrncpy(Usr_Flash_Params.Pwd,argv[1],sizeof(Usr_Flash_Params.Pwd)-1);
      Save_Usr_Flash();
   }
   return 0;
}
//--------------------------------------------------------------------------------
char Buff_Cmd[APP_INPUT_BUF_SIZE];
void Init_Uart(void)
{
   ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
   ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
   ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
   ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

   UARTStdioConfig ( 0, 115200, configCPU_CLOCK_HZ);
   g_psCmdTable=Login_Cmd_Table;
}

void User_Commands_Task(void* nil)
{
   Cmd_Welcome(UART_MSG,0,NULL);
   Cmd_Help(UART_MSG,0,NULL);
   while(1) {
      while(xSemaphoreTake(Uart_Studio_Semphr,portMAX_DELAY)!=pdTRUE)
         ;
      UARTgets       ( Buff_Cmd,APP_INPUT_BUF_SIZE );
      CmdLineProcess ( Buff_Cmd,UART_MSG            );
   }
}

int Cmd_Spi_App_Cmd(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   uint8_t i,param[4];
   argc--;
   for(i=0;i<argc && i<sizeof(param);i++) {
      param[i]=atoi(argv[i+1]);
   }
   Send_Cmd2Spi (tpcb, param,i );
   return 0;
}
int Cmd_Spi_Get_Param_Cmd(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   uint8_t param[4]={0};
   param[0]=atoi(argv[1]) | 0x20;
   Send_Cmd2Spi (tpcb, param,4 );
   return 0;
}
int Cmd_Powerstep_Init_Cmd(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   UART_ETHprintf(tpcb,"Inicializacion de registros\n");
   Init_Powerstep_Regs(tpcb);
   return 0;
}
int Cmd_Toogle_Pulses_Cmd(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   uint32_t Pulses=atoi(argv[1]);
   bool Dir=atoi(argv[2]);
   UART_ETHprintf(tpcb,"inicio pulsos%d\n",Pulses);
   Toogle_Pulse(Pulses,Dir);
   UART_ETHprintf(tpcb,"Fin de pulses\n");

   return 0;
}

////*****************************************************************************
////
//// Input buffer for the command line interpreter.
////
////*****************************************************************************
////*****************************************************************************
////
//// Prompts the user for a command, and blocks while waiting for the user's
//// input. This function will return after the execution of a single command.
////
////*****************************************************************************
//char Buff[APP_INPUT_BUF_SIZE];
//void Init_Uart(void)
//{
//   ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
//   ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
//   ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
//   ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
//
//   UARTStdioConfig ( 0, 115200, configCPU_CLOCK_HZ);
//}
//
//void User_Commands_Task(void* nil)
//{
//   Cmd_Welcome(UART_MSG,0,NULL);
//   Cmd_Help(UART_MSG,0,NULL);
//   while(1) {
//      while(xSemaphoreTake(Uart_Studio_Semphr,portMAX_DELAY)!=pdTRUE)
//         ;
//      UARTgets       ( Buff,APP_INPUT_BUF_SIZE );
//      CmdLineProcess ( Buff,UART_MSG            );
//   }
//}
////void User_Commands_Task(void* nil)
////{
////   char* Buff =(char*)pvPortMalloc(APP_INPUT_BUF_SIZE);
////   int len;
////   while(1) {
////      if(UARTPeek('\r') != -1) {
////         while((len=UARTPeek('\r')) != -1) {
////            if(len!=0)
////               UARTgets       ( Buff, APP_INPUT_BUF_SIZE );
////            else
////               UARTFlushRx();
////            CmdLineProcess ( Buff,NULL);
////         }
////      }
////      vTaskDelay( 100 / portTICK_RATE_MS );
////   }
////  vPortFree(Buff);
////
////}
