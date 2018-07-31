#include <events.h>

Events Event_Buffer[MAX_EVENTS+1];        //array usado de buffer para almacenar los datos...
unsigned char Point2Write,Point2Read;     //puntero para escritura y lectura...
 
#define Next_Pointer(Actual)     ((Actual == MAX_EVENTS)?0x00:Actual+1)
#define Previous_Pointer(Actual) ((Actual == 0)?MAX_EVENTS:Actual-1)

//-----------------------------------------------------------------
void Init_Events(void)  
{
 Point2Read = Point2Write = 0x00;      //inicializa la cola circular de eventos...
 Init_Event_Listener();          //inicializa el sistema de redireccionamiento multiple de eventos..
}
//-----------------------------------------------------------------
void Send_Event(unsigned int Event,const State** Machine)   //escribe un nuevo dato a la cola...
{
 unsigned char Next=Next_Pointer(Point2Write);
 if (Next != Point2Read)            //si el siguiente casillero no esta apuntado por Point2Read...
   {
    Event_Buffer[Point2Write].Machine = Machine;   //almacena el dato...
    Event_Buffer[Point2Write].Event   = Event;     //almacena el dato...
    Point2Write = Next;             //apunta al siguiente...
   }
  else
    for(;;);                  //debug
 return;
}
void Atomic_Send_Event(unsigned int Event,const State** Machine) {Atomic(Send_Event(Event,Machine));} 
//-------------------------------------------------------------------------------------
void Insert_Event(unsigned int Event,const State** Machine) // escribe un nuevo dato a la cola...
{
 unsigned char Previous=Previous_Pointer(Point2Read);
 if (Previous != Point2Write)                               // si el siguiente casillero no esta apuntado por Point2Read...
   {
    Point2Read = Previous;                                  // apunta al siguiente...
    Event_Buffer[Point2Read].Machine = Machine;             // almacena el dato...
    Event_Buffer[Point2Read].Event   = Event;               // almacena el dato...
   }
  else
   {
// Serial_Phisical1_Tx(0XEF); //debug
    for(;;);                                                // debug
   }
}
void Atomic_Insert_Event(unsigned int Event,const State** Machine)   {
   Atomic(Insert_Event(Event,Machine));
}
//-------------------------------------------------------------------------------------
Events Atomic_Read_Event(void)
{
   Events Ans;                                 // por defeccto devuelve Empty, con lo cual NO se procesa....
   CPUcpsid();
   if ( Point2Read != Point2Write ) {          // si los apuntadores son diferentes,es porque hay algo para leer...
      Ans=Event_Buffer[Point2Read];            // auxiliar para guardar la respuesta, que por defoult se graba la data de la posicion actual...
      Point2Read = Next_Pointer(Point2Read);   // se incrementa el puntero...
   }
   else Ans = (Events){0,Empty_State_Machine}; // por defeccto devuelve Empty, con lo cual NO se procesa....
   CPUcpsie();
 return Ans;                                   // notar que si no hay nada para leer, devuelve simempre el ultimo dato escrito...
}

//-------------------------------------------------------------------------------------
Events Event_Listener_Buffer[MAX_EVENT_LISTENER+1];
//-----------------------------
void Set_Event_Listener(unsigned char Index,unsigned int Event,const State** Machine)
{
 Event_Listener_Buffer[Index].Event=Event;
 Event_Listener_Buffer[Index].Machine=Machine;
}
//--------------------------------------
void Init_Event_Listener(void)            //iniicializa la lista de tiempos...
{
 unsigned char i;
 for(i=0;i<MAX_EVENT_LISTENER;i++)  Set_Event_Listener(i,0,(const State**)Empty_State_Machine);
}
//------------------------------------------
unsigned char Search_Empty_Event_Listener(void)
{
 unsigned char i;
 for(i=0;i<MAX_EVENT_LISTENER && Event_Listener_Buffer[i].Machine!=(const State**)Empty_State_Machine;i++); //recorre todas las entradas fijas en busca de un lugar vacio...
 return i;                                //si no hubo lugar, lamento mucho devuelve 0Xff...
}
//---------------------------------
void Send_Event2Event_Listener(unsigned int Search_Event,unsigned int Sending_Event)
{
 unsigned char i;
// Send_NVDebug_Events_Data2Serial(24,"Event Listener Received\n");
 for(i=0;i<MAX_EVENT_LISTENER;i++)
  if(Event_Listener_Buffer[i].Machine!=(const State**)Empty_State_Machine && Event_Listener_Buffer[i].Event==Search_Event)
    {
     Atomic_Send_Event(Sending_Event,Event_Listener_Buffer[i].Machine);
  //   Send_NVDebug_Events_Data2Serial(33,"Event Listener Finded and Sended\n");
    }
}
//----------------------------------------------------------------------
void Add_Event_Listener(unsigned int Event,const State** Machine)
{
 unsigned char i=Search_Empty_Event_Listener(); 
 if(i<MAX_EVENT_LISTENER) 
   {
    Set_Event_Listener(i,Event,Machine);
 //   Send_NVDebug_Events_Data2Serial(21,"Event Listener added\n");
 //   Send_NVDebug_Events_Data2Serial(4,(unsigned char*)&Event_Listener_Buffer[i]);
   }
}
//---------------------
void Free_Event_Listener(unsigned int Event,const State** Machine)
{
 unsigned char i;
 for(i=0;i<MAX_EVENT_LISTENER;i++)
  if(Event_Listener_Buffer[i].Machine==Machine && Event_Listener_Buffer[i].Event==Event)
    {
    Set_Event_Listener(i,ANY_Event,(const State**)Empty_State_Machine);
 //   Send_NVDebug_Events_Data2Serial(21,"Event Listener Deleted\n");
    }
}
//---------------------
void Free_All_Event_Listener(const State** Machine)
{
 unsigned char i;
 for(i=0;i<MAX_EVENT_LISTENER;i++)
  if(Event_Listener_Buffer[i].Machine==Machine)
    {
    Set_Event_Listener(i,ANY_Event,(const State**)Empty_State_Machine);
 //   Send_NVDebug_Events_Data2Serial(21,"Event Listener Deleted\n");
    }
}


