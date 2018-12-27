#ifndef  MOVES
#define  MOVES

typedef struct
{
   uint32_t Line_Number                   ;
   uint8_t  Command                       ;
   float    Gcode_Vel                     ;
   float    Scaled_Vel                    ;
   float    Restringed_GCode_Vel          ;
   int16_t  Limited_Vel                   ;
   uint8_t  Speed_Scale                   ;
   uint32_t Minor_Delay2Goto              ;
   float    Actual_Distance               ;
   float    Max_Vel           [ NUM_AXES ];
   float    Max_Acc           [ NUM_AXES ];
   float    Max_Dec           [ NUM_AXES ];
   int32_t  Pos               [ NUM_AXES ];
   float    Actual_Pos        [ NUM_AXES ];
   int32_t  Target            [ NUM_AXES ];
   float    Actual_Target     [ NUM_AXES ];
   int32_t  Delta             [ NUM_AXES ];
   float    Actual_Delta      [ NUM_AXES ];
   uint32_t Dec_Step          [ NUM_AXES ];
   uint32_t Acc_Step          [ NUM_AXES ];
   uint8_t  Dir               [ NUM_AXES ];
   float    Vel               [ NUM_AXES ];
   float    Restringed_Max_Vel[ NUM_AXES ];
   float    Vel4Acc_Limit     [ NUM_AXES ];
   float    Acc               [ NUM_AXES ];
   float    Dec               [ NUM_AXES ];
} Motor_t                                 ;

extern QueueHandle_t Moves_Queue;
extern Motor_t       GMotor;

void     Moves_Parser          ( void* nil                                         );
int      Cmd_Gcode_GL          ( struct tcp_pcb* tpcb, int argc, char *argv[]      );
int      Cmd_Gcode_F           ( struct tcp_pcb* tpcb, int argc, char *argv[]      );
int      Cmd_Gcode_Print_Motor ( struct tcp_pcb* tpcb, int argc, char *argv[]      );
int      Cmd_Gcode_Ramps       ( struct tcp_pcb* tpcb, int argc, char *argv[]      );
int      Cmd_Get_Queue_Space   ( struct tcp_pcb* tpcb, int argc, char *argv[]      );
void     Print_Motor_t         ( struct tcp_pcb* tpcb,Motor_t* M                   );
void     Set_Max_Vel           ( Motor_t* M,float Vel,uint16_t Limit,uint8_t Scale );
void     Set_Acc_Dec_Ramp      ( Motor_t* M,float Acc, float Dec                   );
int      Cmd_Halt_GCode_Queue  ( struct tcp_pcb* tpcb, int argc, char *argv[]      );
int      Cmd_Actual_GCode_Line ( struct tcp_pcb* tpcb, int argc, char *argv[]      );
int      Cmd_Kombo_Data        ( struct tcp_pcb* tpcb, int argc, char *argv[]      );
int      Cmd_Limited_Speed     ( struct tcp_pcb* tpcb, int argc, char *argv[]      );
int      Cmd_Speed_Scale       ( struct tcp_pcb* tpcb, int argc, char *argv[]      );
void     Target2Actual_Target  ( Motor_t* M                                        );
void     Limit_Max_Vel         ( Motor_t* M                                        );
int      Cmd_Pause             ( struct tcp_pcb* tpcb, int argc, char *argv[]      );
int      Cmd_Resume            ( struct tcp_pcb* tpcb, int argc, char *argv[]      );
void     Restringed_Vel        ( Motor_t* M                                        );
int      Cmd_Rst_Z             ( struct tcp_pcb* tpcb, int argc, char *argv[]      );
int      Cmd_Rst_Xy            ( struct tcp_pcb* tpcb, int argc, char *argv[]      );

#endif

