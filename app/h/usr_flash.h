#ifndef  USR_FLASH
#define  USR_FLASH


struct Usr_Flash_Struct {
   uint8_t     N                    ;
   uint8_t     Crc                  ;
   uint8_t     Mac_Addr      [ 8 ]  ;
   uint32_t    Ip_Addr              ;
   uint32_t    Mask_Addr            ;
   uint32_t    Gateway_Addr         ;
   uint8_t     Dhcp_Enable          ;
   uint16_t    Config_Port          ;
   char        Id[ 20 ]             ;
   char        Pwd[ 10 ]            ;
   uint32_t    Wdog            ;
};
extern struct Usr_Flash_Struct Usr_Flash_Params;

void Init_Usr_Flash                 ( void );
extern void Usr_Flash2Defult_Values ( void );
extern void Save_Usr_Flash          ( void );
extern void Get_Usr_Flash           ( void );

#endif

