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
#include "udp.h"
#include "spi_phisical.h"


// The error routine that is called if the driver library encounters an error.
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif


// Required by lwIP library to support any host-related timer functions.
void lwIPHostTimerHandler(void)
{
}

int main(void)
{
    uint32_t ui32User0, ui32User1;
    uint8_t pui8MACArray[8];

   Init_Clk();

    //
    // Configure the device pins.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTStdioConfig ( 0, 115200, configCPU_CLOCK_HZ);
    UART_ETHprintf ( NULL,"\033[2J\033[H" );
    UART_ETHprintf ( NULL,"Ethergate\n\n" );

    //
    // Configure the hardware MAC address for Ethernet Controller filtering of
    // incoming packets.  The MAC address will be stored in the non-volatile
    // USER0 and USER1 registers.
    //
    MAP_FlashUserGet(&ui32User0, &ui32User1);
    ui32User0=0x786000;
    ui32User1=0xb5e406;
    //
    //
    // Convert the 24/24 split MAC address from NV ram into a 32/16 split
    // MAC address needed to program the hardware registers, then program
    // the MAC address into the Ethernet Controller registers.
    //
    pui8MACArray[0] = ((ui32User0 >>  0) & 0xff);
    pui8MACArray[1] = ((ui32User0 >>  8) & 0xff);
    pui8MACArray[2] = ((ui32User0 >> 16) & 0xff);
    pui8MACArray[3] = ((ui32User1 >>  0) & 0xff);
    pui8MACArray[4] = ((ui32User1 >>  8) & 0xff);
    pui8MACArray[5] = ((ui32User1 >> 16) & 0xff);


   lwIPInit(configCPU_CLOCK_HZ, pui8MACArray,0xC0A8020A, 0xFFFFFF00,0xC0A80201, IPADDR_USE_STATIC);


   Init_Wdog();
   Init_Leds   ( );
   Init_Events ( );
   xTaskCreate ( State_Machine      ,"sm"            ,configMINIMAL_STACK_SIZE ,NULL ,2 ,NULL );
   xTaskCreate ( Schedule           ,"schedule"      ,configMINIMAL_STACK_SIZE ,NULL ,1 ,NULL );
   xTaskCreate ( Led_Link_Task      ,"led link"      ,configMINIMAL_STACK_SIZE ,NULL ,1 ,NULL );
   xTaskCreate ( Led_Eth_Data_Task  ,"led eth data"  ,configMINIMAL_STACK_SIZE ,NULL ,1 ,NULL );
   xTaskCreate ( User_Commands_Task ,"user commands" ,configMINIMAL_STACK_SIZE ,NULL ,1 ,NULL );
   Init_Telnet       ( );
   Init_Udp          ( );
   Init_Spi_Phisical ( );


   vTaskStartScheduler();
   while(1)
       ;
}
