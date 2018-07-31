#ifndef SNMP_AGENT
#define SNMP_AGENT

enum Snmp_Events
       {
   Snmp_Packet_Received_Event = 0x0000,
   No_Such_Name_Error_Event   = 0x0001,
   Get_Request_Event          = 0x00A0,
   Get_Next_Request_Event     = 0x00A1,
   Get_Response_Event         = 0x00A2,
   Set_Request_Event          = 0x00A3,
   Get_Bulk_Request_Event     = 0x00A5 //parece que win usa este codigo para el snmp-walk
 };
//----------------------------------------------------------------
struct Snmp_Header_Struct
{
 unsigned char Message_Code;
 unsigned char Message_Length;
 unsigned char Version_Code;
 unsigned char Version_Length;
 unsigned char Version;
 unsigned char Community_Code;
 unsigned char Community_Length;
  char Community[1];
};
struct Snmp_Msg_Struct
{
 unsigned char Msg_Code;
 unsigned char Msg_Length;
 unsigned char Request_Id_Code;
 unsigned char Request_Id_Length;   //win manda 2 bytes de largo mientras que linux 4... 
 unsigned char Request_Id[1];       //esto puede valer 2 bytes o 4 asi que se parte la estructura aca para realijar la siguiente dinamicamente...
};
struct Snmp_Object_Struct
{
 unsigned char Error_Status_Code;
 unsigned char Error_Status_Length;
 unsigned char Error_Status;
 unsigned char Error_Index_Code;
 unsigned char Error_Index_Length;
 unsigned char Error_Index;
 unsigned char Binding_Code;
 unsigned char Binding_Length;
 unsigned char Item_Code;
 unsigned char Item_Length;
 unsigned char Object_Name_Code;
 unsigned char Object_Name_Length;
 unsigned char Object_Name[1];
};
struct Snmp_Value_Struct
{
 unsigned char Value_Code;
 unsigned char Value_Length;
 unsigned char Value[1];
};
//----------------------------------------------------------------
void Snmp_Packet_Arrived (struct udp_pcb *upcb, struct pbuf *p, ip_addr_t* addr,  u16_t port);
extern void    Init_Snmp_Agent       ( void );
extern void    Load_Oid4Tcp          ( void );
extern void    View_Loaded_Oid       ( void );
extern void    Save_Oid4Loaded       ( void );
extern void    View_Actual_Oid       ( void );
// ----------------------------------------------------------------
extern void    Load_Community4Tcp    ( void );
extern void    View_Loaded_Community ( void );
extern void    Save_Community4Loaded ( void );
extern void    View_Actual_Community ( void );
//------------------------------------------------------------------------------

#endif

