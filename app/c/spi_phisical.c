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

}
//-------------------------------------------------------------
void Send_Cmd2Spi(uint8_t* Params,uint8_t Len)
{
   uint32_t Ans;
   uint8_t i;

   for(i=0;i<Len;i++) {
      Cs_Lo();
      MAP_SSIDataPut(SSI2_BASE,Params[i]);
      MAP_SSIDataGet(SSI2_BASE,&Ans);
      UART_ETHprintf(NULL,"Command=0x%x - Ans=0x%x\r\n",Params[i],(uint8_t) Ans);
      Params[i]=(uint8_t)Ans;
      Cs_Hi();
   }
}
//----------------------------------------------------------------------------------------------------

