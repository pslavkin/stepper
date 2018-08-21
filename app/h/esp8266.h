#ifndef  ESP
#define  ESP

extern void Esp_Task         ( void* nil );
extern void Init_Uart7       ( void      );
extern void UART_Esp_Handler ( void      );
extern int Esp_UARTwrite(const char *pcBuf, uint32_t ui32Len);
extern int Esp_UARTgets(char *pcBuf, uint32_t ui32Len);

#endif

