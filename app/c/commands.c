//*****************************************************************************
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "driverlib/sysctl.h"
#include "driverlib/emac.h"
#include "utils/cmdline.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "utils/lwiplib.h"
#include "commands.h"
#include "opt.h"
#include "spi_phisical.h"

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
tCmdLineEntry g_psCmdTable[] =
{
    { "help" ,Cmd_help              ,": Display list of commands" }               ,
    { "h"    ,Cmd_help              ,": alias for help" }                         ,
    { "?"    ,Cmd_help              ,": alias for help" }                         ,
    { "Mac"  ,Cmd_Mac               ,": show MAC address" }                       ,
    { "ip"   ,Cmd_Ip                ,": show IP address" }                        ,
    { "task" ,Cmd_TaskList          ,": lista de tareas" }                        ,
    { "link" ,Cmd_Links_State       ,": Estado del link ethernet" }               ,
    { "sa"   ,Cmd_Spi_App_Cmd       ,": Send app comand x spi ej: sp 208 0 0 0" } ,
    { "gp"   ,Cmd_Spi_Get_Param_Cmd ,": Get param comand x spi ej: sp 5" }        ,
    { 0      ,0               ,0 }
};

//*****************************************************************************
//
// This function implements the "help" command.  It prints a simple list of the
// available commands with a brief description.
//
//*****************************************************************************
int Cmd_help(struct tcp_pcb* tpcb, int argc, char *argv[])
{
    tCmdLineEntry *pEntry;
    UART_ETHprintf(tpcb,"\nAvailable commands\n------------------\n");
    pEntry = g_psCmdTable;
    for(;pEntry->pcCmd;pEntry++)
        UART_ETHprintf(tpcb,"%15s%s\n", pEntry->pcCmd, pEntry->pcHelp);
    return 0;
}
//----------------------------------------------------------------------------------------------------
int Cmd_Mac(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   UART_ETHprintf(tpcb,"parametros %d\n",argc);
   UpdateMACAddr(tpcb);
   return(0);
}
int Cmd_Ip(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   DisplayIPAddress(tpcb,lwIPLocalIPAddrGet());
   return(0);
}

void UpdateMACAddr(struct tcp_pcb* tpcb)
{
    uint8_t pui8MACAddr[6]={1,2,3,4,5,6};
    UART_ETHprintf(tpcb,"MAC: %02x:%02x:%02x:%02x:%02x:%02x",
            pui8MACAddr[0], pui8MACAddr[1], pui8MACAddr[2], pui8MACAddr[3],
            pui8MACAddr[4], pui8MACAddr[5]);
}
void DisplayIPAddress(struct tcp_pcb* tpcb,uint32_t ui32Addr)
{
    UART_ETHprintf(tpcb, "%d.%d.%d.%d", ui32Addr & 0xff, (ui32Addr >> 8) & 0xff,
            (ui32Addr >> 16) & 0xff, (ui32Addr >> 24) & 0xff);
}

int Cmd_TaskList(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   char* Buff=(char*)pvPortMalloc(UART_TX_BUFFER_SIZE);
   vTaskList( Buff );
   UART_ETHprintf(tpcb,Buff);
   vPortFree(Buff);
   return 0;
}

int Cmd_Links_State(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   UART_ETHprintf(tpcb,"PHY=%d",EMACPHYLinkUp());
   return 0;
}
int Cmd_Spi_App_Cmd(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   uint8_t i,param[4];
   argc--;
   for(i=0;i<argc && i<sizeof(param);i++) {
      param[i]=atoi(argv[i+1]);
   }
   Send_Cmd2Spi ( param,i );
   return 0;
}
int Cmd_Spi_Get_Param_Cmd(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   uint8_t param[4]={0};
   param[0]=atoi(argv[1]) | 0x20;
   Send_Cmd2Spi ( param,4 );
   return 0;
}
//*****************************************************************************
//
// Input buffer for the command line interpreter.
//
//*****************************************************************************
//*****************************************************************************
//
// Prompts the user for a command, and blocks while waiting for the user's
// input. This function will return after the execution of a single command.
//
//*****************************************************************************
void User_Commands_Task(void* nil)
{
   char* Buff =(char*)pvPortMalloc(APP_INPUT_BUF_SIZE);
   int len;
   while(1) {
      if(UARTPeek('\r') != -1) {
         while((len=UARTPeek('\r')) != -1) {
            if(len!=0)
               UARTgets       ( Buff, APP_INPUT_BUF_SIZE );
            else
               UARTFlushRx();
            CmdLineProcess ( Buff,NULL);
         }
      }
      vTaskDelay( 100 / portTICK_RATE_MS );
   }
  vPortFree(Buff);

}
