#include <stdint.h>
#include "utils/lwiplib.h"
#include "utils/cmdline.h"
#include "opt.h"
#include "telnet.h"

struct tcp_pcb* soc;

void Create_Socket(void* nil);
//-------------------------------------------------------------------------------------
void Init_Telnet(void)         //inicializa los puertos que se usan en esta maquina de estados de propositos multiples...
{
   tcpip_callback(Create_Socket,0);
}

//magia, como me llega el tpcb segun quien corresponda, estare responidendo a ese
//socket y no a otro, con lo cual tengo resuelte los estaodos de cada uno asi si mas
err_t Rcv_Fn (void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
   if(p!=NULL)  {
      ((char*)p->payload)[p->len-1]='\0';
      CmdLineProcess(p->payload,tpcb);
      tcp_recved(tpcb, p->len);        //aviso que el windows crecio
      pbuf_free(p);                    //libreo bufer
      return 0;
   }
   else {
      tcp_accepted(soc);               //libreo 1 lugar para el blog
      tcp_close(tpcb);                 //cierro
      return ERR_ABRT;                 //aviso que cerre (no se si esta ok)
   }
}

err_t accept_fn (void *arg, struct tcp_pcb *newpcb, err_t err)
{
   tcp_recv(newpcb,Rcv_Fn);
   return 0;
}

void Create_Socket(void* nil)
{
   soc=tcp_new                 (                        );
   tcp_bind                    ( soc ,IP_ADDR_ANY ,1234 );
   soc=tcp_listen_with_backlog ( soc ,3                 );
   tcp_accept                  ( soc,accept_fn          );
}
