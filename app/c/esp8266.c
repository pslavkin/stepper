#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "utils/cmdline.h"
#include "driverlib/sysctl.h"
#include "driverlib/emac.h"
#include "driverlib/flash.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "utils/cmdline.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "utils/lwiplib.h"
#include "opt.h"
#include "commands.h"
#include "utils/ringbuf.h"
#include "esp8266.h"
#include "telnet.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "gcode.h"

static void Esp_UARTPrimeTransmit(uint32_t ui32Base);

tRingBufObject Esp_Rx_RB, Esp_Tx_RB;
uint8_t Esp_Rx_Buffer[UART_RX_BUFFER_SIZE];
uint8_t Esp_Tx_Buffer[UART_TX_BUFFER_SIZE];

SemaphoreHandle_t Esp_Uart_Semphr;

void Init_Uart7(void)
{
    Esp_Uart_Semphr = xSemaphoreCreateCounting ( 1000, 0 );
    RingBufInit ( &Esp_Rx_RB,Esp_Rx_Buffer,sizeof(Esp_Rx_Buffer));
    RingBufInit ( &Esp_Tx_RB,Esp_Tx_Buffer,sizeof(Esp_Tx_Buffer));

   ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
   ROM_GPIOPinConfigure(GPIO_PC4_U7RX);
   ROM_GPIOPinConfigure(GPIO_PC5_U7TX);
   ROM_GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5);
   // Enable the UART peripheral for use.
   MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART7);
   // Configure the UART for 115200, n, 8, 1
   MAP_UARTConfigSetExpClk(UART7_BASE,
                            configCPU_CLOCK_HZ,
                            115200,
                            (UART_CONFIG_PAR_NONE |
                             UART_CONFIG_STOP_ONE |
                             UART_CONFIG_WLEN_8)
                           );
    MAP_UARTFIFOLevelSet(UART7_BASE, UART_FIFO_TX1_8, UART_FIFO_RX1_8);
    //
    // We are configured for buffered output so enable the master interrupt
    // for this UART and the receive interrupts.  We don't actually enable the
    // transmit interrupt in the UART itself until some data has been placed
    // in the transmit buffer.
    //
    MAP_UARTIntDisable(UART7_BASE, 0xFFFFFFFF);
    MAP_UARTIntEnable(UART7_BASE, UART_INT_RX | UART_INT_RT);
    MAP_IntEnable(INT_UART7);
    MAP_UARTEnable(UART7_BASE);
}
struct Gcode_Queue_Struct D,E;

void Esp_Task(void* nil)
{
   uint32_t Esp_Id=0;
   while(1) {
//      while(xSemaphoreTake(Esp_Uart_Semphr,portMAX_DELAY)!=pdTRUE) 
//            ;
      xSemaphoreTake(Esp_Uart_Semphr,pdMS_TO_TICKS(1000));
      {
         int32_t Len=RingBufPeek(&Esp_Rx_RB,'\n');
         if(Len>0) {
            RingBufRead(&Esp_Rx_RB,D.Buff,Len+1);
            D.Buff[Len]='\0';
            D.tpcb = ESP_MSG;
            D.Id   = Esp_Id;
            if(ustrncmp("+IPD,0,",(char*)D.Buff,7)==0) {
                  char L[10];
                  uint16_t Len,i;
                  for(i=7;D.Buff[i]!=':' && D.Buff[i]!='\0';i++)
                     L[i-7]=D.Buff[i];
                  L[i-7]='\0';
                  Len=atoi(L);
                  ustrncpy((char*)E.Buff,(char*)D.Buff+i+1,Len);
                  E.tpcb=D.tpcb;
                  E.Id=Esp_Id;
                  Esp_Id++;
                  while(xQueueSend(Gcode_Queue,&E,portMAX_DELAY)!=pdTRUE)
                     ;
                  Print_Slide(&E);
                  }
            else
               UART_ETHprintf(UART_MSG,"%s\r\n",D.Buff);
//            while(xQueueSend(Gcode_Queue,&D,portMAX_DELAY)!=pdTRUE)
//               ;
            }
         else if(Len==0) {
            RingBufReadOne(&Esp_Rx_RB);
         }
      }
   }
}

int Esp_UARTwrite(const char *pcBuf, uint32_t ui32Len)
{
   RingBufWrite(&Esp_Tx_RB,(uint8_t*)pcBuf,ui32Len);
   Esp_UARTPrimeTransmit(UART7_BASE);
   MAP_UARTIntEnable(UART7_BASE, UART_INT_TX);
   return(ui32Len);
}
static void Esp_UARTPrimeTransmit(uint32_t ui32Base)
{
        while(MAP_UARTSpaceAvail(UART7_BASE) && !RingBufEmpty(&Esp_Tx_RB))
        {
          MAP_UARTCharPutNonBlocking(UART7_BASE,RingBufReadOne(&Esp_Tx_RB));
        }
}
void UART_Esp_Handler(void)
{
    uint32_t ui32Ints;
    int8_t cChar;
    int32_t i32Char;

    // Get and clear the current interrupt source(s)
    ui32Ints = MAP_UARTIntStatus(UART7_BASE, true);
    MAP_UARTIntClear(UART7_BASE, ui32Ints);

    // Are we being interrupted because the TX FIFO has space available?
    if(ui32Ints & UART_INT_TX)
    {
        // Move as many bytes as we can into the transmit FIFO.
        Esp_UARTPrimeTransmit(UART7_BASE);
        // If the output buffer is empty, turn off the transmit interrupt.
      if(RingBufEmpty(&Esp_Tx_RB)) {
          MAP_UARTIntDisable(UART7_BASE, UART_INT_TX);
      }
    }

    // Are we being interrupted due to a received character?
    if(ui32Ints & (UART_INT_RX | UART_INT_RT))
    {
        // Get all the available characters from the UART.
        while(MAP_UARTCharsAvail(UART7_BASE))
        {
            // Read a character
            i32Char = MAP_UARTCharGetNonBlocking(UART7_BASE);
            cChar   = (unsigned char)(i32Char & 0xFF);
            // See if a newline or escape character was received.
            if((cChar == '\r') || (cChar == '\n') || (cChar == 0x1b)) {
                    // put a CR in the receive buffer as a marker 
               cChar = '\n';
            }
            // If there is space in the receive buffer,
            if(!RingBufFull(&Esp_Rx_RB)) {
                // Store the new character in the receive buffer
                RingBufWriteOne(&Esp_Rx_RB,cChar);
                if(cChar == '\n')
                   xSemaphoreGiveFromISR(Esp_Uart_Semphr,NULL);
            }
        }
    }
}

