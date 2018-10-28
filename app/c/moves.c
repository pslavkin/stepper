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

Motor_t GMotor={0};
QueueHandle_t Moves_Queue;

void Begin_Dir(Motor_t* M)
{
   uint8_t i;
   for(i=0;i<NUM_AXES;i++) {
      if(M->Pos[i]>=M->Target[i]) {
         M->Begin_Dir[i]   = 0;
      }
      else {
         M->Begin_Dir[i]   = 1;
      }
   }
}
void Actual_Dir(Motor_t* M)
{
   uint8_t i;
   for(i=0;i<NUM_AXES;i++) {
      if(M->Pos[i]>=M->Target[i]) {
         M->Actual_Dir[i]   = 0;
      }
      else {
         M->Actual_Dir[i]   = 1;
      }
   }
}
void Delta(Motor_t* M)
{
   uint8_t i;
   for(i=0;i<NUM_AXES;i++) {
      if(M->Pos[i]>=M->Target[i]) {
         M->Delta[i] = M->Pos[i]-M->Target[i];
      }
      else {
         M->Delta[i] = M->Target[i]-M->Pos[i];
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
   UART_ETHprintf(tpcb,"V=%f Acc=%f Dec=%f\r\n",
         GMotor.Total_Vel, GMotor.Total_Acc, GMotor.Total_Dec);
   return 0;
}
int Cmd_Gcode_G1(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   uint8_t i;
   Motor_t Motor;
   Motor.Total_Vel = GMotor.Total_Vel;
   Motor.Total_Acc = GMotor.Total_Acc;
   Motor.Total_Dec = GMotor.Total_Dec;
   if(argc>1) {
      for(i=0;i<NUM_AXES;i++)
         Motor.Target[i] = ustrtof(argv[1+i],NULL);
      xQueueSend(Moves_Queue,&Motor,portMAX_DELAY);
   }
  return 0;
}

int Cmd_Gcode_F(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   if(argc>1) {
      GMotor.Total_Vel = ustrtof ( argv[1],NULL );
      UART_ETHprintf(DEBUG_MSG,"New feed=%f\r\n",GMotor.Total_Vel);
   }
  return 0;
}
int Cmd_Gcode_Ramps(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   if(argc>1) {
      GMotor.Total_Acc = ustrtof ( argv[1],NULL );
      GMotor.Total_Dec = ustrtof ( argv[2],NULL );
      UART_ETHprintf(DEBUG_MSG,"New Ramps Acc=%f Dec=%f\r\n",GMotor.Total_Acc,GMotor.Total_Dec);
   }
  return 0;
}

void Moves_Parser(void* nil)
{
   Moves_Queue= xQueueCreate(MOVES_QUEUE_SIZE,sizeof(Motor_t));
   GMotor.Total_Vel = 100;
   GMotor.Total_Acc = 100;
   GMotor.Total_Dec = 100;
   Motor_t Motor;
   while(1) {
      while(xQueueReceive(Moves_Queue,&Motor,portMAX_DELAY)!=pdTRUE)
         ;
   Motor.Acc_Step[0] = (128*Motor.Total_Vel*Motor.Total_Vel)/(2*Motor.Total_Acc); //sale de que Vf^2=V0^2+2*a*X
   Motor.Dec_Step[0] = (128*Motor.Total_Vel*Motor.Total_Vel)/(2*Motor.Total_Dec)+Motor.Total_Vel*128/10;
   UART_ETHprintf(UART_MSG,"step to acc=%d dec=%d\r\n",Motor.Acc_Step[0],Motor.Dec_Step[0]);


      Abs_Pos   ( Motor.Pos );
      Begin_Dir ( &Motor    );
      Delta     ( &Motor    );
      Distance  ( &Motor    );
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
         Run     ( Motor.Begin_Dir, Motor.Vel );
         uint8_t Ans=1;
         while(Ans==1) {
            Abs_Pos    ( Motor.Pos );
            Actual_Dir ( &Motor    );
            Delta      ( &Motor    );
            Distance   ( &Motor    );
            if ( Motor.Distance <  Motor.Dec_Step[0] ||
                  Motor.Actual_Dir[0]!=Motor.Begin_Dir[0] ||
                  Motor.Actual_Dir[1]!=Motor.Begin_Dir[1])
               Ans=0;
            UART_ETHprintf(UART_MSG,"pos_x=%d pos_y=%d\r\n",Motor.Pos[0], Motor.Pos[1]);
            vTaskDelay( pdMS_TO_TICKS(10) );
         }
      }
      while(Busy_Read()==0)
         vTaskDelay ( pdMS_TO_TICKS(10 ));
      UART_ETHprintf(UART_MSG,"goto\r\n");
      Goto(Motor.Target);
      while(Motor.Pos[0]!=Motor.Target[0] ||
            Motor.Pos[1]!=Motor.Target[1]) {
            Abs_Pos ( Motor.Pos   );
            Speed   ( Motor.Speed );
            UART_ETHprintf(UART_MSG,"pos_x=%d pos_y=%d vel_x=%f vel_y=%f\r\n",Motor.Pos[0], Motor.Pos[1],Motor.Speed[0],Motor.Speed[1]);
            vTaskDelay( pdMS_TO_TICKS(10) );
      }
      UART_ETHprintf(DEBUG_MSG,"target\r\n");
   }
}


