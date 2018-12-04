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
void Init_Powerstep(void)
{
   uint32_t    V[ NUM_AXES ];
//   Set_Reg_Equal ( 9          , 30    ,1 );  //hold
//   Set_Reg_Equal ( 10         ,  80    ,1 ); //run para stepper de prueba 80, para cnc 150
//   Set_Reg_Equal ( 11         ,  80    ,1 ); //acc
//   Set_Reg_Equal ( 12         ,  80    ,1 ); //dec
//para debug con steppers
   V[0]=20; V[1]=20; V[2]=20;
   Set_Reg ( 9 ,V ,1 );//hold
   V[0]=20; V[1]=20; V[2]=20;
   Set_Reg ( 10 ,V ,1 );
   V[0]=20; V[1]=20; V[2]=20;
   Set_Reg ( 11 ,V ,1 );
   V[0]=20; V[1]=20; V[2]=20;
   Set_Reg ( 12 ,V ,1 );
   V[0]=20; V[1]=20; V[2]=20;
   //Set_Reg ( 9 ,V ,1 );//hold
   //V[0]=120; V[1]=130; V[2]=80;
   //Set_Reg ( 10 ,V ,1 );
   //V[0]=100; V[1]=160; V[2]=80;
   //Set_Reg ( 11 ,V ,1 );
   //V[0]=100; V[1]=160; V[2]=80;
   //Set_Reg ( 12 ,V ,1 );
   //minimo para que se mueva la maquina en 128
//   V[0]=30; V[1]=30; V[2]=30;
//   Set_Reg ( 9 ,V ,1 );//hold
//   V[0]= 70; V[1]= 80; V[2]=70;
//   Set_Reg ( 10 ,V ,1 );
//   V[0]= 70; V[1]= 80; V[2]=70;
//   Set_Reg ( 11 ,V ,1 );
//   V[0]= 70; V[1]= 80; V[2]=70;
//   Set_Reg ( 12 ,V ,1 );

   V[0]=0x4F0B; // uno genera el clk de salida de 16m desde su clk interno
   V[1]=0x4F0D; // y ekl otro recibe y regenera inviertido
   V[2]=0x4F0D;
   Set_Reg       ( Config_Reg ,V      ,2 );
   Set_Reg_Equal ( Dec_Reg    ,0x000A ,2 );
   Set_Reg_Equal ( Acc_Reg    ,0x000A ,2 );
   Set_Reg_Equal ( Gate1_Reg  ,0x03C3 ,2 ); // boost=250ns Igate =64mA  Tcc= 500nseg tiempo de corriente constate al prender el mos
   Set_Reg_Equal ( Gate2_Reg  ,0x63   ,1 ); // Tblank para medir corriente = 500n  tdt deadtime = 500ns
   Set_Reg_Equal ( Fs_Spd     ,0x02FF ,2 ); // full step speed
   Set_Reg_Equal ( Int_Speed  ,3300   ,2 ); // intersection speed threshold
   Set_Reg_Equal ( St_Slp     ,30     ,1 ); // start slope compensation
   Set_Reg_Equal ( Fn_Slp_Acc ,0x00   ,1 ); // fins slope
   Set_Reg_Equal ( Fn_Slp_Dec ,0x00   ,1 ); //
   Set_Reg_Equal ( Go_Home_Cmd,0x00   ,0 ); //
   //Set_Reg_Equal ( Step_Mode_Reg ,0x00   ,1 );
}
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
void Get_Max_Speed(float* V)
{
   uint32_t    Ans[ NUM_AXES ];
   uint8_t     i              ;
   Get_Reg     ( Max_Speed_Reg,Ans,2 );
   for(i=0;i<NUM_AXES;i++)
      V[i]=Ans[i]/0.065535;
}

void Set_Min_Speed(float* V)
{
   uint32_t    Ans[ NUM_AXES ];
   uint8_t     i              ;
   for(i=0;i<NUM_AXES;i++)
      Ans[i]=(uint32_t)(V[i]*4.194304);
   Set_Reg ( Min_Speed_Reg,Ans,2);
}
void Get_Min_Speed(float* V)
{
   uint32_t    Ans[ NUM_AXES ];
   uint8_t     i              ;
   Get_Reg ( Min_Speed_Reg,Ans,2 );
   for(i=0;i<NUM_AXES;i++)
      V[i]=Ans[i]/4.194304;
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
      Ans[i]=(float)V[i]*67.108864;
   Send_Data(Run_Dir_Cmd,Dir,Ans,3);
}
void  Abs_Pos(int32_t* Pos)
{
   uint8_t i;
   uint32_t Ans[ NUM_AXES ];
   Get_Reg(Abs_Pos_Reg,Ans,3);
   for(i=0;i<NUM_AXES;i++)
      Pos[i]=((Ans[i]&0x00200000)?0xFFC00000:0)|Ans[i];
}
void Stop(void)
{
   Send_App_Equal (Soft_Stop_Cmd, 0,0,0);
}
void Hard_Stop(void)
{
   Send_App_Equal (Hard_Stop_Cmd, 0,0,0);
}
void  Speed(float* V)
{
   uint32_t Ans[ NUM_AXES ];
   uint8_t i;
   Get_Reg(Speed_Reg,Ans,3);
   for(i=0;i<NUM_AXES;i++)
      V[i]=Ans[i]/67.108864;
}


//----------------------------------------------------------------------------------------------------
