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

Motor_t GMotor={0}; //global motor
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
     // UART_ETHprintf(UART_MSG,"delta=%d\r\n",M->Delta[i]);
   }
}
void Acc_Dec_Steps(Motor_t* M)
{
   uint8_t i;
   for(i=0;i<NUM_AXES;i++) {
      M->Acc_Step[i]= (MICROSTEP*M->Vel[i]*M->Vel[i])/(2*M->Acc[i]);
      M->Dec_Step[i]= (MICROSTEP*M->Vel[i]*M->Vel[i])/(2*M->Dec[i])+(M->Vel[i]*MICROSTEP*0.05);
//      UART_ETHprintf(UART_MSG,"step to acc=%d dec=%d\r\n",M->Acc_Step[i],M->Dec_Step[i]);
   }
}

void Clear_Goto(Motor_t* M)
{
   uint8_t i;
   for(i=0;i<NUM_AXES;i++)
      M->Goto[i]=0;
}

bool Time2Goto(Motor_t* M)
{
   uint8_t i;
   uint8_t Ans=0;
   for(i=0;i<NUM_AXES;i++) {
      if(M->Dec_Step[i]>=M->Delta[i])
         M->Goto[i]=1;
      Ans+=M->Goto[i];
   }
   return Ans==NUM_AXES;
}
bool Run_Or_Goto(Motor_t* M)
{
   uint8_t i,Goto=0;
   for(i=0;i<NUM_AXES ;i++) {
      if((M->Acc_Step[i]+M->Dec_Step[i])>=M->Delta[i])
         Goto++;
   }
   return Goto==NUM_AXES;
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
      M->Vel[i]=(float)M->Delta[i]*(M->Max_Vel[i]/M->Distance);
 //  M->Vel[2]/=4.3;
}
void Accel(Motor_t* M)
{
   uint8_t i;
   for(i=0;i<NUM_AXES;i++) {
      M->Acc[i]=M->Vel[i]*(GMotor.Max_Acc[i]/M->Max_Vel[i]);
      M->Dec[i]=M->Vel[i]*(GMotor.Max_Dec[i]/M->Max_Vel[i]);
   }
}

int Cmd_Get_Queue_Space(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   UART_ETHprintf(tpcb,"space=%d\r\n",uxQueueSpacesAvailable(Moves_Queue));
   return 0;
}

int Cmd_Gcode_Print_Motor(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   Print_Motor_t(&GMotor);
   return 0;
}


int Cmd_Gcode_GL(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   static Motor_t Motor;
   uint8_t i;
   bool Change=false;
   for(i=1;i<argc;i++) {
      switch(argv[i][0]) {
         case 'N':
            Motor.Line_Number=atoi(argv[i]+1);
            break;
         case 'G':
            Motor.Command=atoi(argv[i]+1);
            break;
         case 'X':
            Motor.Target[0]=(ustrtof(argv[i]+1,NULL)*GMotor.Scale[0]);
            Change=true;
            break;
         case 'Y':
            Motor.Target[1]=(ustrtof(argv[i]+1,NULL)*GMotor.Scale[1]);
            Change=true;
            break;
         case 'Z':
            Motor.Target[2]=(ustrtof(argv[i]+1,NULL)*GMotor.Scale[2]);
            Change=true;
            break;
         case 'F':
            Set_Max_Vel(&Motor,ustrtof(argv[i]+1,NULL));
            break;
         default:
            break;
      }
   }
   if(Change) {
      //Print_Motor_t(Motor);
      xQueueSend(Moves_Queue,&Motor,portMAX_DELAY);
   }
  return 0;
}
void Set_Max_Vel(Motor_t* M,float Vel)
{
   uint8_t i;
   for(i=0;i<NUM_AXES;i++)
      M->Max_Vel[i]=(Vel*GMotor.Scale[i])/MICROSTEP; //la vel esta en pasos no micropasos
}

void Print_Motor_t(Motor_t* M)
{
      UART_ETHprintf(UART_MSG,
//            "N= %d "
//            "Command=%d \r\n"
            "TargetX=%d "
            "TargetY=%d "
            "TargetZ=%d \r\n"
            "VelX=%f "
            "VelY=%f "
            "VelZ=%f \r\n"
            "AccelX=%f "
            "AccelY=%f "
            "AccelZ=%f \r\n"
//            "SpeedX=%f "
//            "SpeedY=%f "
//            "SpeedZ=%f \r\n"
//            "Max VelX=%f "
//            "Max VelY=%f "
//            "Max VelZ=%f \r\n"
            "Step_AccX=%d "
            "Step_AccY=%d "
            "Step_AccZ=%d \r\n"
            "Step_DecX=%d "
            "Step_DecY=%d "
            "Step_DecZ=%d "
            "\r\n",
//            M.Line_Number,
//            M.Command,
            M->Target[0],
            M->Target[1],
            M->Target[2],
            M->Vel[0],
            M->Vel[1],
            M->Vel[2],
            M->Acc[0],
            M->Acc[1],
            M->Acc[2],
//            M.Speed[0],
//            M.Speed[1],
//            M.Speed[2],
//            M.Max_Vel[0],
//            M.Max_Vel[1],
//            M.Max_Vel[2],
            M->Acc_Step[0],
            M->Acc_Step[1],
            M->Acc_Step[2],
            M->Dec_Step[0],
            M->Dec_Step[1],
            M->Dec_Step[2]
               );
}

int Cmd_Gcode_Ramps(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   if(argc>1) {
      Set_Acc_Dec_Ramp(&GMotor,ustrtof ( argv[1],NULL ),ustrtof ( argv[2],NULL ));
      UART_ETHprintf(DEBUG_MSG,"New Ramps Acc X=%f Y=%f Z=%f\r\n Dec X=%f Y=%f Z=%f\r\n",
            GMotor.Max_Acc[0],
            GMotor.Max_Acc[1],
            GMotor.Max_Acc[2],
            GMotor.Max_Dec[0],
            GMotor.Max_Dec[1],
            GMotor.Max_Dec[2]
            );
   }
   return 0;
}

void Set_Acc_Dec_Ramp(Motor_t* M,float Acc, float Dec)
{
   uint8_t i;
   for(i=0;i<NUM_AXES;i++) {
      M->Max_Acc[i] = Acc*GMotor.Scale[i]/MICROSTEP;
      M->Max_Dec[i] = Dec*GMotor.Scale[i]/MICROSTEP;
   }
}

void Moves_Parser(void* nil)
{
   Moves_Queue= xQueueCreate(MOVES_QUEUE_SIZE,sizeof(Motor_t));
   GMotor.Scale[0]=X_SCALE;
   GMotor.Scale[1]=Y_SCALE;
   GMotor.Scale[2]=Z_SCALE;
   Set_Acc_Dec_Ramp(&GMotor,  100,  100);
   Set_Max_Vel(&GMotor,10);

   float Aux_Vel[NUM_AXES];
   Motor_t Motor;
   while(1) {
      while(xQueueReceive(Moves_Queue,&Motor,portMAX_DELAY)!=pdTRUE)
         ;

      Motor.Max_Acc[0]=GMotor.Max_Acc[0];
      Motor.Max_Acc[1]=GMotor.Max_Acc[1];
      Motor.Max_Acc[2]=GMotor.Max_Acc[2];
      Motor.Max_Dec[0]=GMotor.Max_Dec[0];
      Motor.Max_Dec[1]=GMotor.Max_Dec[1];
      Motor.Max_Dec[2]=GMotor.Max_Dec[2];

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
   //   Print_Motor_t(&Motor);
      if(Run_Or_Goto(&Motor)==false) {
  //       UART_ETHprintf(UART_MSG,"run\r\n");
         Run     ( Motor.Dir, Motor.Vel );
         Clear_Goto(&Motor);
         while(Busy_Read()==0)
            vTaskDelay ( pdMS_TO_TICKS(10 ));
//        uint16_t Correction=0;
         while(Time2Goto(&Motor)==false) {
            Abs_Pos  ( Motor.Pos            );
            Delta    ( &Motor               );
            //if(Correction++>10) {
            //   Correction=0;
            //   Dir      ( &Motor               );
            //   Distance ( &Motor               );
            //   Vel      ( &Motor               );
            //   Run      ( Motor.Dir, Motor.Vel );
            //}
            vTaskDelay ( pdMS_TO_TICKS(10 ));
         }
      }
//     UART_ETHprintf(UART_MSG,"goto\r\n");
      Goto(Motor.Target);
      while(Busy_Read()==0)
         vTaskDelay ( pdMS_TO_TICKS(10 ));
 //    UART_ETHprintf(UART_MSG,"target\r\n");
   }
}


