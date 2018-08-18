#ifndef __OPT_H__
#define __OPT_H__


#define APP_INPUT_BUF_SIZE      128 
#define UART_BUFFERED
#define UART_RX_BUFFER_SIZE     128
#define UART_TX_BUFFER_SIZE     2048


#define USR_FLASH_START 0x00020000                          //donde te plazca que no pise codigo. y que este alineado a 4
#define USR_FLASH_END   0x00028000                          // 2 bloques de 16 minimo para garantizar lectura
#define USR_FLASH_SIZE  (USR_FLASH_END-USR_FLASH_START)/128 // no puede haber mas de 128 bloques.. y como cada pedazo borrable de flas es de 16k. 32k/128 son 256 bytes por cada estructura de parametros.. por ahora creo que no se que poner que ocupe 256 bytes.. asi que sobra.. sino se baja este numer y chau

#define SSI_BIT_RATE  10000

#endif // __COMMANDS_H__
