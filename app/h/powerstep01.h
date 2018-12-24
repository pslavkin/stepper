#ifndef POWERSTEP01
#define POWERSTEP01

#include <stdbool.h>

void        Init_Powerstep ( void                   );
void        Get_Acc        ( float* V               );
void        Get_Dec        ( float* V               );
void        Set_Acc        ( float* V               );
void        Set_Dec        ( float* V               );
void        Set_Max_Speed  ( float* V               );
void      Get_Max_Speed  ( float* V               );
void        Set_Min_Speed  ( float* V               );
void      Get_Min_Speed  ( float* V               );
void        Goto           ( int32_t* Target        );
void        Run            ( uint8_t* Dir, float* V );
void        Stop           ( void                   );
void        Hard_Stop      ( void                   );
void Set_Abs_Pos(int32_t* Target);
void    Abs_Pos        ( int32_t* Pos           );
void      Speed          ( float* V               );
void Goto_Mark(int32_t* Target);
void Goto_Dir(int32_t* Target, uint8_t* Dir);

#endif
