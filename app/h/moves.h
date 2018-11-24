#ifndef  MOVES
#define  MOVES

#define MOVES_QUEUE_SIZE 20
#define X_SCALE          600  // micropasos por mm
#define Y_SCALE          600  // micropasos por mm
#define Z_SCALE          2560 // micropasos por mm
#define MICROSTEP        128

typedef struct
{
   uint32_t Line_Number;
   uint8_t  Command;
   float    Max_Vel[NUM_AXES];
   float    Max_Acc[NUM_AXES];
   float    Max_Dec[NUM_AXES];
   float    Distance          ;
    int32_t Pos     [ NUM_AXES ];
    int32_t Target  [ NUM_AXES ];
   uint32_t Delta   [ NUM_AXES ];
   uint32_t Dec_Step[ NUM_AXES ];
   uint32_t Acc_Step[ NUM_AXES ];
   uint8_t  Dir     [ NUM_AXES ];
   float    Speed   [ NUM_AXES ];
   float    Vel     [ NUM_AXES ];
   float    Acc     [ NUM_AXES ];
   float    Dec     [ NUM_AXES ];
   float    Scale   [ NUM_AXES ];
   uint8_t  Goto   [ NUM_AXES ];
} Motor_t;

extern QueueHandle_t Moves_Queue;
extern void          Moves_Parser(void* nil);
extern Motor_t GMotor;

struct Moves_Queue_Struct {
   char A;
};


extern int     Cmd_Gcode_GL          ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int     Cmd_Gcode_F           ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int     Cmd_Gcode_Print_Motor ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int     Cmd_Gcode_Ramps       ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int     Cmd_Get_Queue_Space   ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern void    Print_Motor_t         ( Motor_t* M                                    );
void Set_Max_Vel(Motor_t* M,float Vel);
void Set_Acc_Dec_Ramp(Motor_t* M,float Acc, float Dec);
void Clear_Goto(Motor_t* M);

#endif

