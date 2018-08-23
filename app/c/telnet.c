#include <stdint.h>
#include "utils/lwiplib.h"
#include "utils/cmdline.h"
#include "opt.h"
#include "events.h"
#include "usr_flash.h"
#include "commands.h"
#include "utils/uartstdio.h"
#include "utils/ringbuf.h"
#include "telnet.h"
#include "gcode.h"

struct tcp_pcb* soc;

void Create_Socket(void* nil);
//-------------------------------------------------------------------------------------
void Init_Telnet(void)         //inicializa los puertos que se usan en esta maquina de estados de propositos multiples...
{
   tcpip_callback(Create_Socket,0);
}

void Print_Slide(struct Gcode_Queue_Struct *D)
{
      uint8_t i;
      char Slide[21];
      uint8_t Len=(uxQueueMessagesWaiting(Gcode_Queue)*(sizeof(Slide)-1))/GCODE_QUEUE_SIZE;
      for(i=0;i<Len;i++)
         Slide[i]='*';
      for(;i<(sizeof(Slide)-1);i++)
         Slide[i]='.';
      Slide[i]='\0';
      UART_ETHprintf ( D->tpcb,"#%06d %12s| %s | %12s\r\n",
                        D->Id,
                        Actual_Cmd.Buff,
                        Slide,
                        D->Buff);
}

err_t Rcv_Fn (void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
   struct Telnet_Args* B=arg;
   if(p!=NULL)  {
      RingBufWrite(&B->RB, p->payload, p->len);
      while(!RingBufEmpty(&B->RB)) {
         int32_t Len=RingBufPeek(&B->RB,'\n');
         if(Len>=0) {
            struct Gcode_Queue_Struct D;
            RingBufRead(&B->RB,D.Buff,Len+1);
            D.Buff[Len]='\0';
            D.tpcb=tpcb;
            D.Id=B->Id;
            B->Id++;
            while(xQueueSend(Gcode_Queue,&D,portMAX_DELAY)!=pdTRUE)
               ;
            Print_Slide(&D);
         }
         else
            break;
      }
      tcp_recved(tpcb,p->len);
      pbuf_free(p);                    //libero bufer
      return ERR_OK;
   }
   else {
      vPortFree(arg);                  //libero el buffer de recepcion
      tcp_accepted(soc);               //libreo 1 lugar para el blog
      tcp_close(tpcb);                 //cierro
      return ERR_OK;
   }
}

void Telnet_Close ( struct tcp_pcb *tpcb)
{
   if(tpcb!=DEBUG_MSG && tpcb!=UART_MSG && tpcb!=ESP_MSG) {
      tcp_close(tpcb);
   }
}

err_t accept_fn (void *arg, struct tcp_pcb *newpcb, err_t err)
{
   struct Telnet_Args* R= ( struct Telnet_Args* )pvPortMalloc(sizeof(struct Telnet_Args));
   RingBufInit ( &R->RB,R->Buff,APP_INPUT_BUF_SIZE );
   R->Id=0;
   tcp_recv    ( newpcb ,Rcv_Fn                    );
   tcp_arg     ( newpcb ,R                         );
   return 0;
}

void Create_Socket(void* nil)
{
   soc=tcp_new                 (                                                );
   tcp_bind                    ( soc ,IP_ADDR_ANY ,Usr_Flash_Params.Config_Port );
   soc=tcp_listen_with_backlog ( soc ,3                                         );
   tcp_accept                  ( soc,accept_fn                                  );
}
