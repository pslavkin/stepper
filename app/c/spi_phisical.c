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
#include <stdlib.h>
#include <string.h>

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

SemaphoreHandle_t Busy_Sem;
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
    Busy_Sem = xSemaphoreCreateMutex();

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
void Send_Cmd2Spi(Spi_Params* Params)
{
   uint32_t Ans;
   uint8_t i;
   int8_t n;
   xSemaphoreTake(Busy_Sem,portMAX_DELAY);
   for(i=0;i<=Params->Len;i++) {
      Cs_Lo();
         for(n=NUM_AXES-1;n>=0;n--) {
            MAP_SSIDataPut(SSI2_BASE, Params->Data[n][i]);
            MAP_SSIDataGet(SSI2_BASE,&Ans);
            Params->Data[n][i]=(uint8_t)Ans;
         }
      Cs_Hi();
      Delay_0_25Useg(1); //hay que esperar 650nseg antes de bajar de nuevo el CS.. lo medi y esta oka con este delay...
   }
   xSemaphoreGive(Busy_Sem);
}
//--------------------------------------------------------------------------------
void Get_Data(uint8_t Reg, uint8_t Option,uint32_t* Ans, uint8_t Len)
{
   Spi_Params Params;
   uint8_t i,j;
   for(i=0;i<NUM_AXES;i++)
      for(j=0;j<4;j++)
         Params.Data[i][j]=0;
   Params.Len=Len;
   Cmd2Params   ( &Params   ,Reg|Option );
   Send_Cmd2Spi ( &Params               );
   Params2Value ( &Params   ,Ans        );
}
void Send_Data(uint8_t Cmd, uint8_t* Option,uint32_t *V,uint8_t Len)
{
   Spi_Params Params;
   Params.Len=Len;
   Cmd2Params_Individual ( &Params,Cmd,Option );
   Value2Params          ( &Params,V          );
   Send_Cmd2Spi          ( &Params            );
}
//--------------------------------------------------------------------------------
void Get_Reg(uint8_t Reg, uint32_t* Ans, uint8_t Len)
{
   Get_Data(Reg,Get_Param_Cmd,Ans,Len);
}
void Set_Reg( uint8_t Reg, uint32_t* V , uint8_t Len)
{
   uint8_t  Option_Array[ NUM_AXES ];
   uint8_t i;
   for(i=0;i<NUM_AXES;i++)
      Option_Array[i]=Set_Param_Cmd;
   Send_Data(Reg,Option_Array,V,Len);
}
void Get_App(uint8_t Cmd, uint32_t* Ans, uint8_t Len)
{
   Get_Data(Cmd,0,Ans,Len);
}
//--------------------------------------------------------------------------------
void Get_Reg4Args( char** argv, uint32_t* Ans)
{
   Get_Reg(atoi(argv[1]),Ans,atoi(argv[2]));
}
void Set_Reg4Args ( char** argv)
{
   uint32_t V[ NUM_AXES ];
   uint8_t i;
   for(i=0;i<NUM_AXES;i++)
      V[ i ] = atoi ( argv[ 3+i ] );
   Set_Reg(atoi(argv[1]),V,atoi(argv[2]));
}
//--------------------------------------------------------------------------------
void Set_Reg_Equal(uint8_t Reg, uint32_t V,uint8_t Len)
{
   uint32_t V_Array[NUM_AXES];
   uint8_t i;
   for(i=0;i<NUM_AXES;i++)
      V_Array[i]=V;
   Set_Reg(Reg,V_Array,Len);
}
//--------------------------------------------------------------------------------
void Send_App_Equal(uint8_t Cmd, uint8_t Option,uint32_t V,uint8_t Len)
{
   uint32_t V_Array     [ NUM_AXES ];
   uint8_t  Option_Array[ NUM_AXES ];
   uint8_t i;
   for(i=0;i<NUM_AXES;i++) {
      Option_Array[i]=Option;
      V_Array[i]=V;
   }
   Send_Data(Cmd,Option_Array,V_Array,Len);
}
void Send_App4Args_Option ( uint8_t Cmd, char *argv[] , uint8_t Len)
{
   uint8_t  Options[ NUM_AXES ];
   uint32_t V      [ NUM_AXES ];
   uint8_t i;
   for(i=0;i<NUM_AXES;i++) {
      Options[ i ] = atoi ( argv[ 1+2*i ]);
      V      [ i ] = atoi ( argv[ 2+2*i ]);
   }
   Send_Data(Cmd,Options,V,Len);
}
void Send_App4Args ( uint8_t Cmd, char *argv[], uint8_t Len)
{
   uint8_t  Options[ NUM_AXES ];
   uint32_t V      [ NUM_AXES ];
   uint8_t i;
   for(i=0;i<NUM_AXES;i++) {
      Options[ i ] = 0;
      V      [ i ] = atoi ( argv[ 1+i ]);
   }
   Send_Data(Cmd,Options,V,Len);
}
//--------------------------------------------------------------------------------
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
      UART_ETHprintf(DEBUG_MSG,"ready\n");
   }
}
//----------------------------------------------------------------------------------------------------
