#ifndef SCHEDULE
#define SCHEDULE
#include <state_machine.h>
//------------------------------------------------------------------------
#define MAX_SCHEDULE_INDEX       9     //define el maximo numero de solicitantes de tiem+outs concurrentes...

enum Schedule_Events {
    Sec1_Event = 0x5000,
    Min1_Event = 0x5001
   };
//------------------------------------------------------------------------
void           Init_Schedule   ( void                                                           );
void           Schedule        ( void* nil                                                           );
unsigned char  Update_Schedule ( unsigned int Time_Out,unsigned int Event,const State** Machine );
//------------------------------------------------------------------------
void           Update_Func_Schedule       ( unsigned int Time_Out,void (*Func                              )(void));
void           Free_Schedule              ( unsigned int Event,const State** Machine                       )       ;
void           Free_All_Schedule          ( const State** Machine                                          )       ;
void           Pause_Schedule             ( unsigned int Event,const State** Machine                       )       ;
unsigned char  Resume_Schedule            ( unsigned int Event,const State** Machine                       )       ;
void           New_Periodic_Schedule      ( unsigned int Time_Out,unsigned int Event,const State** Machine )       ;
void           New_None_Periodic_Schedule ( unsigned int Time_Out,unsigned int Event,const State** Machine )       ;
unsigned int   Read_Schedule_TOut         ( unsigned int Event,const State** Machine                       )       ;
//------------------------------------------------------------------------
void           New_None_Periodic_Func_Schedule      ( unsigned int Time_Out,void (*Func                              )(void));
void           New_Periodic_Func_Schedule           ( unsigned int Time_Out,void (*Func                              )(void));
void           Pause_Func_Schedule                  ( void (*Func                                                    )(void));
unsigned char  Resume_Func_Schedule                 ( void (*Func                                                    )(void));
void           Free_Func_Schedule                   ( void (*Func                                                    )(void));
void           Resume_Or_New_Periodic_Func_Schedule ( unsigned int Time_Out,void (*Func                              )(void));
void           Update_Or_New_None_Periodic_Schedule ( unsigned int Time_Out,unsigned int Event,const State** Machine )       ;
//------------------------------------------------------------------------
void       Periodic_1Sec4Sm            ( const State** Machine );
void       None_Periodic_1Sec          ( void                  );
void       Free_Schedule_1Sec          ( void                  );
// -----------------------------------------
void       None_Periodic_1Min          ( void                  );
void       Periodic_1Min               ( void                  );
void       Pause_Periodic_1Min         ( void                  );
void       Resume_Periodic_1Min        ( void                  );
void       Free_Schedule_1Min          ( void                  );
void       Resume_Or_New_Periodic_1Min ( void                  );
//----------------------------

#endif

