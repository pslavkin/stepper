#ifndef  BUTTONS
#define  BUTTONS

//-----------------------------------------------------------
void     Init_Busy      ( void );
bool     Busy_Read      ( void );
void     BusyIntHandler ( void );
//----------------------------------------------------
#define BUSY_PIN        GPIO_PIN_2
#define BUSY_PORT       GPIO_PORTP_BASE
#define BUSY_PERIPH     SYSCTL_PERIPH_GPIOP
#define BUSY_INT        INT_GPIOP2
void Print_Busy_Flag( void );
void Reset_Busy_Flag( void );
extern SemaphoreHandle_t Busy_Semphr;
//---------------------------------------------------------

#endif

