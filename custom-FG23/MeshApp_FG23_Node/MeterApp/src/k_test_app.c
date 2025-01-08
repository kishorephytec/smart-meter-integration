#include "StackPHYConf.h"
#include "l3_configuration.h"
#include "l3_timer_utility.h"
#include "common.h"
#include "queue_latest.h"
#include "buff_mgmt.h"
#include "list_latest.h"
#include "queue_latest.h"
#include "buffer_service.h"
#include "hw_tmr.h"
#include "sw_timer.h"
#include "timer_service.h"
#include "mac_interface_layer.h"
#include "sm.h"
#include "ie_element_info.h"
#include "fan_api.h"
#include "k_test_app.h"
#include "network-manager.h"
#include "uart_hal.h"
#include "uip.h"
#include "k_data_buffer.h"
#include "AppUDPprocess.h"

// Define identifier tags enum
typedef enum {
  ID_NONE,
  ID_MAC_ADDR_SRC = 1,
  ID_MAC_ADDR_DST,
  ID_IP_ADDR_SRC,
  ID_IP_ADDR_DST,
  ID_SELF_MAC,
  ID_PAN_ID,
  ID_NET_NAME,
  ID_RSSI,
  ID_NODE_TYPE,
  ID_PAYLOAD,
  ID_NEIGHBOR_COUNT,
  ID_NEIGHBOR_LIST,
  ID_NEIGHBOR,
  ID_PREFERED_PARENT,
  ID_CRC,
  ID_CHAN,
  ID_EVENT_ID,
  ID_FRAME_ID,
  ID_TOTAL_ELEMENTS,
  ID_PATH_COST,
  ID_COMMAND_GET,
  ID_COMMAND_SET,
} IdentifierTag;

// Define datatype tags enum
typedef enum {
   DT_NULLDATA = 0,
   DT_ARRAY = 1,
   DT_STRUCTURE = 2,
   DT_BOOLEAN = 3,
   DT_BITSTRING = 4,
   DT_INT32 = 5,
   DT_UINT32 = 6,
   DT_FLOATING_POINT = 7,
   DT_OCTET_STRING = 9,
   DT_VISIBLE_STRING =10,
   DT_GENERALIZED_TIME = 11,
   DT_BCD = 13,
   DT_INT8 =15,
   DT_INT16 =16,
   DT_UINT8 = 17,
   DT_UINT16 = 18,
   DT_COMPACT_ARRAY =19,
   DT_INT64 = 20,
   DT_UINT64 = 21,
   DT_ENUM = 22,
   DT_FLOAT32 = 23,
   DT_FLOAT64 = 24,
   DT_DATETIME = 25,
   DT_DATE = 26,
   DT_TIME = 27,
   DT_DONTCARE = 255
} DataTypeTag;

//Max number of nbors
k_nbors_t nbor_list[4];

//Start of packet
#define SOP "$$KIM"
#define EOP "##"
#define SOP_LEN (sizeof(SOP) - 1)

//Extern from Procubed Stack
extern fan_nwk_manager_sm_t fan_nwk_manager_app;
extern fan_mac_nbr_t fan_mac_nbr;
extern void mem_rev_cpy(uint8_t* dest, uint8_t* src, uint16_t len );
extern uint8_t global_addr_device[16];
extern uip_ipaddr_t root_global_addr;
extern uint8_t UART_Data_send_udp(uint8_t *buf, uint16_t length);

//Prototypes
static void appendNbor(data_struct_t* dt, k_nbors_t nbors);
static void addOctetString(data_struct_t* dt, IdentifierTag it, uint8_t* data, uint16_t dataLen);
static void createTestPacket(data_struct_t *dt);
static int getNborTable();
void k_start_test_app(void);


//Timer declarations
//static sw_tmr_t oneSecTimerIns; //[Kimbal] do not use sw timer in app
static struct l3_ctimer oneSecTimerIns; 
static void oneSecTimer_cb(void *ptr);

//zero buff
uint8_t null_buff[16];

//Buffer to hold proprietary packet
data_struct_t m_ds_buff = {0};
uint8_t m_ds_data[500]; //Temp buffer TODO Allocate address from bm_alloc()


//Initialize test app
void k_init_test_app()
{
    init_data_struct(&m_ds_buff, m_ds_data, sizeof(m_ds_data));
}


/*********************TEST_APP_PROCESS**************************************/

typedef enum
{
  APP_UART_DLMS_PKT_RCV_EVENT,
  APP_UDP_PKT_RCV_EVENT,
  APP_UDP_PKT_SEND_EVENT,
}k_test_app_event_e;


L3_PROCESS_NAME(test_app_process);
L3_PROCESS(test_app_process, "Test App Process");


//Call this function to start the process
void test_app_thread_start( void )
{
  l3_process_start(&test_app_process, NULL);   
}


//Test process event handler
static uint8_t 
test_app_thread_process_event_handler(
              l3_process_event_t ev, 
              l3_process_data_t data)
{
  switch (ev)
  {   
  case APP_UART_DLMS_PKT_RCV_EVENT:
    break;
  case APP_UDP_PKT_RCV_EVENT:
    break;
  case APP_UDP_PKT_SEND_EVENT:
    break;
  default:
    break;
  }
  return 0;
}


//Run process task
L3_PROCESS_THREAD(test_app_process, ev, data)
{
  L3_PROCESS_BEGIN();
  
  while(1) 
  {
    L3_PROCESS_YIELD();
    test_app_thread_process_event_handler(ev, data);
  }
  L3_PROCESS_END();
}


void test_app_thread_post_event( l3_process_event_t ev, uint8_t* data)
{
  l3_process_post(&test_app_process, ev, data);
}


//void app_uart_data_recvd_send_over_udp(uint8_t* Appdata)
//{
//  test_app_thread_post_event(0, NULL);    
//} 

#define CALLBACK_INTERVAL  ( 10 * CLOCK_SECOND )

extern uint8_t join_state;
//One second timer
static void oneSecTimer_cb (void *ptr)
{
  
  if(join_state == 0x06)
  {
    char buff[39];
    buff[0]=0x4D;
    buff[1]=0x11;
     for (int ind = 0; ind < 16; ind++) {
        buff[1 + ind] = global_addr_device[ind];  // Fill IP address starting from index 1
    }
    char data[21]={0x00,0x01,0x00,0x01,0x00,0x10,0x00,0x0D,0xC4,0x01,0xC1,0x00,0x0A,0x07,0x73,0x79,0x73,0x74,0x65,0x6D,0x30};
    memcpy(buff+18,data,21);
    
    
    //uart_hal_write((uint8_t *)buff, 21);
    UART_Data_send_udp((uint8_t *)buff, 39);
  static uint64_t count = 0;
  count++;
  if(count % 2 == 0)
  {
    char buffer[20];
    buffer[0]=0x07;
    for (int ind = 0; ind < 16; ind++) {
        buffer[1 + ind] = global_addr_device[ind];  // Fill IP address starting from index 1
    }
    char data[3]={0x01,0x02,0x03};
    memcpy(buffer+17,data,3);
    
     uart_hal_write((uint8_t *)buffer,20);
    UART_Data_send_udp((uint8_t *)buffer, 20);
    
   /* createTestPacket(&m_ds_buff);
    uart_hal_write((uint8_t *)m_ds_buff.data_buffer, m_ds_buff.data_size);
    UART_Data_send_udp((uint8_t *)m_ds_buff.data_buffer, m_ds_buff.data_size);
*/
  }
  }
  
  
  
  l3_ctimer_set (&oneSecTimerIns, CALLBACK_INTERVAL, oneSecTimer_cb, NULL);
  //tmr_stop(&(oneSecTimerIns));
  //tmr_start_relative(&(oneSecTimerIns));
}
void k_start_test_app()
{
    UART_init();
   //tmr_create_one_shot_timer(&oneSecTimerIns, 833333, (sw_tmr_cb_t)&oneSecTimer_cb, NULL);    
   //tmr_start_relative( &(oneSecTimerIns));
  l3_ctimer_set (&oneSecTimerIns, CALLBACK_INTERVAL, oneSecTimer_cb, NULL);
}

static void createTestPacket(data_struct_t *dt)
{
    static uint16_t frameCount;
    init_data_struct(&m_ds_buff, m_ds_data, sizeof(m_ds_data));
    uint8_t totalElements = 10, nborCount = getNborTable();
    
    //Copy headder
    copy_data_buffer(dt, SOP, SOP_LEN, 1);
    
    //Add length
    set_16(dt, 00, APPND);
    
//    //Total elements
//    set_8(dt, ID_TOTAL_ELEMENTS, APPND);
//    set_8(dt, DT_UINT8, APPND);
//    set_8(dt, totalElements, APPND);
//    


    //EVENT_ID
    set_8(dt, ID_EVENT_ID, APPND);
    set_8(dt, DT_UINT8, APPND);
    set_8(dt, 01, APPND);

    //FRAME_ID
    set_8(dt, ID_FRAME_ID, APPND);
    set_8(dt, DT_UINT16, APPND);
    set_16(dt, frameCount++, APPND);

    //Source
    addOctetString(dt, ID_MAC_ADDR_SRC, fan_nwk_manager_app.node_basic_cfg.self_ieee_addr, 8);
    addOctetString(dt, ID_IP_ADDR_SRC, global_addr_device, 16);
    
    //Destination
    addOctetString(dt, ID_MAC_ADDR_DST, root_global_addr.u8, 8);
    addOctetString(dt, ID_IP_ADDR_DST, root_global_addr.u8, 16);

    //PAN_ID
    set_8(dt, ID_PAN_ID, APPND);
    set_8(dt, DT_UINT16, APPND);
    set_16(dt, fan_nwk_manager_app.node_basic_cfg.selected_pan_id, APPND);

    //NET NAME
    set_8(dt, ID_NET_NAME, APPND);
    set_string(dt, STR_TYPE_VISIBLE, fan_nwk_manager_app.node_basic_cfg.netname_ie.network_name, fan_nwk_manager_app.node_basic_cfg.netname_ie.length);

    //RSSI
    set_8(dt, ID_RSSI, APPND);
    set_8(dt, DT_UINT8, APPND);
    set_8(dt, fan_nwk_manager_app.node_basic_cfg.pa_level, APPND);//TODO

    //PREFERED_PARENT
    addOctetString(dt, ID_PREFERED_PARENT, null_buff, 8);

    //Neigbor Count
    set_8(dt, ID_NEIGHBOR_COUNT, APPND);
    set_8(dt, DT_UINT8, APPND);
    set_8(dt, nborCount, APPND);//TODO

    //Nbor List
    set_8(dt, ID_NEIGHBOR_LIST, APPND);
    set_8(dt, DT_ARRAY, APPND);
    set_8(dt, nborCount, APPND);
    
    nborCount = getNborTable();
    for (uint8_t i = 0; i < nborCount; i++)
    {
      appendNbor(dt, nbor_list[i]);
    }
    
    //CRC
    set_8(dt, ID_CRC, APPND);
    set_8(dt, DT_UINT16, APPND);
    set_8(dt, 00, APPND);
    
    //EOP
    copy_data_buffer(dt, EOP, 2, 1);
    
    uint16_t pkt_size = dt->data_size;
    
    dt->data_buffer[SOP_LEN] = (pkt_size >> 8) & 0xFF;
    dt->data_buffer[SOP_LEN + 1] =  pkt_size & 0xFF; 
}
static void appendNbor(data_struct_t* dt, k_nbors_t nbors)
{
    set_8(dt, ID_NEIGHBOR, APPND);
    set_8(dt, DT_STRUCTURE, APPND);
    set_8(dt, 5, APPND); //NO of elements in nbor table

    set_8(dt, ID_SELF_MAC, APPND);
    set_string(dt, STR_TYPE_OCTAT, nbors.macAddr, 8); //MACAddr

    set_8(dt, ID_RSSI, APPND);
    set_8(dt, DT_UINT8, APPND);
    set_8(dt, nbors.rssi, APPND);

    set_8(dt, ID_PATH_COST, APPND);
    set_8(dt, DT_UINT8, APPND);
    set_8(dt, nbors.routing_cost, APPND);

    set_8(dt, ID_CHAN, APPND);
    set_8(dt, DT_UINT8, APPND);
    set_8(dt, nbors.channel, APPND);


    set_8(dt, ID_NONE, APPND);
    set_8(dt, DT_UINT8, APPND);
    set_8(dt, 0, APPND);
}
static void addOctetString(data_struct_t *dt, 
                           IdentifierTag it, 
                           uint8_t* data,
                           uint16_t dataLen)
{
    set_8(dt, it, APPND);
    set_string(dt, STR_TYPE_OCTAT, data, dataLen);
}


int getNborTable()
{
    int idx = 0;
    uint16_t nbr_count = 0;
    mac_nbr_descriptor_t *p_nbr_desc = NULL;
    
    //Not valid for LBR
    if(fan_nwk_manager_app.node_basic_cfg.fan_device_type == LBR_TYPE)
    {
      return 0;
    }
    
    //fill each nbor details
    for (idx = 0; idx < fan_mac_nbr.mac_nbr_info_table_entries; idx++)
    {
      //get nbor details from queue
      p_nbr_desc = (mac_nbr_descriptor_t*)queue_item_read_from (&fan_mac_nbr.desc_table, idx);
      
      if (p_nbr_desc != NULL)
      { 
        nbr_count++;
        mem_rev_cpy( nbor_list[idx].macAddr, p_nbr_desc->mac_addr, 8); 
        nbor_list[idx].rssi = p_nbr_desc->rssi; 
        nbor_list[idx].routing_cost = p_nbr_desc->rev_pan_metrics.routing_cost;
      }
    } 
  return nbr_count;  
}


void send_meter_plus_info_data(uint8_t *payload_buff, uint16_t payload_length, data_struct_t *dt)
{
    static uint16_t frameCount;
    init_data_struct(&m_ds_buff, m_ds_data, sizeof(m_ds_data));
    uint8_t totalElements = 10, nborCount = getNborTable();
    uint8_t l_address[8];
    
    //Copy headder
    copy_data_buffer(dt, SOP, SOP_LEN, 1);
    
    //Add length
    set_16(dt, 00, APPND);

    //EVENT_ID
    set_8(dt, ID_EVENT_ID, APPND);
    set_8(dt, DT_UINT8, APPND);
    set_8(dt, 01, APPND);

    //FRAME_ID
    set_8(dt, ID_FRAME_ID, APPND);
    set_8(dt, DT_UINT16, APPND);
    set_16(dt, frameCount++, APPND);

    //Source
    memcpy(l_address,&global_addr_device[8],8);
    l_address[0] = 0x00;
    addOctetString(dt, ID_MAC_ADDR_SRC, l_address, 8);
    //addOctetString(dt, ID_MAC_ADDR_SRC, fan_nwk_manager_app.node_basic_cfg.self_ieee_addr, 8);
    addOctetString(dt, ID_IP_ADDR_SRC, global_addr_device, 16);
    
    //Destination
    memcpy(l_address,&root_global_addr.u8[8],8);
    l_address[0] = 0x00;    
    addOctetString(dt, ID_MAC_ADDR_DST, l_address, 8);
//    addOctetString(dt, ID_MAC_ADDR_DST, root_global_addr.u8, 8);
    addOctetString(dt, ID_IP_ADDR_DST, root_global_addr.u8, 16);

    //PAN_ID
    set_8(dt, ID_PAN_ID, APPND);
    set_8(dt, DT_UINT16, APPND);
    set_16(dt, fan_nwk_manager_app.node_basic_cfg.selected_pan_id, APPND);

    //NET NAME
    set_8(dt, ID_NET_NAME, APPND);
    set_string(dt, STR_TYPE_VISIBLE, fan_nwk_manager_app.node_basic_cfg.netname_ie.network_name, fan_nwk_manager_app.node_basic_cfg.netname_ie.length);

    //RSSI
    set_8(dt, ID_RSSI, APPND);
    set_8(dt, DT_UINT8, APPND);
    set_8(dt, fan_nwk_manager_app.node_basic_cfg.pa_level, APPND);//TODO

    //PREFERED_PARENT
    addOctetString(dt, ID_PREFERED_PARENT, null_buff, 8);

    //Neigbor Count
    set_8(dt, ID_NEIGHBOR_COUNT, APPND);
    set_8(dt, DT_UINT8, APPND);
    set_8(dt, nborCount, APPND);//TODO

    //Nbor List
    set_8(dt, ID_NEIGHBOR_LIST, APPND);
    set_8(dt, DT_ARRAY, APPND);
    set_8(dt, nborCount, APPND);
    
    nborCount = getNborTable();
    for (uint8_t i = 0; i < nborCount; i++)
    {
      appendNbor(dt, nbor_list[i]);
    }

    
    //Payload
    addOctetString(dt, ID_PAYLOAD, payload_buff, payload_length);
    
    //CRC
    set_8(dt, ID_CRC, APPND);
    set_8(dt, DT_UINT16, APPND);
    set_8(dt, 00, APPND);
    
    //EOP
    copy_data_buffer(dt, EOP, 2, 1);
    
    uint16_t pkt_size = dt->data_size;
    
    dt->data_buffer[SOP_LEN] = (pkt_size >> 8) & 0xFF;
    dt->data_buffer[SOP_LEN + 1] =  pkt_size & 0xFF; 
}

