#include <schedule.h>
#include <state_machine.h>
#include <events.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

struct Schedule_Event_Machine   // estructura que define los campos que se usaran para almacenar los tim_outs solicitados...
  {
   unsigned int Fixed_Time_Out; // este entero se acuerda de lo que le pidieron inicialmente para eventos repetitivos...
   unsigned int Time_Out;       // un entero para el time_out para poder tener tiempos largos...
   unsigned int Event;          // el evento que enviara cuando se acabe el tiempo...
   unsigned char Enable;        // es una manera de poder parar o continuar una schedule...me parecio practuco y lo uso puntualmente en una aplicacion...
   union 
   {
    const State      **Machine; // el solicitante...
    void (*Func) (void);
   }Sm_Func;
  } Schedule_Event_Machine;

struct Schedule_Event_Machine Schedule_List[MAX_SCHEDULE_INDEX];    
//----------------------------------------------------------------------
void Init_Schedule(void)            //iniicializa la lista de tiempos...
{
 unsigned char i;
 for(i=0;i<MAX_SCHEDULE_INDEX;i++) Schedule_List[i].Sm_Func.Machine=Empty_State_Machine;
}
//----------------------------------------------------------------------
void Schedule(void* nil)
{
   unsigned char i;
   Init_Schedule();           //iniicializa la lista de tiempos...
   while(1) {
      for(i=0;i<MAX_SCHEDULE_INDEX;i++)                       //para toda la lista...
         if(Schedule_List[i].Sm_Func.Machine!=Empty_State_Machine && Schedule_List[i].Enable)   {//si la entrada no es libre
            if(!Schedule_List[i].Time_Out) {                   //si el decrementado time_out es cero....
               if(Schedule_List[i].Event!=Invalid_Event) Atomic_Send_Event(Schedule_List[i].Event,Schedule_List[i].Sm_Func.Machine);     //ei el evento es valido manda un mensaje a la maquina de estados que lo solicito...
               else Schedule_List[i].Sm_Func.Func();                                    //si el evento es invalido, entonces es un schedule de funcion y se la ejecuta asi sin mas...
            Schedule_List[i].Time_Out=Schedule_List[i].Fixed_Time_Out;                   //se transfiere el fixed en time_out renovando el conteo. En caso de que sea cero se libera la entrada en la proxima linea y es equivalente a un evento unico...
            if(!Schedule_List[i].Time_Out) Schedule_List[i].Sm_Func.Machine=Empty_State_Machine;           //si el time_out es cero entonces se libera la entrada....
         }
         else
          --Schedule_List[i].Time_Out;                   //si no es cero entonces decrementa (para evutar que si entra con cero pase a 0xFFFF!).
      }
      vTaskDelay( 100 / portTICK_RATE_MS );
   }
}
//----------------------------------------------------------------------
void Add_Schedule(unsigned char Index,unsigned int Fixed_Time_Out,unsigned int Time_Out,unsigned int Event,const State** Machine,void (*Func)(void),unsigned char Sm_Func)    //enlista una nueva entrada de tiempo ...
{
 Schedule_List[Index].Fixed_Time_Out=Fixed_Time_Out?--Fixed_Time_Out:Fixed_Time_Out;
 Schedule_List[Index].Time_Out=Time_Out?--Time_Out:Time_Out;
 Schedule_List[Index].Event=Event;
 Schedule_List[Index].Enable=1;
 if(!Sm_Func) Schedule_List[Index].Sm_Func.Machine=Machine; else Schedule_List[Index].Sm_Func.Func=Func;
}
unsigned char Search_Schedule_Index(void)
{
 unsigned char i;
 for(i=0;i<MAX_SCHEDULE_INDEX && Schedule_List[i].Sm_Func.Machine!=Empty_State_Machine;i++); //recorre todas las entradas fijas en busca de un lugar vacio...
 return i;                                //si no hubo lugar, lamento mucho devuelve 0Xff...
}
//----------------------------------------------------------------------
unsigned char Update_Schedule(unsigned int Time_Out,unsigned int Event,const State** Machine)      //actualiza el timout de un evento conociendo su indexador otorgado en el momento de asignar el timeout
{
 unsigned char i;
 for(i=0;i<MAX_SCHEDULE_INDEX && !(Schedule_List[i].Sm_Func.Machine==Machine && Schedule_List[i].Event==Event);i++); //recorre todas las entradas fijas en busca de un lugar vacio...
 if(i<MAX_SCHEDULE_INDEX) Add_Schedule(i,Time_Out,Time_Out,Event,Machine,Rien,0);
 return i;
}
void Free_Schedule(unsigned int Event,const State** Machine)
{
 unsigned char i;
 for(i=0;i<MAX_SCHEDULE_INDEX && !(Schedule_List[i].Sm_Func.Machine==Machine && Schedule_List[i].Event==Event);i++); //recorre todas las entradas fijas en busca de un lugar vacio...
 if(i<MAX_SCHEDULE_INDEX) Add_Schedule(i,0,0,0,Empty_State_Machine,Rien,0);                  //actualiza la entada pero con una maquina VACIA!! que es equivelente eliminar la entrada...
}
void Free_All_Schedule(const State** Machine)
{
 unsigned char i;
 for(i=0;i<MAX_SCHEDULE_INDEX;i++)
  if(Schedule_List[i].Sm_Func.Machine==Machine) Add_Schedule(i,0,0,0,Empty_State_Machine,Rien,0);                 //actualiza la entada pero con una maquina VACIA!! que es equivelente eliminar la entrada...
}
void Pause_Schedule(unsigned int Event,const State** Machine)
{
 unsigned char i;
 for(i=0;i<MAX_SCHEDULE_INDEX && (Schedule_List[i].Sm_Func.Machine!=Machine || Schedule_List[i].Event!=Event);i++);  //recorre todas las entradas fijas en busca de un lugar vacio...
 if(i<MAX_SCHEDULE_INDEX) Schedule_List[i].Enable=0;
}
unsigned char Resume_Schedule(unsigned int Event,const State** Machine)
{
 unsigned char i;
 for(i=0;i<MAX_SCHEDULE_INDEX && !(Schedule_List[i].Sm_Func.Machine==Machine && Schedule_List[i].Event==Event);i++); //recorre todas las entradas fijas en busca de un lugar vacio...
 if(i<MAX_SCHEDULE_INDEX) Schedule_List[i].Enable=1;                       //si la encontro dentro de la table la reestablece para que siga contando...
 return i;
}
void Resume_Or_New_Periodic_Schedule(unsigned int Time_Out,unsigned int Event,const State** Machine)
{
 if(!(Resume_Schedule(Event,Machine)<MAX_SCHEDULE_INDEX)) New_Periodic_Schedule(Time_Out,Event,Machine);
}
void Update_Or_New_None_Periodic_Schedule(unsigned int Time_Out,unsigned int Event,const State** Machine)
{
 if(!(Update_Schedule(Time_Out,Event,Machine)<MAX_SCHEDULE_INDEX)) New_None_Periodic_Schedule(Time_Out,Event,Machine);
}
void New_Periodic_Schedule(unsigned int Time_Out,unsigned int Event,const State** Machine)
{
 unsigned char i; 
 if((i=Search_Schedule_Index())<MAX_SCHEDULE_INDEX);
   Add_Schedule(i,Time_Out,Time_Out,Event,Machine,Rien,0);
}
void New_None_Periodic_Schedule(unsigned int Time_Out,unsigned int Event,const State** Machine)    //enlista una nueva entrada de tiempo ...
{
 unsigned char i; 
 if((i=Search_Schedule_Index())<MAX_SCHEDULE_INDEX)
   Add_Schedule(i,0,Time_Out,Event,Machine,Rien,0);
}
unsigned int Read_Schedule_TOut(unsigned int Event,const State** Machine)
{
 unsigned char i; 
 if((i=Search_Schedule_Index())<MAX_SCHEDULE_INDEX) return Schedule_List[i].Time_Out;
 return 0;
}
//----------------------------------------------------------------------
void Free_Func_Schedule(void (*Func)(void))  
{
 unsigned char i;
 for(i=0;i<MAX_SCHEDULE_INDEX && Schedule_List[i].Sm_Func.Func!=Func;i++); //recorre todas las entradas fijas en busca de un lugar vacio...
 if(i<MAX_SCHEDULE_INDEX) Add_Schedule(i,0,0,0,Empty_State_Machine,Rien,0);
}
void Update_Func_Schedule(unsigned int Time_Out,void (*Func)(void))  
{
 unsigned char i;
 for(i=0;i<MAX_SCHEDULE_INDEX && Schedule_List[i].Sm_Func.Func!=Func;i++); //recorre todas las entradas fijas en busca de un lugar vacio...
 if(i<MAX_SCHEDULE_INDEX) Add_Schedule(i,Time_Out,Time_Out,Invalid_Event,Empty_Sm(),Func,1);
}
void New_None_Periodic_Func_Schedule(unsigned int Time_Out,void (*Func)(void))    //enlista una nueva entrada de tiempo ...
{
 unsigned char i; 
 if((i=Search_Schedule_Index())<MAX_SCHEDULE_INDEX)
   Add_Schedule(i,0,Time_Out,Invalid_Event,Empty_Sm(),Func,1); //como se pasa el parametro Sm_Func en uno, se refiere a un schedul de funcion, con lo cual el parametro Empty_SM(), no se usa!!...
}
void New_Periodic_Func_Schedule(unsigned int Time_Out,void (*Func)(void))    //enlista una nueva entrada de tiempo ...
{
 unsigned char i; 
 if((i=Search_Schedule_Index())<MAX_SCHEDULE_INDEX)
   Add_Schedule(i,Time_Out,Time_Out,Invalid_Event,Empty_Sm(),Func,1);   //como se pasa el parametro Sm_Func en uno, se refiere a un schedul de funcion, con lo cual el parametro Empty_SM(), no se usa!!...
}

void Pause_Func_Schedule         (void (*Func)(void))
{
// Pause_Schedule(Invalid_Event,(const State**)Func);
}
unsigned char Resume_Func_Schedule     (void (*Func)(void))                {
//   return Resume_Schedule(Invalid_Event,(const State**)Func);
   return 0;
}
void Resume_Or_New_Periodic_Func_Schedule (unsigned int Time_Out,void (*Func)(void))
{
 if(!(Resume_Func_Schedule(Func)<MAX_SCHEDULE_INDEX)) New_Periodic_Func_Schedule(Time_Out,Func);
}
//----------------------------------------------------------------------
void Periodic_1Sec4Sm   ( const State** Machine ) { New_Periodic_Schedule(10,Sec1_Event,Machine)         ;}
void None_Periodic_1Sec ( void                  ) { New_None_Periodic_Schedule(10,Sec1_Event,Actual_Sm());}
void Free_Schedule_1Sec ( void                  ) { Free_Schedule(Sec1_Event,Actual_Sm())                ;}
//----------------------------------------------------------------------
void None_Periodic_1Min          ( void ) { New_None_Periodic_Schedule(600,Min1_Event,Actual_Sm())      ;}
void Periodic_1Min               ( void ) { New_Periodic_Schedule(600,Min1_Event,Actual_Sm())           ;}
void Pause_Periodic_1Min         ( void ) { Pause_Schedule(Min1_Event,Actual_Sm())                      ;}
void Resume_Periodic_1Min        ( void ) { Resume_Schedule(Min1_Event,Actual_Sm())                     ;}
void Free_Schedule_1Min          ( void ) { Free_Schedule(Min1_Event,Actual_Sm())                       ;}
void Resume_Or_New_Periodic_1Min ( void ) { if(!Resume_Schedule(Min1_Event,Actual_Sm())) Periodic_1Min();}
//----------------------------------------------------------------------


