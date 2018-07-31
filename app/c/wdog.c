#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "utils/uartstdio.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "wdog.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

void Wdog_Handler ( void )
{
//   MAP_WatchdogIntClear(WATCHDOG0_BASE);
   MAP_UARTCharPut(UART0_BASE, 'w');
   MAP_UARTCharPut(UART0_BASE, 'd');
   MAP_UARTCharPut(UART0_BASE, 'o');
   MAP_UARTCharPut(UART0_BASE, 'g');
   MAP_UARTCharPut(UART0_BASE, ' ');
}

void Wdog_Task(void* nil)
{
   while(1) {
      vTaskDelay(pdMS_TO_TICKS(500));
      MAP_WatchdogReloadSet(WATCHDOG0_BASE, configCPU_CLOCK_HZ);
   }
}
void Init_Wdog(void)
{
   MAP_SysCtlPeripheralEnable ( SYSCTL_PERIPH_WDOG0                );
   MAP_IntEnable              ( INT_WATCHDOG                       );
   MAP_WatchdogIntEnable      ( WATCHDOG0_BASE                     ); // Enable the watchdog interrupt.
   MAP_WatchdogReloadSet      ( WATCHDOG0_BASE, configCPU_CLOCK_HZ ); // Set the period of the watchdog timer to 1 second.
   MAP_WatchdogResetEnable    ( WATCHDOG0_BASE                     ); // Enable reset generation from the watchdog timer.
   MAP_WatchdogEnable         ( WATCHDOG0_BASE                     ); // Enable the watchdog timer.
   xTaskCreate ( Wdog_Task ,"wdog" ,configMINIMAL_STACK_SIZE ,NULL ,1 ,NULL );
}


