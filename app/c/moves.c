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

Motor_t GMotor={0}; // global motor
Motor_t QMotor={0}; // Queue motor
Motor_t AMotor;     // es la que se esta ejecutando en cada instante..
bool Stop_Now=false;
QueueHandle_t Moves_Queue;

void Target2Actual_Target(Motor_t* M)
{
   uint8_t i;
   for(i=0;i<NUM_AXES;i++)
      M->Actual_Target[i]=M->Target[i]/GMotor.Scale[i];
}

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
         M->Delta[i]        = M->Pos[i]-M->Target[i];
         M->Actual_Delta[i] = M->Actual_Pos[i]-M->Actual_Target[i];
      }
      else {
         M->Delta[i]        = M->Target[i]-M->Pos[i];
         M->Actual_Delta[i] = M->Actual_Target[i]-M->Actual_Pos[i];
      }
     // UART_ETHprintf(UART_MSG,"delta=%d\n",M->Delta[i]);
   }
}
void Acc_Dec_Steps(Motor_t* M)
{
   uint8_t i;
   for(i=0;i<NUM_AXES;i++) {
      M->Acc_Step[i]= (MICROSTEP*M->Vel[i]*M->Vel[i])/(2*M->Acc[i]+1);
      M->Dec_Step[i]= (MICROSTEP*M->Vel[i]*M->Vel[i])/(2*M->Dec[i]+1)+(M->Vel[i]*MICROSTEP*0.05);
//      UART_ETHprintf(UART_MSG,"step to acc=%d dec=%d\n",M->Acc_Step[i],M->Dec_Step[i]);
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
   M->Actual_Distance=sqrt(M->Actual_Delta[0]*M->Actual_Delta[0]+
                           M->Actual_Delta[1]*M->Actual_Delta[1]+
                           M->Actual_Delta[2]*M->Actual_Delta[2]);
}


void Vel(Motor_t* M)
{
   uint8_t i;
   for(i=0;i<NUM_AXES;i++) {
      if(M->Delta[i]==0)
         M->Vel[i]=0;
      else {
         M->Vel[i]=(float)M->Actual_Delta[i]*(M->Max_Vel[i]/M->Actual_Distance);
         if(M->Vel[i]<0.015) M->Vel[i]=0.015;
      }
   }
}
void Accel(Motor_t* M)
{
   uint8_t i;
   for(i=0;i<NUM_AXES;i++) {
      M->Acc[i]=M->Vel[i]*(GMotor.Max_Acc[i]/M->Max_Vel[i]);
      if(M->Acc[i]==0) M->Acc[i]=1;
      M->Dec[i]=M->Vel[i]*(GMotor.Max_Dec[i]/M->Max_Vel[i]);
      if(M->Dec[i]==0) M->Dec[i]=1;
   }
}

int Cmd_Get_Queue_Space(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   UART_ETHprintf(tpcb,"space=%d\n",uxQueueSpacesAvailable(Moves_Queue));
   return 0;
}
 int Cmd_Halt_GCode_Queue(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   xQueueReset(Moves_Queue);            // vacio la cola de comandos
   UART_ETHprintf(tpcb,"ok\n");
   Stop_Now=true;                      // aviso para que el sistema de control para donde sea que este y luego el mismo cambia el estado de este flag
   vTaskDelay ( pdMS_TO_TICKS(200 )); // espero a que termine de moverrse
   Stop_Now=false;
   Stop();                              // mano un stop por si acaso...podria haber estado moviemndose
   while(Busy_Read()==0)
      vTaskDelay ( pdMS_TO_TICKS(20 )); // espero a que termine de moverrse
   Abs_Pos  ( AMotor.Pos            );  // tomo la posicion donde quedo
   AMotor.Target[0] = AMotor.Pos[0];    // mq eudo con la posicion del motor actual
   AMotor.Target[1] = AMotor.Pos[1];
   AMotor.Target[2] = AMotor.Pos[2];
   Target2Actual_Target ( &AMotor ) ;   // y tambien la actual posicion (es es en mm)
   AMotor.Line_Number = 0;
   QMotor             = AMotor       ;  // y con esto sincronizo la cola de comandos con el motor
   return 0;
}
 int Cmd_Actual_GCode_Line(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   UART_ETHprintf(tpcb,"line=%d\n",AMotor.Line_Number);
   return 0;
}
 int Cmd_Kombo_Data(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   int32_t  Pos[ NUM_AXES ];
   float    V  [ NUM_AXES ];
   Abs_Pos ( Pos );
   Speed   ( V   );
   UART_ETHprintf(tpcb,"%d %d %d %f %f %f %d %d\n",
         Pos[ 0 ],Pos[ 1 ],Pos[ 2 ],
         V  [ 0 ],V  [ 1 ],V  [ 2 ],
         uxQueueSpacesAvailable(Moves_Queue),
         AMotor.Line_Number);
   return 0;
}
int Cmd_Gcode_Print_Motor(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   Print_Motor_t(&AMotor);
   return 0;
}

int Cmd_Gcode_GL(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   uint8_t i;
   bool Change=false;
   QMotor.Pos[0]        = QMotor.Target[0];
   QMotor.Pos[1]        = QMotor.Target[1];
   QMotor.Pos[2]        = QMotor.Target[2];
   QMotor.Actual_Pos[0] = QMotor.Actual_Target[0];
   QMotor.Actual_Pos[1] = QMotor.Actual_Target[1];
   QMotor.Actual_Pos[2] = QMotor.Actual_Target[2];
   QMotor.Max_Acc[0]    = GMotor.Max_Acc[0];
   QMotor.Max_Acc[1]    = GMotor.Max_Acc[1];
   QMotor.Max_Acc[2]    = GMotor.Max_Acc[2];
   QMotor.Max_Dec[0]    = GMotor.Max_Dec[0];
   QMotor.Max_Dec[1]    = GMotor.Max_Dec[1];
   QMotor.Max_Dec[2]    = GMotor.Max_Dec[2];
//   QMotor.Command=2; //comando invalido


   for(i=1;i<argc;i++) {
      switch(argv[i][0]) {
         case 'N':
            QMotor.Line_Number=atoi(argv[i]+1);
            Change=true;
            break;
         case 'G':
            QMotor.Command=atoi(argv[i]+1);
            break;
         case 'X':
            QMotor.Actual_Target[0] = ustrtof(argv[i]+1,NULL);
            QMotor.Target[0]        = QMotor.Actual_Target[0]*GMotor.Scale[0];
            break;
         case 'Y':
            QMotor.Actual_Target[1] = ustrtof(argv[i]+1,NULL);
            QMotor.Target[1]        = QMotor.Actual_Target[1]*GMotor.Scale[1];
            break;
         case 'Z':
            QMotor.Actual_Target[2] = ustrtof(argv[i]+1,NULL);
            QMotor.Target[2]        = QMotor.Actual_Target[2]*GMotor.Scale[2];
            break;
         case 'F':
            Set_Max_Vel(&QMotor,ustrtof(argv[i]+1,NULL)/60); //la vel viene x minuto.. yo quiero por segundo
            break;
         default:
            break;
      }
   }
   if(Change) {
      //Print_Motor_t(QMotor);
      Dir           ( &QMotor    );
      Delta         ( &QMotor    );
      Distance      ( &QMotor    );
      Vel           ( &QMotor    );
      Accel         ( &QMotor    );
      xQueueSend(Moves_Queue,&QMotor,portMAX_DELAY);
   }
   UART_ETHprintf(tpcb,"ok\n");
  return 0;
}
void Set_Max_Vel(Motor_t* M,float Vel)
{
   uint8_t i;
   for(i=0;i<NUM_AXES;i++)
      M->Max_Vel[i]=(Vel*GMotor.Scale[i])/MICROSTEP; //la vel esta en pasos no micropasos
}

void Print_Motor_t(Motor_t* M)
{/*{{{*/
      UART_ETHprintf(UART_MSG,
            "N=%d "
            "Command=%d \n"
            "TargetX=%d "
            "TargetY=%d "
            "TargetZ=%d \n"
            "DeltaX=%d "
            "DeltaY=%d "
            "DeltaZ=%d \n"
            "Distance=%f \n"
            "Max VelX=%f "
            "Max VelY=%f "
            "Max VelZ=%f \n"
           "Actual DeltaX=%f "
           "Actual DeltaY=%f "
           "Actual DeltaZ=%f \n"
           "Actual Distance=%f \n"
           "AccelX=%f "
           "AccelY=%f "
           "AccelZ=%f \n"
           "DecX=%f "
           "DecY=%f "
           "DecZ=%f \n"
           "Step_AccX=%d "
           "Step_AccY=%d "
           "Step_AccZ=%d \n"
           "Step_DecX=%d "
           "Step_DecY=%d "
           "Step_DecZ=%d "
//            "PosX=%d "
//            "PosY=%d "
//            "PosZ=%d \n"
//            "SpeedX=%f "
//            "SpeedY=%f "
//            "SpeedZ=%f \n"
            "VelX=%f "
            "VelY=%f "
            "VelZ=%f \n"
            "",
            M->Line_Number,
            M->Command,
            M->Target[0],
            M->Target[1],
            M->Target[2],
            M->Delta[0],
            M->Delta[1],
            M->Delta[2],
            M->Distance,
            M->Max_Vel[0],
            M->Max_Vel[1],
            M->Max_Vel[2],
            M->Actual_Delta[0],
            M->Actual_Delta[1],
            M->Actual_Delta[2],
            M->Actual_Distance,
            M->Acc[0],
            M->Acc[1],
            M->Acc[2],
            M->Dec[0],
            M->Dec[1],
            M->Dec[2],
            M->Acc_Step[0],
            M->Acc_Step[1],
            M->Acc_Step[2],
            M->Dec_Step[0],
            M->Dec_Step[1],
            M->Dec_Step[2],
//            M->Pos[0],
//            M->Pos[1],
//            M->Pos[2],
//            M->Speed[0],
//            M->Speed[1],
//            M->Speed[2],
            M->Vel[0],
            M->Vel[1],
            M->Vel[2]
            );
}/*}}}*/

int Cmd_Gcode_Ramps(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   if(argc>1) {
      Set_Acc_Dec_Ramp(&GMotor,ustrtof ( argv[1],NULL ),ustrtof ( argv[2],NULL ));
   }
      UART_ETHprintf(UART_MSG,"New Ramps Acc X=%f Y=%f Z=%f\n Dec X=%f Y=%f Z=%f\n",
            GMotor.Max_Acc[0],
            GMotor.Max_Acc[1],
            GMotor.Max_Acc[2],
            GMotor.Max_Dec[0],
            GMotor.Max_Dec[1],
            GMotor.Max_Dec[2]
            );
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
   Init_Powerstep();
   Moves_Queue= xQueueCreate(MOVES_QUEUE_SIZE,sizeof(Motor_t));
   GMotor.Scale[0]=X_SCALE;
   GMotor.Scale[1]=Y_SCALE;
   GMotor.Scale[2]=Z_SCALE;
   Set_Acc_Dec_Ramp ( &GMotor , 700 , 700 );
   Set_Max_Vel      ( &GMotor ,100        );
   AMotor.Line_Number=0;
   AMotor.Command=2; //invalido al inicio

   float Aux_Vel[NUM_AXES];
   while(1) {
begin:xQueueReceive(Moves_Queue,&AMotor,portMAX_DELAY);
      if(AMotor.Command==0 || AMotor.Command==1)
      {
         Set_Acc       ( AMotor.Acc );
         Set_Dec       ( AMotor.Dec );
         Get_Acc       ( AMotor.Acc );
         Get_Dec       ( AMotor.Dec );
         Acc_Dec_Steps ( &AMotor    );

         Aux_Vel[0]=AMotor.Vel[0]+20;   //le pongo un poco mas de vel para que no limite
         Aux_Vel[1]=AMotor.Vel[1]+20;
         Aux_Vel[2]=AMotor.Vel[2]+20;
         Set_Max_Speed ( Aux_Vel );
         if(Run_Or_Goto(&AMotor)==false) {
            Run     ( AMotor.Dir, AMotor.Vel );
            Clear_Goto(&AMotor);
            while(Busy_Read()==0) {
               if(Stop_Now) {
                 Stop_Now=false;
                 goto begin;
               }
               vTaskDelay ( pdMS_TO_TICKS(20 ));
            }
            while(Time2Goto(&AMotor)==false) {
               Abs_Pos    ( AMotor.Pos       ) ;
               Delta      ( &AMotor          ) ;
               vTaskDelay ( pdMS_TO_TICKS(20 ));
               if(Stop_Now) {
                 Stop_Now=false;
                 goto begin;
               }
            }
         }
         Goto(AMotor.Target);
         while(Busy_Read()==0) {
            vTaskDelay ( pdMS_TO_TICKS(20 ));
               if(Stop_Now) {
                 Stop_Now=false;
                 goto begin;
               }
         }
      }
   }
}


