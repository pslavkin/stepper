#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/emac.h"
#include "driverlib/rom_map.h"
#include "leds_session.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
//-------------------------------------------------------------------
void Init_Buttons(void)
{
   //MOSI PD1
 MAP_SysCtlPeripheralEnable ( SYSCTL_PERIPH_GPIOD                                             );
 MAP_GPIOPinTypeGPIOInput   ( GPIO_PORTD_BASE,GPIO_PIN_1                                      );
 MAP_GPIOPadConfigSet       ( GPIO_PORTD_BASE ,GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD );

 MAP_SysCtlPeripheralEnable ( BUTTON2_PERIPH                                                   );
 MAP_GPIOPinTypeGPIOInput   ( BUTTON2_PORT,BUTTON2_PIN                                         );
 MAP_GPIOPadConfigSet       ( BUTTON2_PORT,BUTTON2_PIN,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU );
}
uint8_t Button1_Read (void)
{
 return MAP_GPIOPinRead(BUTTON1_PORT,BUTTON1_PIN);
}

void Init_Leds(void)
{
 MAP_SysCtlPeripheralEnable (LED_RUN_PERIPH);
 MAP_GPIOPinTypeGPIOOutput  (LED_RUN_PORT,LED_RUN_PIN);

 MAP_SysCtlPeripheralEnable (LED_ONE_WIRE_PERIPH);
 MAP_GPIOPinTypeGPIOOutput  (LED_ONE_WIRE_PORT,LED_ONE_WIRE_PIN);
}

void Led_One_Wire_Set   ( void ) { GPIOPinSet (LED_ONE_WIRE_PORT,LED_ONE_WIRE_PIN)   ;}
void Led_One_Wire_Reset ( void ) { GPIOPinReset  (LED_ONE_WIRE_PORT,LED_ONE_WIRE_PIN);}


void vApplicationIdleHook(void)
{
   static uint16_t State=0;
   MAP_GPIOPinWrite(LED_RUN_PORT,LED_RUN_PIN,++State&0x4000?LED_RUN_PIN:0);
}


SemaphoreHandle_t Led_Eth_Data_Semphr;
void Init_Led_Eth_Data(void)
{
   MAP_SysCtlPeripheralEnable (LED_ETH_DATA_PERIPH);
   MAP_GPIOPinTypeGPIOOutput  (LED_ETH_DATA_PORT,LED_ETH_DATA_PIN);
   Led_Eth_Data_Semphr = xSemaphoreCreateCounting(20,0);
}
void Led_Eth_Data_Task ( void* nil )
{
   while(1) {
      GPIOPinReset   ( LED_ETH_DATA_PORT,LED_ETH_DATA_PIN ) ;
      xSemaphoreTake ( Led_Eth_Data_Semphr, portMAX_DELAY    ) ;
      GPIOPinSet     ( LED_ETH_DATA_PORT,LED_ETH_DATA_PIN ) ;
      vTaskDelay     ( pdMS_TO_TICKS(10              ));
   }
}

void Led_Link_Task(void* nil)
{
   SysCtlPeripheralEnable (LED_LINK_PERIPH);
   GPIOPinTypeGPIOOutput  (LED_LINK_PORT,LED_LINK_PIN);
   while(1) {
      vTaskDelay(pdMS_TO_TICKS(1000));
      GPIOPinWrite(LED_LINK_PORT, LED_LINK_PIN, EMACPHYLinkUp());
   }
}
//------------------------------------------------------------------
