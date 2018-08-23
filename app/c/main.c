#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "driverlib/flash.h"
#include "utils/uartstdio.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/pin_map.h"
#include "commands.h"
#include "clk.h"
#include "state_machine.h"
#include "events.h"
#include "wdog.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "leds_session.h"
#include "commands.h"
#include "schedule.h"
#include "telnet.h"
#include "spi_phisical.h"
#include "usr_flash.h"
#include "gcode.h"
#include "esp8266.h"


int main(void)
{
   Init_Clk          ( );
   Init_Usr_Flash    ( );
   Init_Led_Eth_Data ( );
   lwIPInit(configCPU_CLOCK_HZ,
            Usr_Flash_Params.Mac_Addr,
            Usr_Flash_Params.Ip_Addr,
            Usr_Flash_Params.Mask_Addr,
            Usr_Flash_Params.Gateway_Addr,
            IPADDR_USE_STATIC);
   Init_Wdog   ( );
   Init_Uart0  ( );
   Init_Uart7  ( );
   Init_Events ( );
   Init_Schedule();
   xTaskCreate ( State_Machine      ,"sm"            ,configMINIMAL_STACK_SIZE ,NULL ,tskIDLE_PRIORITY+1 ,NULL );
   xTaskCreate ( Schedule           ,"schedule"      ,configMINIMAL_STACK_SIZE ,NULL ,tskIDLE_PRIORITY+1 ,NULL );
   xTaskCreate ( Led_Link_Task      ,"led link"      ,configMINIMAL_STACK_SIZE ,NULL ,tskIDLE_PRIORITY+1 ,NULL );
   xTaskCreate ( Led_Eth_Data_Task  ,"led eth data"  ,configMINIMAL_STACK_SIZE ,NULL ,tskIDLE_PRIORITY+1 ,NULL );
   xTaskCreate ( User_Commands_Task ,"user commands" ,configMINIMAL_STACK_SIZE ,NULL ,tskIDLE_PRIORITY+1 ,NULL );
   xTaskCreate ( Gcode_Parser       ,"gcode"         ,configMINIMAL_STACK_SIZE ,NULL ,tskIDLE_PRIORITY+1 ,NULL );
   xTaskCreate ( Esp_Task           ,"esp"           ,configMINIMAL_STACK_SIZE ,NULL ,tskIDLE_PRIORITY+1 ,NULL );
//    xTaskCreate(Busy_Read_Task,"busy read",configMINIMAL_STACK_SIZE ,NULL ,1 ,NULL );
   Init_Telnet         ( );
   Init_Spi_Phisical   ( );
   vTaskStartScheduler ( );
   while(1)
       ;
}

// The error routine that is called if the driver library encounters an error.
#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line) { }
#endif
// Required by lwIP library to support any host-related timer functions.
void lwIPHostTimerHandler(void) { }
