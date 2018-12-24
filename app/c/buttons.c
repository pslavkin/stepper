#include <stdbool.h>
#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/emac.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "utils/uartstdio.h"
#include "leds_session.h"
#include "buttons.h"

//-------------------------------------------------------------------
SemaphoreHandle_t Busy_Semphr;
bool Busy_Flag=1;

void Init_Busy(void)
{
   Busy_Semphr = xSemaphoreCreateBinary     ( );
   MAP_SysCtlPeripheralEnable ( BUSY_PERIPH                                                   );
   MAP_GPIOPinTypeGPIOInput   ( BUSY_PORT ,BUSY_PIN                                           );
   MAP_GPIOPadConfigSet       ( BUSY_PORT ,BUSY_PIN ,GPIO_STRENGTH_2MA ,GPIO_PIN_TYPE_STD_WPU );

   MAP_GPIOIntTypeSet         ( BUSY_PORT ,BUSY_PIN ,GPIO_RISING_EDGE);
   MAP_GPIOIntClear           ( BUSY_PORT ,BUSY_PIN                                           );
   MAP_GPIOIntEnable          ( BUSY_PORT ,BUSY_PIN                                           );
   MAP_IntEnable              ( BUSY_INT                                                      );
}

void BusyIntHandler(void)
{
   BaseType_t Context_Change=pdFALSE;
   MAP_GPIOIntClear      ( BUSY_PORT, BUSY_PIN         );
   xSemaphoreGiveFromISR ( Busy_Semphr,&Context_Change );
//   Busy_Flag=0;
   portYIELD_FROM_ISR    ( Context_Change              );
}

void Print_Busy_Flag( void )
{
   UART_ETHprintf(UART_MSG,"busy flag %d\n",Busy_Flag);
}
void Reset_Busy_Flag( void )
{
   Busy_Flag=1;
}
bool Busy_Read ( void ) { return GPIOPinRead ( BUSY_PORT, BUSY_PIN) ;}
//------------------------------------------------------------------
