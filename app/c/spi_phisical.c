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

void  Init_Spi_Phisical (void)
{

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
      UART_ETHprintf(tpcb,"Command=0x%02x - Ans=0x%02x\r\n",Params[i],(uint8_t) Ans);
      Params[i]=(uint8_t)Ans;
      Cs_Hi();
   }
}

void Init_Powerstep_Regs(struct tcp_pcb* tpcb)
{
   uint8_t p[]= { 9  ,150 ,10 ,150 ,11 ,150 ,12 ,150 , // compensacion de vss
                  26 ,44  ,8  ,                        // que no se apague el puente si salta overcurrente
                  5  ,0   ,10 ,6   ,0  ,10};           // aceleracion y descaeleracion
   Send_Cmd2Spi(tpcb,p,sizeof p);
}
void Toogle_Pulse(uint32_t Pulses, bool Dir)
{
   uint32_t i;
   for ( i=0;i<Pulses;i++ ) {
      Pulse_Lo();
      vTaskDelay ( pdMS_TO_TICKS(1 ));
      Pulse_Hi();
      vTaskDelay ( pdMS_TO_TICKS(1 ));
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
      UART_ETHprintf(UART_MSG,"ready\n");
   }
}
//----------------------------------------------------------------------------------------------------
