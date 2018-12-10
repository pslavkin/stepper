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
#ifdef CNC_STEPPERS
//para CNC
   V[0]=20;
   V[1]=20;
   V[2]=30;
   Set_Reg ( 9 ,V ,1 );//hold
   V[0]=40;
   V[1]=40;
   V[2]=75;
   Set_Reg ( 10 ,V ,1 ); //run
   V[0]=30;
   V[1]=30;
   V[2]=50;
   Set_Reg ( 11 ,V ,1 ); //acc
   V[0]=30;
   V[1]=30;
   V[2]=35;
   Set_Reg ( 12 ,V ,1 ); //dec

#else
//para debug con steppers
   V[0]=20;
   V[1]=20;
   V[2]=20;
   Set_Reg ( 9 ,V ,1 );//hold
   V[0]=20;
   V[1]=20;
   V[2]=20;
   Set_Reg ( 10 ,V ,1 ); //run
   V[0]=20;
   V[1]=20;
   V[2]=20;
   Set_Reg ( 11 ,V ,1 ); //acc
   V[0]=20;
   V[1]=20;
   V[2]=20;
   Set_Reg ( 12 ,V ,1 ); //dec
#endif

   V[0]=0x4F8B;                              // uno genera el clk de salida de 16m desde su clk interno
   V[1]=0x4F8D;                              // y ekl otro recibe y regenera inviertido
   V[2]=0x4F8D;
   Set_Reg       ( Config_Reg  ,V      ,2 );
   V[0]=0x01;                                // uno genera el clk de salida de 16m desde su clk interno
   V[1]=0x02;                                // y ekl otro recibe y regenera inviertido
   V[2]=0x01;
   Set_Reg       ( Ocd_Reg     ,V      ,1 ); // overcurren ,para Y un poco mas porqu eosn 2 motores
   Set_Reg_Equal ( Dec_Reg     ,0x000A ,2 );
   Set_Reg_Equal ( Acc_Reg     ,0x000A ,2 );
   Set_Reg_Equal ( Gate1_Reg   ,0x03C3 ,2 ); // boost=250ns Igate =64mA  Tcc= 500nseg tiempo de corriente constate al prender el mos
   Set_Reg_Equal ( Gate2_Reg   ,0x63   ,1 ); // Tblank para medir corriente = 500n  tdt deadtime = 500ns
   Set_Reg_Equal ( Fs_Spd      ,0x02FF ,2 ); // full step speed
   Set_Reg_Equal ( Int_Speed   ,3300   ,2 ); // intersection speed threshold
   V[0]=50;                                // uno genera el clk de salida de 16m desde su clk interno
   V[1]=50;                                // y ekl otro recibe y regenera inviertido
   V[2]=50;
   Set_Reg       ( St_Slp      ,V     ,1 ); // start slope compensation
   Set_Reg_Equal ( Fn_Slp_Acc  ,0x00   ,1 ); // fins slope
   Set_Reg_Equal ( Fn_Slp_Dec  ,0x00   ,1 ); //
   Set_Reg_Equal ( Go_Home_Cmd ,0x00   ,0 ); //
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
   uint8_t  Options[ NUM_AXES ]={0,0,0};
   uint32_t T[NUM_AXES];
   uint8_t i;
   for(i=0;i<NUM_AXES;i++)
      T[i]=Target[i]&0x003FFFFF;
   Send_Data(Goto_Cmd,Options,T,3);
}
void Goto_Mark(int32_t* Target)
{
   uint32_t T[NUM_AXES];
   uint8_t i;
   for(i=0;i<NUM_AXES;i++)
      T[i]=Target[i]&0x003FFFFF;
   Set_Reg        ( Mark_Reg     ,T,3 );
   Send_App_Equal ( Go_Mark_Cmd, 0,0,0      )            ;
}
void Run(uint8_t* Dir, float* V)
{
   uint32_t Ans[ NUM_AXES ];
   uint8_t i;
   for(i=0;i<NUM_AXES;i++)
      Ans[i]=(float)V[i]*67.108864;
   Send_Data(Run_Dir_Cmd,Dir,Ans,3);
}

void Set_Abs_Pos(int32_t* Target)
{
   uint32_t T[NUM_AXES];
   uint8_t i;
   for(i=0;i<NUM_AXES;i++)
      T[i]=Target[i]&0x003FFFFF;
   Set_Reg(Abs_Pos_Reg,T,3);
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
