#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#define configUSE_PREEMPTION                1
#define configUSE_IDLE_HOOK                 0
#define configUSE_TICK_HOOK                 0
#define configCPU_CLOCK_HZ                  ( ( unsigned long ) 120000000 )
#define configTICK_RATE_HZ                  ( ( portTickType ) 100 )
#define configMINIMAL_STACK_SIZE            ( ( unsigned short ) 300 )
#define configTOTAL_HEAP_SIZE               ( ( size_t ) ( 30000 ) )
#define configMAX_TASK_NAME_LEN             ( 16 )
#define configUSE_TRACE_FACILITY            1
#define configUSE_STATS_FORMATTING_FUNCTIONS 1
#define configUSE_16_BIT_TICKS              0
#define configIDLE_SHOULD_YIELD             0
#define configUSE_CO_ROUTINES               0
#define configUSE_MUTEXES                   1
#define configUSE_RECURSIVE_MUTEXES         1
#define configCHECK_FOR_STACK_OVERFLOW      0

#define configMAX_PRIORITIES                (16UL) // ( unsigned portBASE_TYPEE ) 16 )
#define configMAX_CO_ROUTINE_PRIORITIES     ( 2 )
#define configQUEUE_REGISTRY_SIZE           10
/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define configUSE_COUNTING_SEMAPHORES        1

#define INCLUDE_vTaskPrioritySet            1
#define INCLUDE_uxTaskPriorityGet           1
#define INCLUDE_vTaskDelete                 1
#define INCLUDE_vTaskCleanUpResources       0
#define INCLUDE_vTaskSuspend                1
#define INCLUDE_vTaskDelayUntil             1
#define INCLUDE_vTaskDelay                  1
#define INCLUDE_uxTaskGetStackHighWaterMark 1

/* Be ENORMOUSLY careful if you want to modify these two values and make sure
 * you read http://www.freertos.org/a00110.html#kernel_priority first!
 */
#define configKERNEL_INTERRUPT_PRIORITY         ( 7 << 5 )    /* Priority 7, or 0xE0 as only the top three bits are implemented.  This is the lowest priority. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY     ( 5 << 5 )  /* Priority 5, or 0xA0 as only the top three bits are implemented. */

#endif /* FREERTOS_CONFIG_H */
