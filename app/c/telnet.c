#include <stdint.h>
#include "utils/lwiplib.h"
#include "utils/cmdline.h"
#include "opt.h"
#include "telnet.h"
#include "events.h"
#include "usr_flash.h"
#include "commands.h"
#include "utils/uartstdio.h"
#include "utils/ringbuf.h"

struct tcp_pcb* soc;

void Create_Socket(void* nil);
//-------------------------------------------------------------------------------------
void Init_Telnet(void)         //inicializa los puertos que se usan en esta maquina de estados de propositos multiples...
{
   tcpip_callback(Create_Socket,0);
}

//defino una estructura para pasarle al Rfv_Fn para que vaya armando la linea de comandos
//en linux no es necesario porque la arma bash, pero en win o con linux pero en modo raw
//es necesario almacenar localmente.. no encontre otra manera de usar  los pbufs encadenados
//sin luberar el windows como para no gastar RAM. estuve cerca pero no me anduvo

struct Gcode_Queue_Struct
{
   uint8_t Buff[APP_INPUT_BUF_SIZE];
   struct tcp_pcb* tpcb;
};
QueueHandle_t Gcode_Queue;

void Gcode_Parser(void* nil)
{
   Gcode_Queue= xQueueCreate(20,sizeof(struct Gcode_Queue_Struct));
   struct Gcode_Queue_Struct D;
   while(1) {
      while(xQueueReceive(Gcode_Queue,&D,portMAX_DELAY)!=pdTRUE)
            ;
      UART_ETHprintf(D.tpcb,"%s ",D.Buff);
      CmdLineProcess((char*)D.Buff,D.tpcb);
      UART_ETHprintf(D.tpcb,"listo\r\n",D.Buff);
   }
}





struct Args_Struct
{
   tRingBufObject RB;
   uint8_t Buff[APP_INPUT_BUF_SIZE];
};

err_t Rcv_Fn (void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
   struct Args_Struct* B=arg;
   if(p!=NULL)  {
      RingBufWrite(&B->RB, p->payload, p->len);
      while(!RingBufEmpty(&B->RB)) {
         int32_t Len=RingBufPeek(&B->RB,NULL);
         if(Len>=0) {
            struct Gcode_Queue_Struct D;
            RingBufRead(&B->RB,D.Buff,Len+1);
            D.Buff[Len]='\0';
            D.tpcb=tpcb;
            xQueueSend(Gcode_Queue,&D,portMAX_DELAY);
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
   if(tpcb!=DEBUG_MSG && tpcb!=UART_MSG) {
      tcp_close(tpcb);                 //cierro
   }
}

err_t accept_fn (void *arg, struct tcp_pcb *newpcb, err_t err)
{
   struct Args_Struct* R=(struct Args_Struct*)pvPortMalloc(sizeof(struct Args_Struct));
   RingBufInit( &R->RB,R->Buff,APP_INPUT_BUF_SIZE);
   tcp_recv    ( newpcb ,Rcv_Fn  );
   tcp_arg     ( newpcb ,R       );
//   Cmd_Welcome ( newpcb ,0 ,NULL );
//   Cmd_Help    ( newpcb ,0 ,NULL );
   return 0;
}

void Create_Socket(void* nil)
{
   soc=tcp_new                 (                        );
   tcp_bind                    ( soc ,IP_ADDR_ANY ,Usr_Flash_Params.Config_Port );
   soc=tcp_listen_with_backlog ( soc ,3                 );
   tcp_accept                  ( soc,accept_fn          );
}
