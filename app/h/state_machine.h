#ifndef STATE_MACHINE
#define STATE_MACHINE

#include "stdbool.h"
#include "stdint.h"
#include "driverlib/cpu.h"
#include "driverlib/rom_map.h"
#include "driverlib/interrupt.h"

//------------------------------------------------------------------
typedef struct Efn      //Event-Function-Next
 {
   unsigned int Event;
   void (*Func) (void);
   const struct Efn *Next_State;
 } State;
//------------------------------------------------------------------
//#define Atomic(Function)   {CPUcpsid();Function;CPUcpsie();}
#define Atomic(Function)   {MAP_IntMasterDisable();Function;MAP_IntMasterEnable();}
//------------------------------------------------------------------
void           Rien          ( void                                         );
void           Set_State     ( const State* New_State,const State** Machine );
unsigned int   Actual_Event  ( void                                         );
const State**  Actual_Sm     ( void                                         );
void           State_Machine ( void* nil                                         ); // maquina principal de principales que ejecuta la maquina de estados...
const State**  Empty_Sm      ( void                                         );
const State*   Empty_App     ( void                                         );
void           Soft_Reset    ( void                                         );
// void     Delay_100useg  (void);
void           Delay_Useg    ( unsigned int Useg                            );
//------------------------------------------------------------------
#define  Empty_State_Machine     ((const State**)0x00000000)

enum Event_Code {
       Empty_Event   = 0x0000, // este evento se usa para saber cuando la cola de eventos esta vacia...
       Invalid_Event = 0xFFFD,
       Rti_Event     = 0xFFFE,
       ANY_Event     = 0xFFFF  // este eventos se usa como default en la comparacion. Es decir si el evento no coincide con ninguno de los posibles, entonces terminara la busqueda cuando encuentre ANY. Notar que es muy imporante que haya un ANY en cada estado para tener por donde salir en el caso de que las comparaciones fallen.
      };
//------------------------------------------------------------------
#endif

