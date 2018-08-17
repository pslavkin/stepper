#include <stdbool.h>
#include <stdint.h>
#include "utils/lwiplib.h"
#include "utils/cmdline.h"
#include "utils/flash_pb.h"
#include "utils/uartstdio.h"
#include "opt.h"
#include "state_machine.h"
#include "usr_flash.h"


struct Usr_Flash_Struct Default_Usr_Flash_Params = {
      .Mac_Addr       = {0x00,0x15,0xA5,0x5D,0x03,0xE9},
      .Ip_Addr        = 0xC0A80264,
      .Mask_Addr      = 0xFFFFFF00,
      .Gateway_Addr   = 0xC0A80201,
      .Dhcp_Enable    = false,
      .Config_Port    = 49152,
      .Id             = "ehergate_12345678",
      .Pwd            = "1234",
      .Wdog           = 0,
};
struct Usr_Flash_Struct Usr_Flash_Params;


void Init_Usr_Flash(void)
{
   FlashPBInit(USR_FLASH_START,USR_FLASH_END,USR_FLASH_SIZE);
   UART_ETHprintf(DEBUG_MSG,"flash inicializada\n");
   Usr_Flash2Defult_Values();
   Get_Usr_Flash();
}
void Usr_Flash2Defult_Values(void)
{
   Usr_Flash_Params=Default_Usr_Flash_Params;
}
void Save_Usr_Flash(void)
{
   FlashPBSave(&Usr_Flash_Params.N);
   UART_ETHprintf(DEBUG_MSG,"flash grabada\n");
}
void Get_Usr_Flash(void)
{
   uint8_t* New_Params=FlashPBGet();
   if(New_Params!=NULL) {
         Usr_Flash_Params=*(struct Usr_Flash_Struct*)New_Params;
         UART_ETHprintf(DEBUG_MSG,"flash leida=%d\n",Usr_Flash_Params.N);
   }
   else
      UART_ETHprintf(DEBUG_MSG,"flash no leida=%d\n",Usr_Flash_Params.N);
}

//-------------------------------------------------------------------------------------
//
