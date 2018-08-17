#ifndef WDOG
#define WDOG
//-----------------------------------------------
extern void Init_Wdog    ( void );
extern void Wdog_Handler ( void );
extern void Wdog_Clear   ( void );
extern uint32_t Read_Uptime ( void );
//-----------------------------------------------
#endif
