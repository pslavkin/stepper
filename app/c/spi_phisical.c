#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "inc/hw_memmap.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/rom_map.h"
#include "opt.h"
#include "commands.h"
#include "spi_phisical.h"
#include "state_machine.h"
#include "utils/lwiplib.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
//-------------------------------------------------------------
void Cs_Hi    ( void ) { GPIOPinSet   ( GPIO_PORTN_BASE ,GPIO_PIN_2 );}
void Cs_Lo    ( void ) { GPIOPinReset ( GPIO_PORTN_BASE ,GPIO_PIN_2 );}

void Rst_Hi   ( void ) { GPIOPinSet   ( GPIO_PORTN_BASE ,GPIO_PIN_3 );}
void Rst_Lo   ( void ) { GPIOPinReset ( GPIO_PORTN_BASE ,GPIO_PIN_3 );}

void Pulse_Hi ( void ) { GPIOPinSet   ( GPIO_PORTM_BASE ,GPIO_PIN_3 );}
void Pulse_Lo ( void ) { GPIOPinReset ( GPIO_PORTM_BASE ,GPIO_PIN_3 );}

bool Busy_Read ( void ) { return GPIOPinRead ( GPIO_PORTP_BASE ,GPIO_PIN_2 ) ;}

QueueHandle_t   Busy_Sem;
bool Wait_Busy=false;

void  Set_Wait_Busy     (void)
{
      Wait_Busy=true;
      UART_ETHprintf(DEBUG_MSG,"wait true\r\n");
}
void  Unset_Wait_Busy   (void)
{
      Wait_Busy=false;
      UART_ETHprintf(DEBUG_MSG,"wait false\r\n");
}
void  Init_Spi_Phisical (void)
{
    Busy_Sem = xSemaphoreCreateBinary();

    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);

    //MISO MOSI y CLK
    GPIOPinConfigure(GPIO_PD1_SSI2XDAT0);
    GPIOPinConfigure(GPIO_PD0_SSI2XDAT1);
    GPIOPinConfigure(GPIO_PD3_SSI2CLK);
    GPIOPinTypeSSI(GPIO_PORTD_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3 );
    SSIConfigSetExpClk(SSI2_BASE, configCPU_CLOCK_HZ, SSI_FRF_MOTO_MODE_3, SSI_MODE_MASTER, SSI_BIT_RATE, 8);
    SSIEnable(SSI2_BASE);
    //CS
    MAP_GPIOPinTypeGPIOOutput ( GPIO_PORTN_BASE,GPIO_PIN_2 );
    Cs_Hi();
    //reset
    MAP_GPIOPinTypeGPIOOutput ( GPIO_PORTN_BASE,GPIO_PIN_3 );
    Rst_Hi();
    //Busy 
    MAP_GPIOPinTypeGPIOInput ( GPIO_PORTP_BASE,GPIO_PIN_2 );

    //flag
    MAP_GPIOPinTypeGPIOInput ( GPIO_PORTH_BASE,GPIO_PIN_3 );

    //pulse
    MAP_GPIOPinTypeGPIOOutput ( GPIO_PORTM_BASE,GPIO_PIN_3 );
    Pulse_Hi();
}
//-------------------------------------------------------------
void Send_Cmd2Spi(struct tcp_pcb* tpcb, Spi_Params* Params)
{
   uint32_t Ans;
   uint8_t i,n;
   for(i=0;i<Params->Len;i++) {
      Cs_Lo();
         for(n=0;n<NUM_AXES;n++) {
            MAP_SSIDataPut(SSI2_BASE, Params->Data[n][i]);
            UART_ETHprintf(DEBUG_MSG,"Command=0x%02x -",Params->Data[n][i]);
            MAP_SSIDataGet(SSI2_BASE,&Ans);
            Params->Data[n][i]=(uint8_t)Ans;
            UART_ETHprintf(DEBUG_MSG,"Ans=0x%02x\r\n",Params->Data[n][i]);
            }
      if(Wait_Busy==true && i==(Params->Len-1)) {
         while(Busy_Read()==0)
            ;
         Wait_Busy=false;
      }
      Cs_Hi();
   }
}
//--------------------------------------------------------------------------------
void Get_Reg(uint8_t Reg, Spi_Params* Ans, uint8_t Len)
{
   Ans->Len=Len;
   Cmd2Params   ( Ans       ,Reg|Get_Param_Cmd );
   Send_Cmd2Spi ( DEBUG_MSG ,Ans               );
}
void Get_Reg1 ( uint8_t Reg, Spi_Params* Ans ) { Get_Reg(Reg,Ans,2); }
void Get_Reg2 ( uint8_t Reg, Spi_Params* Ans ) { Get_Reg(Reg,Ans,3); }
void Get_Reg3 ( uint8_t Reg, Spi_Params* Ans ) { Get_Reg(Reg,Ans,4); }
//--------------------------------------------------------------------------------
void Set_Reg(uint8_t Reg, uint32_t V,uint8_t Len)
{
   Spi_Params Params;
   uint32_t V_Array[NUM_AXES];
   uint8_t i;
   for(i=0;i<NUM_AXES;i++)
      V_Array[i]=V;
   Params.Len=Len;
   Cmd2Params   ( &Params,Reg|Set_Param_Cmd );
   Value2Params ( &Params,V_Array           );
   Send_Cmd2Spi ( DEBUG_MSG,&Params         );
}
void Set_Reg1 ( uint8_t Reg, uint8_t  V ) { Set_Reg(Reg,V,2);}
void Set_Reg2 ( uint8_t Reg, uint16_t V ) { Set_Reg(Reg,V,3);}
void Set_Reg3 ( uint8_t Reg, uint32_t V ) { Set_Reg(Reg,V,4);}
//--------------------------------------------------------------------------------
void Get_App(uint8_t Cmd, Spi_Params* Ans, uint8_t Len)
{
   Ans->Len=Len;
   Cmd2Params   ( Ans,Cmd);
   Send_Cmd2Spi ( DEBUG_MSG,Ans );
}
void Get_App3(uint8_t Cmd, Spi_Params* Ans) { Get_App(Cmd,Ans,4); }
//--------------------------------------------------------------------------------
void Send_App(uint8_t Cmd, uint8_t Option,uint32_t *V,uint8_t Len)
{
   Spi_Params Params;
   Params.Len=Len;
   Cmd2Params   ( &Params,Cmd|Option );
   Value2Params ( &Params,V          );
   Send_Cmd2Spi ( DEBUG_MSG,&Params  );
}
void Send_App_Equal(uint8_t Cmd, uint8_t Option,uint32_t V,uint8_t Len)
{
   uint32_t V_Array[NUM_AXES];
   uint8_t i;
   for(i=0;i<NUM_AXES;i++)
      V_Array[i]=V;
   Send_App(Cmd,Option,V_Array,Len);
}
void Send_App0 ( uint8_t Cmd, uint8_t Option             ) { Send_App_Equal(Cmd,Option,0,1);}
void Send_App1 ( uint8_t Cmd, uint8_t Option, uint8_t  V ) { Send_App_Equal(Cmd,Option,V,2);}
void Send_App2 ( uint8_t Cmd, uint8_t Option, uint16_t V ) { Send_App_Equal(Cmd,Option,V,3);}
void Send_App3 ( uint8_t Cmd, uint8_t Option, uint32_t V ) { Send_App_Equal(Cmd,Option,V,4);}
//--------------------------------------------------------------------------------



void Init_Powerstep(struct tcp_pcb* tpcb)
{
   Set_Reg1( 9,150);
   Set_Reg1(10,150);
   Set_Reg1(11,150);
   Set_Reg1(12,150);
   Set_Reg2(Config_Reg,0x2C08);
   Set_Reg2(Dec_Reg,0x000A);
   Set_Reg2(Acc_Reg,0x000A);
}
void Toogle_Pulses(uint32_t Pulses)
{
   uint32_t i;
   for ( i=0;i<Pulses;i++ ) {
      Pulse_Lo();
      Delay_Useg(2000);
      Pulse_Hi();
      Delay_Useg(2000);  //uso esto porque no anda vtaskdelay para tiempos cortos.. tengo el tick en 100hz
   }
}
void Busy_Read_Task(void* nil)
{
   while(1) {
      while(Busy_Read()==1)
         vTaskDelay ( pdMS_TO_TICKS(10 ));
      UART_ETHprintf(DEBUG_MSG,"busy\n");
      while(Busy_Read()==0)
         vTaskDelay ( pdMS_TO_TICKS(10 ));
      xSemaphoreGive(Busy_Sem);
      UART_ETHprintf(DEBUG_MSG,"ready\n");
   }
}
//----------------------------------------------------------------------------------------------------
