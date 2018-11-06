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
void Get_Acc(float* V)
{
   uint32_t Ans[ NUM_AXES ];
   uint8_t  i              ;
   Get_Reg ( Acc_Reg,Ans,2 );
   for(i=0;i<NUM_AXES;i++)
      V[i]=Ans[i]/0.068719476736;
}
void Get_Dec(float* V)
{
   uint32_t Ans[ NUM_AXES ];
   uint8_t  i              ;
   Get_Reg ( Dec_Reg,Ans,2 );
   for(i=0;i<NUM_AXES;i++)
      V[i]=Ans[i]/0.068719476736;
}
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
void Goto(int32_t* Target)
{
   uint8_t  Options[ NUM_AXES ]={0};
   Send_Data(Goto_Cmd,Options,(uint32_t*)Target,3);
}
void Run(uint8_t* Dir, float* V)
{
   uint32_t Ans[ NUM_AXES ];
   uint8_t i;
   for(i=0;i<NUM_AXES;i++)
      Ans[i]=(float)V[i]*67.108864L;
   Send_Data(Run_Dir_Cmd,Dir,Ans,3);
}
int32_t*  Abs_Pos(int32_t* Pos)
{
   uint8_t i;
   uint32_t Ans[ NUM_AXES ];
   Get_Reg(Abs_Pos_Reg,Ans,3);
   for(i=0;i<NUM_AXES;i++)
      Pos[i]=((Ans[i]&0x00200000)?0xFFC00000:0)|Ans[i];
   return Pos;
}
void Stop(void)
{
   Send_App_Equal (Soft_Stop_Cmd, 0,0,0);
}
void Hard_Stop(void)
{
   Send_App_Equal (Hard_Stop_Cmd, 0,0,0);
}
float* Speed(float* V)
{
   uint32_t Ans[ NUM_AXES ];
   uint8_t i;
   Get_Reg(Speed_Reg,Ans,3);
   for(i=0;i<NUM_AXES;i++)
      V[i]=Ans[i]/67.108864;
   return V;
}


//----------------------------------------------------------------------------------------------------
