#include <stdint.h>
#include "utils/lwiplib.h"
#include "utils/cmdline.h"
#include "opt.h"
#include "telnet.h"
#include "events.h"
#include "usr_flash.h"
#include "commands.h"
#include "utils/uartstdio.h"

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
struct Args_Struct
{
   char Buff[APP_INPUT_BUF_SIZE];
   uint16_t I;
};

//magia, como me llega el tpcb segun quien corresponda, estare responidendo a ese
//socket y no a otro, con lo cual tengo resuelte los estaodos de cada uno asi si mas
err_t Rcv_Fn (void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
   struct Args_Struct* B=arg;
   if(p!=NULL)  {
      int len=p->len<(sizeof(B->Buff)-B->I)?p->len:(sizeof(B->Buff)-B->I);    //ojo de no escribir mas alla de los limites
      pbuf_copy_partial(p, B->Buff+B->I,len,0);
      B->I+=len;
      tcp_recved(tpcb,p->len);
      if( B->Buff[B->I-1] =='\n' || B->Buff[B->I-1] == '\r' || B->I==sizeof(B->Buff))  //si llego enter O se lleno el buffer
      {
         B->Buff[B->I-1] ='\0';
         if( B->Buff[B->I-2] =='\n' || B->Buff[B->I-2] == '\r')
            B->Buff[B->I-2] ='\0';
         B->I=0;
         CmdLineProcess(B->Buff,tpcb);
      }
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
   void* p =pvPortMalloc(sizeof(struct Args_Struct));
   ((struct Args_Struct*)p)->I=0;
   tcp_recv    ( newpcb ,Rcv_Fn  );
   tcp_arg     ( newpcb ,p       );
   Cmd_Welcome ( newpcb ,0 ,NULL );
   Cmd_Help    ( newpcb ,0 ,NULL );
   return 0;
}

void Create_Socket(void* nil)
{
   soc=tcp_new                 (                        );
   tcp_bind                    ( soc ,IP_ADDR_ANY ,Usr_Flash_Params.Config_Port );
   soc=tcp_listen_with_backlog ( soc ,3                 );
   tcp_accept                  ( soc,accept_fn          );
}
