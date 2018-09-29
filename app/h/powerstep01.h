#ifndef POWERSTEP01
#define POWERSTEP01

#include <stdbool.h>

void        Set_Acc       ( float* V               );
void        Set_Dec       ( float* V               );
void        Set_Max_Speed ( float* V               );
float*      Get_Max_Speed ( float* V               );
void        Set_Min_Speed ( float* V               );
float*      Get_Min_Speed ( float* V               );
void        Goto          (  int32_t* Target       );
void        Run           ( uint8_t* Dir, float* V );
void Stop(void);
void Hard_Stop(void);
int32_t*    Abs_Pos       ( int32_t* Pos           );
float* Speed(float* V);

#endif
