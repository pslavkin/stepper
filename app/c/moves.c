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

void Dir(Motor_t* M)
{
   uint8_t i;
   for(i=0;i<NUM_AXES;i++) {
      if(M->Pos[i]>=M->Target[i]) {
         M->Dir[i]   = 0;
      }
      else {
         M->Dir[i]   = 1;
      }
   }
}
void Delta(Motor_t* M)
{
   uint8_t i;
   for(i=0;i<NUM_AXES;i++) {
      if(M->Dir[i]==0) {
         M->Delta[i] = M->Pos[i]-M->Target[i];
      }
      else {
         M->Delta[i] = M->Target[i]-M->Pos[i];
      }
    //  UART_ETHprintf(UART_MSG,"delta=%d\r\n",M->Delta[i]);
   }
}
void Acc_Dec_Steps(Motor_t* M)
{
   uint8_t i;
   for(i=0;i<NUM_AXES;i++) {
      M->Acc_Step[i]= (128*M->Vel[i]*M->Vel[i])/(2*M->Acc[i]);
      M->Dec_Step[i]= (128*M->Vel[i]*M->Vel[i])/(2*M->Dec[i])+(M->Vel[i]*128*0.05);
   //   UART_ETHprintf(UART_MSG,"step to acc=%d dec=%d\r\n",M->Acc_Step[i],M->Dec_Step[i]);
   }
}
bool Time2Goto(Motor_t* M)
{
   uint8_t i,Goto=0;
   for(i=0;i<NUM_AXES;i++) {
      if(M->Dec_Step[i]>=M->Delta[i])
         Goto++;
   }
   return Goto==3;
}
bool Run_Or_Goto(Motor_t* M)
{
   uint8_t i,Goto=0;
   for(i=0;i<NUM_AXES ;i++) {
      if((M->Acc_Step[i]+M->Dec_Step[i])>=M->Delta[i])
         Goto++;
   }
   return Goto==3;
}
void Distance(Motor_t* M)
{
   M->Distance=sqrt((uint64_t)M->Delta[0]*M->Delta[0]+
                    (uint64_t)M->Delta[1]*M->Delta[1]+
                    (uint64_t)M->Delta[2]*M->Delta[2]);
}
void Vel(Motor_t* M)
{
   uint8_t i;
   for(i=0;i<NUM_AXES;i++)
      M->Vel[i]=(float)M->Delta[i]*(M->Total_Vel/M->Distance);
}
void Accel(Motor_t* M)
{
   uint8_t i;
   for(i=0;i<NUM_AXES;i++) {
      M->Acc[i]=M->Vel[i]*(M->Total_Acc/M->Total_Vel);
      M->Dec[i]=M->Vel[i]*(M->Total_Dec/M->Total_Vel);
   }
}

int Cmd_Get_Queue_Space(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   UART_ETHprintf(tpcb,"space=%d\r\n",uxQueueSpacesAvailable(Moves_Queue));
   return 0;
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
   GMotor.Total_Vel = 600;
   GMotor.Total_Acc = 1000;
   GMotor.Total_Dec = 1000;
   float Aux_Vel[NUM_AXES];
   Motor_t Motor;
   while(1) {
      while(xQueueReceive(Moves_Queue,&Motor,portMAX_DELAY)!=pdTRUE)
         ;
//      UART_ETHprintf(UART_MSG,"step to acc=%d dec=%d\r\n",Motor.Acc_Step[0],Motor.Dec_Step[0]);

      Abs_Pos       ( Motor.Pos );
      Dir           ( &Motor    );
      Delta         ( &Motor    );
      Distance      ( &Motor    );
      Vel           ( &Motor    );
      Accel         ( &Motor    );
      Set_Acc       ( Motor.Acc );
      Set_Dec       ( Motor.Dec );
      Get_Acc       ( Motor.Acc );
      Get_Dec       ( Motor.Dec );
      Acc_Dec_Steps ( &Motor    );

      Aux_Vel[0]=Motor.Vel[0]+20;   //le pongo un poco mas de vel para que no limite
      Aux_Vel[1]=Motor.Vel[1]+20;
      Aux_Vel[2]=Motor.Vel[2]+20;
      Set_Max_Speed ( Aux_Vel );
//         Aux_Vel[0]=10; //Motor.Vel[0]-100;
//         Aux_Vel[1]=10; //Motor.Vel[1]-100;
//         Aux_Vel[2]=10; //Motor.Vel[2]-100;
//         Set_Min_Speed ( Aux_Vel );

      if(Run_Or_Goto(&Motor)==false) {
//         UART_ETHprintf(UART_MSG,"run\r\n");
         Run     ( Motor.Dir, Motor.Vel );
         while(Busy_Read()==0)
            vTaskDelay ( pdMS_TO_TICKS(10 ));
         while(Time2Goto(&Motor)==false) {
            Abs_Pos    ( Motor.Pos        ) ;
            Delta      ( &Motor           ) ;
            vTaskDelay ( pdMS_TO_TICKS(10 ));
         }
      }
 //     UART_ETHprintf(UART_MSG,"goto\r\n");
      Goto(Motor.Target);
      while(Busy_Read()==0)
         vTaskDelay ( pdMS_TO_TICKS(10 ));
 //     UART_ETHprintf(UART_MSG,"target\r\n");
   }
}


