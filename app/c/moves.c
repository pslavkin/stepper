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

Motor_t Motor={0};

uint32_t Delta(uint32_t Actual, uint32_t Next, uint32_t* Dir)
{
   uint32_t Ans;
   if(Actual>=Next) {
      Ans=Actual-Next;
      *Dir=1;
   }
   else {
      Ans=Next-Actual;
      *Dir=0;
   }
   return Ans;
}

int Cmd_Gcode_Print_Motor(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   UART_ETHprintf(tpcb,"X=%d Y=%d Next_X=%d Next_Y=%d Vel_X=%f Vel_Y=%f Distance=%f \r\n",
         Motor.Pos[0], Motor.Pos[1], Motor.Target[0],Motor.Target[1], Motor.Vel[0], Motor.Vel[1], Motor.Distance);
   return 0;
}
int Cmd_Gcode_G1(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   uint8_t i;
   Motor.Total_Vel=1000;
   if(argc>1) {
      for(i=0;i<NUM_AXES;i++) {
         Motor.Target[i] = atoi(argv[1+i]);
         Motor.Delta[i]  = Delta(Motor.Pos[i],Motor.Target[i],&Motor.Dir[i]);
      }
      Motor.Distance=sqrt((uint64_t)Motor.Delta[0]*Motor.Delta[0]+
                          (uint64_t)Motor.Delta[1]*Motor.Delta[1]);
      for(i=0;i<NUM_AXES;i++)
         Motor.Vel[i]=Motor.Delta[i]*Motor.Total_Vel/Motor.Distance;
      Set_Max_Speed ( Motor.Vel    );
      Goto          ( Motor.Target );
      for(i=0;i<NUM_AXES;i++)
         Motor.Pos[i]=Motor.Target[i];
   }
   return 0;
}

