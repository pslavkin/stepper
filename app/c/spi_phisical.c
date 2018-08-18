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
#include "spi_phisical.h"
#include "state_machine.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
//-------------------------------------------------------------
void Cs_Hi     ( void ) { GPIOPinSet   ( GPIO_PORTN_BASE ,GPIO_PIN_2 );}
void Cs_Lo     ( void ) { GPIOPinReset ( GPIO_PORTN_BASE ,GPIO_PIN_2 );}

void Rst_Hi    ( void ) { GPIOPinSet   ( GPIO_PORTN_BASE ,GPIO_PIN_3 );}
void Rst_Lo    ( void ) { GPIOPinReset ( GPIO_PORTN_BASE ,GPIO_PIN_3 );}

void Pulse_Hi    ( void ) { GPIOPinSet   ( GPIO_PORTM_BASE ,GPIO_PIN_3 );}
void Pulse_Lo    ( void ) { GPIOPinReset ( GPIO_PORTM_BASE ,GPIO_PIN_3 );}

bool Busy_Read ( void ) { return GPIOPinRead ( GPIO_PORTP_BASE ,GPIO_PIN_2 ) ;}

QueueHandle_t   Busy_Sem;
bool Wait_Busy=false;

void  Set_Wait_Busy     (void)
{
      Wait_Busy=true;
      UART_ETHprintf(UART_MSG,"wait true\r\n");
}
void  Unset_Wait_Busy   (void)
{
      Wait_Busy=false;
      UART_ETHprintf(UART_MSG,"wait false\r\n");
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

    xTaskCreate(Busy_Read_Task,"busy read",configMINIMAL_STACK_SIZE ,NULL ,1 ,NULL );
}
//-------------------------------------------------------------
void Send_Cmd2Spi(struct tcp_pcb* tpcb,uint8_t* Params,uint8_t Len)
{
   uint32_t Ans;
   uint8_t i;
   for(i=0;i<Len;i++) {
      Cs_Lo();
         MAP_SSIDataPut(SSI2_BASE,Params[i]);
         MAP_SSIDataGet(SSI2_BASE,&Ans);
         UART_ETHprintf(UART_MSG,"Command=0x%02x - Ans=0x%02x\r\n",Params[i],(uint8_t) Ans);
         Params[i]=(uint8_t)Ans;
         if(Wait_Busy==true && i==(Len-1)) {
            while(Busy_Read()==0)
               ;
         Wait_Busy=false;
         }
      Cs_Hi();
   }
}
void Send_Cmd2Spi4Int(struct tcp_pcb* tpcb,uint8_t Cmd, uint8_t P, uint32_t N, uint8_t Len)
{
   uint8_t Data[4]={P|Cmd,0};
   switch (Len) {
      case 1:
         Data[1]=(N & 0x0000FF)>> 0;
         break;
      case 2:
         Data[1]=(N & 0x00FF00)>> 8;
         Data[2]=(N & 0x0000FF)>> 0;
         break;
      case 3:
         Data[1]=(N & 0xFF0000)>>16;
         Data[2]=(N & 0x00FF00)>> 8;
         Data[3]=(N & 0x0000FF)>> 0;
         break;
      default:
         break;
   }
   Send_Cmd2Spi (tpcb,Data,Len+1);
}
void Init_Powerstep(struct tcp_pcb* tpcb)
{
   uint8_t p[]= { 9  ,150 ,10 ,150 ,11 ,150 ,12 ,150 , // compensacion de vss
                  26 ,44  ,8  ,                        // que no se apague el puente si salta overcurrente
                  5  ,0   ,10 ,6   ,0  ,10,           // aceleracion y descaeleracion
                  0x16  ,0x80};                       // full step
   Send_Cmd2Spi(tpcb,p,sizeof p);
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
      UART_ETHprintf(UART_MSG,"busy\n");
      while(Busy_Read()==0)
         vTaskDelay ( pdMS_TO_TICKS(10 ));
      xSemaphoreGive(Busy_Sem);
      UART_ETHprintf(UART_MSG,"ready\n");
   }
}
//----------------------------------------------------------------------------------------------------
