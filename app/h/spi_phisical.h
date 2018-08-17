#ifndef SPI_PHISICAL
#define SPI_PHISICAL

enum Step01_App_Cmd {
   NOP              = 0x00,   //0b_0000_0000,
   Set_Param_Cmd    = 0x00,   //0b_0000_0000,
   Get_Param_Cmd    = 0x20,   //0b_0010_0000,
   Run_Dir_Cmd      = 0x50,   //0b_0101_0000,
   Step_Clk_Cmd     = 0x58,   //0b_0101_1000,
   Move_Cmd         = 0x40,   //0b_0100_0000,
   Goto_Cmd         = 0x60,   //0b_0110_0000,
   Goto_Dir_Cmd     = 0x68,   //0b_0110_1000,
   Go_Until_Cmd     = 0x82,   //0b_1000_0010,
   Release_Sw_Cmd   = 0x92,   //0b_1001_0010,
   Go_Home_Cmd      = 0x70,   //0b_0111_0000,
   Go_Mark_Cmd      = 0x78,   //0b_0111_1000,
   Reset_Pos_Cmd    = 0xD8,   //0b_1101_1000,
   Reset_Device_Cmd = 0xC0,   //0b_1100_0000,
   Soft_Stop_Cmd    = 0xB0,   //0b_1011_0000,
   Hard_Stop_Cmd    = 0xB8,   //0b_1011_1000,
   Soft_Hi_Z_Cmd    = 0xA0,   //0b_1010_0000,
   Hard_Hi_Z_Cmd    = 0xA8,   //0b_1010_1000,
   Get_Status_Cmd   = 0xD0    //0b_1101_0000,
};
//------------------------------------------------------
extern void    Init_Spi_Phisical ( void );
extern void    Cs_Hi             ( void );
extern void    Cs_Lo             ( void );
extern void    Rst_Hi            ( void );
extern void    Rst_Lo            ( void );
extern bool    Busy_Read         ( void );
// ------------------------------------------------------
extern void    Send_Data2Spi     ( void                        );
// ------------------------------------------------------
extern void    Send_Cmd2Spi     ( struct tcp_pcb* tpcb,uint8_t* Params,uint8_t Len        );
extern void    Send_Cmd2Spi4Int ( struct tcp_pcb* tpcb,uint8_t Cmd, uint8_t P, uint32_t N );
extern void    Toogle_Pulses    ( uint32_t Pulses                                         );
extern void    Init_Powerstep   ( struct tcp_pcb* tpcb                                    );
extern void    Busy_Read_Task   ( void* nil                                               );
//------------------------------------------------------

#endif
