#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include "utils/cmdline.h"
#include "opt.h"
#include "commands.h"
#include "moves.h"
#include "gcode.h"
#include "powerstep01.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"

Motor_t Motor={0};

void Delta(Motor_t* M)
{
   uint8_t i;
   for(i=0;i<NUM_AXES;i++) {
      if(M->Pos[i]>=M->Target[i]) {
         M->Delta[i] = M->Pos[i]-M->Target[i];
         M->Dir[i]   = 0;
      }
      else {
         M->Delta[i] = M->Target[i]-M->Pos[i];
         M->Dir[i]   = 1;
      }
   }
}
void Distance(Motor_t* M)
{
   M->Distance=sqrt((uint64_t)M->Delta[0]*M->Delta[0]+(uint64_t)M->Delta[1]*M->Delta[1]);
}
void Vel(Motor_t* M)
{
   uint8_t i;
   for(i=0;i<NUM_AXES;i++)
      M->Vel[i]=M->Delta[i]*M->Total_Vel/M->Distance;
}

void Accel(Motor_t* M)
{
   if ( M->Vel[0] > M->Vel[1] ) {
      M->Acc[0] = M->Total_Acc;
      M->Dec[0] = M->Total_Dec;

      M->Acc[1] = (M->Acc[0])/(M->Vel[0])*M->Vel[1];
      M->Dec[1] = (M->Dec[0])/(M->Vel[0])*M->Vel[1];
   }
   else {
      M->Acc[1] = M->Total_Acc;
      M->Dec[1] = M->Total_Dec;

      M->Acc[0] = (M->Acc[1]/M->Vel[1])*M->Vel[0];
      M->Dec[0] = (M->Dec[1]/M->Vel[1])*M->Vel[0];
   }
}

int Cmd_Gcode_Print_Motor(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   UART_ETHprintf(tpcb,"X=%d Y=%d Next_X=%d Next_Y=%d Vel_X=%f Vel_Y=%f Acc_X=%f Acc_Y=%f Distance=%f \r\n",
         Motor.Pos[0], Motor.Pos[1], Motor.Target[0],Motor.Target[1], Motor.Vel[0], Motor.Vel[1], Motor.Acc[0],Motor.Acc[1],Motor.Distance);
   return 0;
}
int Cmd_Gcode_G1(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   uint8_t i;
   Motor.Acc_Step[0] = (Motor.Total_Vel*Motor.Total_Vel)/(2*Motor.Total_Acc);
   Motor.Dec_Step[0] = (Motor.Total_Vel*Motor.Total_Vel)/(2*Motor.Total_Dec)+Motor.Total_Vel/20;
   UART_ETHprintf(DEBUG_MSG,"step to acc=%d dec=%d\r\n",Motor.Acc_Step[0],Motor.Dec_Step[0]);
   if(argc>1) {
      for(i=0;i<NUM_AXES;i++)
         Motor.Target[i] = ustrtof(argv[1+i],NULL)*100;
      Delta    ( &Motor );
      Distance ( &Motor );
        UART_ETHprintf(DEBUG_MSG,"distance=%f\r\n",Motor.Distance);
      Vel   ( &Motor );
      Accel ( &Motor );
         Motor.Vel[0]+=20;Motor.Vel[1]+=20; //le pongo un poco mas de vel para que no limite
      Set_Max_Speed ( Motor.Vel );
         Motor.Vel[0]-=20;Motor.Vel[1]-=20; //vuelvo al valor correcto
      Abs_Pos ( Motor.Pos            );
      Set_Acc ( Motor.Acc            );
      Set_Dec ( Motor.Dec            );
      if((Motor.Acc_Step[0]+Motor.Dec_Step[0])<Motor.Distance) {
         UART_ETHprintf(DEBUG_MSG,"run\r\n");
         Run     ( Motor.Dir, Motor.Vel );
         while(Loop_Til_Target())
            ;
      }
      UART_ETHprintf(DEBUG_MSG,"goto\r\n");
      Goto(Motor.Target);
      for(i=0;i<NUM_AXES;i++)
         Motor.Pos[i]=Motor.Target[i];

      //   UART_ETHprintf(DEBUG_MSG,"target\r\n");
   }
  return 0;
}

bool Loop_Til_Target(void)
{
   Abs_Pos  ( Motor.Pos );
   Delta    ( &Motor    );
   Distance ( &Motor    );
//   UART_ETHprintf(DEBUG_MSG,"distance=%f\r\n",Motor.Distance);
   if(Motor.Distance <  Motor.Dec_Step[0])
      return 0;
//   Vel     ( &Motor               );
//   Run     ( Motor.Dir, Motor.Vel );
   UART_ETHprintf(DEBUG_MSG,"pos_x=%d pos_y=%d\r\n",Motor.Pos[0], Motor.Pos[1]);
   vTaskDelay( pdMS_TO_TICKS(10) );
   return 1;
}
int Cmd_Gcode_F(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   if(argc>1) {
      Motor.Total_Vel = ustrtof ( argv[1],NULL );
      UART_ETHprintf(DEBUG_MSG,"New feed=%f\r\n",Motor.Total_Vel);
   }
  return 0;
}
int Cmd_Gcode_Ramps(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   if(argc>1) {
      Motor.Total_Acc = ustrtof ( argv[1],NULL );
      Motor.Total_Dec = ustrtof ( argv[2],NULL );
      UART_ETHprintf(DEBUG_MSG,"New Ramps Acc=%f Dec=%f\r\n",Motor.Total_Acc,Motor.Total_Dec);
   }
  return 0;
}

