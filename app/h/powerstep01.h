#ifndef POWERSTEP01
#define POWERSTEP01

#include <stdbool.h>

void        Set_Acc       ( float* V               );
void        Set_Dec       ( float* V               );
void        Set_Max_Speed ( float* V               );
float*      Get_Max_Speed ( float* V               );
void        Set_Min_Speed ( float* V               );
float*      Get_Min_Speed ( float* V               );
void        Goto          ( uint32_t* Target       );
void        Run           ( uint8_t* Dir, float* V );
int32_t*    Abs_Pos       ( int32_t* Pos           );

#endif
