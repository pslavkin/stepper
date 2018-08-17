#ifndef  LEDS_SESSION
#define  LEDS_SESSION

#include "state_machine.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
//-----------------------------------------------------------
extern SemaphoreHandle_t Led_Eth_Data_Semphr;
extern void Init_Led_Eth_Data    ( void      );
extern void Led_Link_Task        ( void* nil );
extern void Led_Eth_Data_Task    ( void* nil );
extern void vApplicationIdleHook ( void      );
extern void Init_Leds            ( void      );
//----------------------------------------------------
//degino la posicino de los leds en el puerto N
#define LED_RUN_PIN      GPIO_PIN_1
#define LED_RUN_PORT     GPIO_PORTN_BASE
#define LED_RUN_PERIPH   SYSCTL_PERIPH_GPION

#define LED_ONE_WIRE_PIN      GPIO_PIN_0
#define LED_ONE_WIRE_PORT     GPIO_PORTN_BASE
#define LED_ONE_WIRE_PERIPH   SYSCTL_PERIPH_GPION

#define LED_LINK_PIN          GPIO_PIN_0
#define LED_LINK_PORT         GPIO_PORTF_BASE
#define LED_LINK_PERIPH       SYSCTL_PERIPH_GPIOF

//degino la posicino de los leds en el puerto F
#define LED_ETH_DATA_PIN         GPIO_PIN_4
#define LED_ETH_DATA_PORT        GPIO_PORTF_BASE
#define LED_ETH_DATA_PERIPH      SYSCTL_PERIPH_GPIOF


//degino la posicino del boton
#define BUTTON1_PIN    GPIO_PIN_0
#define BUTTON1_PORT   GPIO_PORTJ_BASE
#define BUTTON1_PERIPH SYSCTL_PERIPH_GPIOJ
#define BUTTON2_PIN    GPIO_PIN_1
#define BUTTON2_PORT   GPIO_PORTJ_BASE
#define BUTTON2_PERIPH SYSCTL_PERIPH_GPIOJ
//---------------------------------------------------------

#endif

