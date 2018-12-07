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
#include "buttons.h"

Motor_t QMotor         = {.Command=2,0}; // Queue motor
Motor_t AMotor;               // es la que se esta ejecutando en cada instante..
uint16_t Limited_Speed = 10;  // en  mm/seg
uint16_t uStep2mm[NUM_AXES]={X_SCALE,Y_SCALE,Z_SCALE};
float Acc_Ramp=1000, Dec_Ramp=1000;
uint32_t Waiting_Line=0;

bool Stop_Now=false;
QueueHandle_t Moves_Queue;

void Target2Actual_Target(Motor_t* M)
{
   uint8_t i;
   for(i=0;i<NUM_AXES;i++)
      M->Actual_Target[i]=M->Target[i]/uStep2mm[i];
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
      M->Acc_Step[i]= ((M->Vel[i]*M->Vel[i])/M->Acc[i])*((float)(MICROSTEP/2));
      M->Dec_Step[i]= ((M->Vel[i]*M->Vel[i])/M->Dec[i])*((float)(MICROSTEP/2))+(M->Vel[i]*MICROSTEP*0.04);
//      UART_ETHprintf(UART_MSG,"step to acc=%d dec=%d\n",M->Acc_Step[i],M->Dec_Step[i]);
   }
}

bool Pre_Time2Goto(Motor_t* M)
{
   uint8_t i;
   bool Ans=false;
   for(i=0;i<NUM_AXES;i++) {
      if(M->Delta[i]<0 || (M->Dec_Step[i]+2000)>M->Delta[i]) {
         Ans=true;
         break;
      }
   }
   return Ans;
}
bool Time2Goto(Motor_t* M)
{
   uint8_t i;
   bool Ans=false;
   for(i=0;i<NUM_AXES;i++) {
      if(M->Delta[i]<0 || M->Dec_Step[i]>M->Delta[i]) {
         Ans=true;
         break;
      }
   }
   return Ans;
}
void Run_Or_Goto(Motor_t* M)
{
   uint8_t i;
   uint8_t Any_Delta=0;
   M->Run_Goto=true; //true es run
   for(i=0;i<NUM_AXES ;i++) {
      if((M->Acc_Step[i]+M->Dec_Step[i]-(M->Vel[i]*MICROSTEP*0.02))>M->Delta[i]) {
         M->Run_Goto=false;
      }
      Any_Delta+=(M->Delta[i]>0)?1:0;
   }
   M->Run_Goto=(M->Run_Goto) && (Any_Delta>0);
//   return M->Run_Goto;

//      UART_ETHprintf(UART_MSG,"acc=%d dec=%d delta=%d run=%d \n",M->Acc_Step[i],M->Dec_Step[i],M->Delta[i],M->Run_Goto);
}
void Distance(Motor_t* M)
{
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
         M->Vel[i]=((float)M->Actual_Delta[i]*M->Max_Vel[i])/M->Actual_Distance;
         if(M->Vel[i]<0.015) M->Vel[i]=0.015;
      }
   }
}
void Accel(Motor_t* M)
{
   uint8_t i;
   uint32_t Acc_Integer;
   //TODO:  si guardo el ultimo paramtro F limpio como vel maxim, el calculo de la aceleracion por canal es mucho mas facil porque sino estoy multiplicando y dividiendo varias veces por step2mm y microstep..
   for(i=0;i<NUM_AXES;i++) {
      M->Acc[i]=M->Vel[i]*((Acc_Ramp*uStep2mm[i]/MICROSTEP)/M->Max_Vel[i]);
      Acc_Integer=M->Acc[i]/14.5519152284+1; //paso a entoreo porque la aceleracion tiene un ste pde 14.55
      M->Acc[i]=Acc_Integer*14.5619152284; //y vuelvo a corregir pero un poquito pasado para que redonde hacia arriba
      M->Dec[i]=M->Vel[i]*((Dec_Ramp*uStep2mm[i]/MICROSTEP)/M->Max_Vel[i]);
      Acc_Integer=M->Dec[i]/14.5519152284+1; //lomismo que para accel
      M->Dec[i]=Acc_Integer*14.5619152284;
//      UART_ETHprintf(UART_MSG,"Acc=%f Dec=%f\n",M->Acc[i],M->Dec[i]);
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
   Waiting_Line       = 0;
   AMotor.Command     = 2;
   QMotor             = AMotor       ;  // y con esto sincronizo la cola de comandos con el motor
   return 0;
}
 int Cmd_Actual_GCode_Line(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   UART_ETHprintf(tpcb,"line=%d\n",AMotor.Line_Number);
   return 0;
}
int Cmd_Limited_Speed(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   if(argc>1)
         Limited_Speed=atoi(argv[1]);
   else
      UART_ETHprintf(tpcb,"limited speed= %d\n",Limited_Speed);
   return 0;
}

 int Cmd_Kombo_Data(struct tcp_pcb* tpcb, int argc, char *argv[])
{
   int32_t  Pos[ NUM_AXES ];
   float    V  [ NUM_AXES ];
   Abs_Pos ( Pos );
   Speed   ( V   );
   UART_ETHprintf(tpcb,"%d %d %d %f %f %f %d %d %d %f %f %d\n",
         Pos[ 0 ],Pos[ 1 ],Pos[ 2 ],
         V  [ 0 ],V  [ 1 ],V  [ 2 ],
         uxQueueSpacesAvailable(Moves_Queue),
         AMotor.Line_Number,
         Waiting_Line,
         Acc_Ramp,
         Dec_Ramp,
         Limited_Speed
         );
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

   for(i=1;i<argc;i++) {
      switch(argv[i][0]) {
         case 'N':
            if(atoi(argv[i]+1)==(Waiting_Line+1)) {
               Waiting_Line++;
               QMotor.Line_Number=Waiting_Line;
               Change=true;
            }
            else return 0;
            break;
         case 'G':
            QMotor.Command=atoi(argv[i]+1);
            break;
         case 'X':
            QMotor.Actual_Target[0] = ustrtof(argv[i]+1,NULL);
            QMotor.Target[0]        = QMotor.Actual_Target[0]*uStep2mm[0];
            break;
         case 'Y':
            QMotor.Actual_Target[1] = ustrtof(argv[i]+1,NULL);
            QMotor.Target[1]        = QMotor.Actual_Target[1]*uStep2mm[1];
            break;
         case 'Z':
            QMotor.Actual_Target[2] = ustrtof(argv[i]+1,NULL);
            QMotor.Target[2]        = QMotor.Actual_Target[2]*uStep2mm[2];
            break;
         case 'F':
            Set_Max_Vel(&QMotor,ustrtof(argv[i]+1,NULL)/30); //la vel viene x minuto.. yo quiero por segundo
            break;
         default:
            break;
      }
   }
   if(Change) {
      Dir           ( &QMotor );
      Delta         ( &QMotor );
      Distance      ( &QMotor );
      Limit_Max_Vel ( &QMotor );
      Vel           ( &QMotor );
      Accel         ( &QMotor );
      Acc_Dec_Steps ( &QMotor );
      Run_Or_Goto   ( &QMotor );
   }
   xQueueSend(Moves_Queue,&QMotor,portMAX_DELAY);
   return 0;
}
void Limit_Max_Vel(Motor_t* M)
{
   if(M->Limited_Vel != Limited_Speed)
      Set_Max_Vel(M,M->Gcode_Vel);
}

void Set_Max_Vel(Motor_t* M,float Vel)
{
   uint8_t i;
   M->Gcode_Vel   = Vel;                          // me acuerdo de la original
   M->Limited_Vel = Limited_Speed;                  // me acuerdo de la original
   if(Vel>Limited_Speed)                            // limio vel
      Vel=Limited_Speed;
   for(i=0;i<NUM_AXES;i++)
      M->Max_Vel[i]=(Vel*uStep2mm[i])/MICROSTEP; // la vel esta en pasos no micropasos
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
            "Max VelX=%f "
            "Max VelY=%f "
            "Max VelZ=%f \n"
           "Actual DeltaX=%f "
           "Actual DeltaY=%f "
           "Actual DeltaZ=%f \n"
           "Actual Distance=%f \n"
           "Step_AccX=%d "
           "Step_AccY=%d "
           "Step_AccZ=%d \n"
           "Step_DecX=%d "
           "Step_DecY=%d "
           "Step_DecZ=%d \n"
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
            M->Max_Vel[0],
            M->Max_Vel[1],
            M->Max_Vel[2],
            M->Actual_Delta[0],
            M->Actual_Delta[1],
            M->Actual_Delta[2],
            M->Actual_Distance,
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
      Acc_Ramp=ustrtof ( argv[1],NULL );
      Dec_Ramp=ustrtof ( argv[2],NULL );
   }
   else
      UART_ETHprintf(UART_MSG,"Ramps Acc=%f Dec=%f\n",Acc_Ramp,Dec_Ramp);
   return 0;
}
void Moves_Parser(void* nil)
{
   Init_Powerstep();
   Moves_Queue= xQueueCreate(MOVES_QUEUE_SIZE,sizeof(Motor_t));
   Set_Max_Vel      ( &AMotor ,10*MICROSTEP );
   Set_Max_Vel      ( &QMotor ,10*MICROSTEP );
   AMotor.Line_Number = 0;
   AMotor.Command     = 2; //invalido al inicio
   float Aux_Vel[NUM_AXES];

   while(1) {
      xQueueReceive(Moves_Queue,&AMotor,portMAX_DELAY);
      if(AMotor.Command==0 || AMotor.Command==1)
      {
         if(AMotor.Limited_Vel != Limited_Speed) {
            Limit_Max_Vel ( &AMotor );
            Vel           ( &AMotor );
            Accel         ( &AMotor );
            Acc_Dec_Steps ( &AMotor );
            Run_Or_Goto   ( &AMotor );
         }
         if(Busy_Read()==0)
            xSemaphoreTake( Busy_Semphr,portMAX_DELAY );

         Set_Acc       ( AMotor.Acc );
         Set_Dec       ( AMotor.Dec );

         Aux_Vel[0]=AMotor.Vel[0]+15.3;   //le pongo un poco mas de vel para que no limite
         Aux_Vel[1]=AMotor.Vel[1]+15.3;
         Aux_Vel[2]=AMotor.Vel[2]+15.3;
         Set_Max_Speed ( Aux_Vel );

         if(AMotor.Run_Goto==true) {
            Run     ( AMotor.Dir, AMotor.Vel );
            if(Busy_Read()==0)
               xSemaphoreTake( Busy_Semphr,portMAX_DELAY );
            Abs_Pos ( AMotor.Pos );
            Delta   ( &AMotor    );
            while   ( Pre_Time2Goto(&AMotor )==false) {
               vTaskDelay ( pdMS_TO_TICKS(20 ));
               Abs_Pos    ( AMotor.Pos       ) ;
               Delta      ( &AMotor          ) ;
            }
            while(Time2Goto(&AMotor)==false) {
               Abs_Pos ( AMotor.Pos );
               Delta   ( &AMotor    );
            }
         }
         if(Busy_Read()==0)                                 //ojo que puede llegar aca sin run, o sea aun esta pendiente el gotyo anterior! hay que esperar busy
            xSemaphoreTake( Busy_Semphr,portMAX_DELAY );
         Goto ( AMotor.Target          );
      }
   }
}


