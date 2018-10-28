#ifndef  MOVES
#define  MOVES

#define MOVES_QUEUE_SIZE 20

typedef struct
{
   float    Total_Vel         ;
   float    Total_Acc         ;
   float    Total_Dec         ;
   float    Distance          ;
    int32_t Pos   [ NUM_AXES ];
    int32_t Target[ NUM_AXES ];
   uint32_t Delta [ NUM_AXES ];
   uint32_t Dec_Step [ NUM_AXES ];
   uint32_t Acc_Step [ NUM_AXES ];
   uint8_t  Actual_Dir  [ NUM_AXES ];
   uint8_t  Begin_Dir   [ NUM_AXES ];
   float    Speed   [ NUM_AXES ];
   float    Vel   [ NUM_AXES ];
   float    Acc   [ NUM_AXES ];
   float    Dec   [ NUM_AXES ];
} Motor_t;

extern QueueHandle_t Moves_Queue;
extern void          Moves_Parser(void* nil);
extern Motor_t GMotor;

struct Moves_Queue_Struct {
   char A;
};


extern int Cmd_Gcode_G1          ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Gcode_F           ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Gcode_Print_Motor ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Gcode_Ramps       ( struct tcp_pcb* tpcb, int argc, char *argv[] );

#endif

