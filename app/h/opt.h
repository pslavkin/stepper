#ifndef __OPT_H__
#define __OPT_H__

//comentar para usar steppers de debug
#define CNC_STEPPERS

#define APP_INPUT_BUF_SIZE      64
#define UART_BUFFERED
#define UART_RX_BUFFER_SIZE     256
#define UART_TX_BUFFER_SIZE     2048

#define GCODE_QUEUE_SIZE        50
#define MOVES_QUEUE_SIZE        200
#define MAX_EVENTS              10
#define SSI_BIT_RATE            5000000                                 //esta en 10k y podria llegar a 5Mb

#define USR_FLASH_START         0x00020000                          //donde te plazca que no pise codigo. y que este alineado a 4
#define USR_FLASH_END           0x00028000                          // 2 bloques de 16 minimo para garantizar lectura
#define USR_FLASH_SIZE          (USR_FLASH_END-USR_FLASH_START)/128 // no puede haber mas de 128 bloques.. y como cada pedazo borrable de flas es de 16k. 32k/128 son 256 bytes por cada estructura de parametros.. por ahora creo que no se que poner que ocupe 256 bytes.. asi que sobra.. sino se baja este numer y chau
#define DELAY_UNTIL_GOTO_MARGIN 980 //margen para el calculo de tiempo entre que tiro run y salta goto.. si me paso de 1000, el motor pega la vuelta para llegar a destino.. 1000 es el teorico, pero tengo que dejar margen.. si dejo mucho el goto tiene tiempo para cambiar las velocidades y distorciona el camino

#define X_SCALE          600  // micropasos por mm
#define Y_SCALE          600  // micropasos por mm
#define Z_SCALE          2560 // micropasos por mm
#define MICROSTEP        128


#endif // __COMMANDS_H__
