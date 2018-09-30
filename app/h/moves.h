#ifndef  MOVES
#define  MOVES

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
   uint8_t  Dir   [ NUM_AXES ];
   float    Vel   [ NUM_AXES ];
   float    Acc   [ NUM_AXES ];
   float    Dec   [ NUM_AXES ];
} Motor_t;

extern int Cmd_Gcode_G1          ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Gcode_F           ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Gcode_Print_Motor ( struct tcp_pcb* tpcb, int argc, char *argv[] );
extern int Cmd_Gcode_Ramps       ( struct tcp_pcb* tpcb, int argc, char *argv[] );
bool Loop_Til_Target(void);

#endif

