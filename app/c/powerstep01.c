#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "opt.h"
#include "commands.h"
#include "spi_phisical.h"

//-------------------------------------------------------------
void Set_Acc(float* V)
{
   uint32_t Ans[ NUM_AXES ];
   uint8_t  i              ;
   for(i=0;i<NUM_AXES;i++)
      Ans[i]=(uint32_t)(V[i]*0.068719476736);
   Set_Reg (Acc_Reg,Ans,2);
}
void Set_Dec(float* V)
{
   uint32_t Ans[ NUM_AXES ];
   uint8_t  i              ;
   for(i=0;i<NUM_AXES;i++)
      Ans[i]=(uint32_t)(V[i]*0.068719476736);
   Set_Reg (Dec_Reg,Ans,2);
}
void Set_Max_Speed(float* V)
{
   uint32_t    Ans[ NUM_AXES ];
   uint8_t     i              ;
      for(i=0;i<NUM_AXES;i++)
         Ans[i]=(uint32_t)(V[i]*0.065535);
      Set_Reg( Max_Speed_Reg,Ans,2);
}
float* Get_Max_Speed(float* V)
{
   uint32_t    Ans[ NUM_AXES ];
   uint8_t     i              ;
   Get_Reg     ( Max_Speed_Reg,Ans,2 );
   for(i=0;i<NUM_AXES;i++)
      V[i]=Ans[i]/0.065535;
   return V;
}

void Set_Min_Speed(float* V)
{
   uint32_t    Ans[ NUM_AXES ];
   uint8_t     i              ;
   for(i=0;i<NUM_AXES;i++)
      Ans[i]=(uint32_t)(V[i]*4.194304);
   Set_Reg ( Min_Speed_Reg,Ans,2);
}
float* Get_Min_Speed(float* V)
{
   uint32_t    Ans[ NUM_AXES ];
   uint8_t     i              ;
   Get_Reg ( Min_Speed_Reg,Ans,2 );
   for(i=0;i<NUM_AXES;i++)
      V[i]=Ans[i]/4.194304;
   return V;
}
void Goto(uint32_t* Target)
{
   uint8_t  Options[ NUM_AXES ]={0};
   Send_Data(Goto_Cmd,Options,Target,3);
}
void Run(uint8_t* Dir, float* V)
{
   uint32_t Ans[ NUM_AXES ];
   uint8_t i;
   for(i=0;i<NUM_AXES;i++)
      Ans[i]=V[i]*67.108864;
   Send_Data(Run_Dir_Cmd,Dir,Ans,3);
}
int32_t*  Abs_Pos(int32_t* Pos)
{
   Get_Reg(Abs_Pos_Reg,(uint32_t*)Pos,3);
   return Pos;
}



//----------------------------------------------------------------------------------------------------
