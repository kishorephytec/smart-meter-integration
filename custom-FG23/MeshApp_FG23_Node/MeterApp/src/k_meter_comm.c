
#include "common.h"
#include "queue_latest.h"
#include "buff_mgmt.h"
#include "list_latest.h"
#include "queue_latest.h"
#include "buffer_service.h"
#include "l3_configuration.h"
#include "l3_timer_utility.h"
#include "hw_tmr.h"
#include "sw_timer.h"
#include "timer_service.h"
#include "mac_interface_layer.h"
#include "sm.h"
#include "ie_element_info.h"
#include "network-manager.h"
#include "uart_hal.h"
#include "uip.h"
#include "AppUDPprocess.h"
#include "k_meter_comm.h"
#include "k_test_app.h" 
#include "k_data_buffer.h"
/*******************************************************************/


int initialisation_done=0;
int flag_set=0;
extern uint8_t global_addr_device[16];
extern uint8_t join_state;
#define APP_CALLBACK_TIMER  ( 50 )

//Timer declarations
static struct l3_ctimer meter_uart_timer; 
static void process_meter_data_cb(void *ptr);
static void process_meter_and_info_data_cb(void *ptr);

#ifdef DEBUG_INFO_COUNT
volatile uint16_t send_count=0;
volatile uint16_t memory_overflow_count=0;
#endif
//Extern from Procubed Stack
extern void mem_rev_cpy(uint8_t* dest, uint8_t* src, uint16_t len );
extern uint8_t UART_Data_send_udp(uint8_t *buf, uint16_t length);

//Extern from k_test_app
extern data_struct_t m_ds_buff;
extern void send_meter_plus_info_data(uint8_t *payload_buff, uint16_t payload_length, data_struct_t *dt);
extern uint8_t m_ds_data[500]; //Temp buffer TODO Allocate address from bm_alloc()
//
uart_dlms_data_t ud_dlms;

//ring buffer to collect meter data
ring_buff_t r_buff;

//protos
static void serial_rx(uint8_t ch);
static uint8_t increment_read_ptr(ring_buff_t* buff);

uart_dlms_callback_t m_dlms_cb;

//using static allocation
uint8_t m_rx_data[MAX_UART_DATA_BUFF_SIZE]; 
uint16_t m_rx_data_len = 0;

//Registers DLMS data rcv event
void register_dlms_callback(uart_dlms_callback_t cb)
{
  m_dlms_cb = cb;
}

//Initializes meter interfacing UART 
void k_init_meter_comm()
{
  register_uart0_cb(serial_rx);
  UART_init();
  init_data_struct(&m_ds_buff, m_ds_data, sizeof(m_ds_data));
  l3_ctimer_set (&meter_uart_timer, APP_CALLBACK_TIMER, process_meter_data_cb, NULL);
}

//Initialize info + meter interfacing UART
void k_init_info_plus_meter_comm()
{
  register_uart0_cb(serial_rx);
  UART_init();
  l3_ctimer_set (&meter_uart_timer, APP_CALLBACK_TIMER, process_meter_and_info_data_cb, NULL);
}


//Interrupt cb meter UART
static void serial_rx(uint8_t ch)
{
  r_buff.data[r_buff.data_ptr] = ch;
  r_buff.data_ptr = (r_buff.data_ptr + 1) % MAX_UART_DATA_BUFF_SIZE;
  r_buff.timeout = 0;
}


// Helper functions for buffer operations
static uint8_t increment_read_ptr(ring_buff_t* buff) 
{
  
  if (buff->read_ptr == buff->data_ptr)
  {
    return 1; // No data to read
  }
  buff->read_ptr++;
  if (buff->read_ptr >= MAX_UART_DATA_BUFF_SIZE)
  {
    buff->read_ptr = 0;
  }
  return 0;
}

//TODO :Anwar add queue
//Process dlms wrapper data from meter uart. 
void get_dlms_data( uint8_t *error_code)
{
  uint16_t index = 0;
  uint16_t bytes_to_read = 0;
  un_data_t un_conversion;
  uint16_t association = 0;
  uint16_t temp_data_ptr = r_buff.data_ptr;
  r_buff.timeout++;
  
  if (temp_data_ptr >= r_buff.read_ptr)
  {
    bytes_to_read = temp_data_ptr - r_buff.read_ptr;
  }
  else
  {
    bytes_to_read = MAX_UART_DATA_BUFF_SIZE + temp_data_ptr - r_buff.read_ptr;
  }
  // Check for available data
  if (bytes_to_read > 1 && r_buff.timeout > 3) 
  {
    // Search for packet start
    uint16_t search_ptr = r_buff.read_ptr;
    while (search_ptr != temp_data_ptr)
    {
      un_conversion.u8_val[1] = r_buff.data[search_ptr];
      un_conversion.u8_val[0] = r_buff.data[(search_ptr + 1) % MAX_UART_DATA_BUFF_SIZE];
      if (un_conversion.u16_val[0] != VERSION_NUMBER)
      {
        increment_read_ptr(&r_buff);
        *error_code = UART_DLMS_INVALID_DATA;
        return;
      }
      if (bytes_to_read < MIN_DLMS_BYTES)
      {
        // Packet incomplete, wait for more data
        *error_code = UART_DLMS_INCOMPLETE_DATA;
        return;
      }
      search_ptr = r_buff.read_ptr;
      uint8_t tempBuff[MIN_DLMS_BYTES];
      for (index = 0; index < MIN_DLMS_BYTES; index++)
      {
        tempBuff[index] = r_buff.data[search_ptr++];
        if (search_ptr >= MAX_UART_DATA_BUFF_SIZE)
        {
          search_ptr = 0;
        }
      }
      index = 2;
      un_conversion.u8_val[1] = tempBuff[index++];
      un_conversion.u8_val[0] = tempBuff[index++];
      if (un_conversion.u16_val[0] != SOURCE_ADDRESS)
      {
        
        increment_read_ptr(&r_buff);
        *error_code = UART_DLMS_INVALID_DATA;
        return;
      }
      un_conversion.u8_val[1] = tempBuff[index++];
      un_conversion.u8_val[0] = tempBuff[index++];
      
      if ((un_conversion.u16_val[0] != LOWEST_LEVEL_ASSN) &&
          (un_conversion.u16_val[0] != LOW_LEVEL_ASSN) &&
            (un_conversion.u16_val[0] != HIGH_LEVEL_ASSN) &&
              (un_conversion.u16_val[0] != PUSH_ASSOCIATION) &&
                (un_conversion.u16_val[0] != FW_UPDATE_ASSN))
      {
        *error_code = UART_DLMS_INVALID_DATA;
        increment_read_ptr(&r_buff);
        return;
      }
      association = un_conversion.u16_val[0];
      un_conversion.u8_val[1] = tempBuff[index++];
      un_conversion.u8_val[0] = tempBuff[index++];
      uint16_t pkt_length = un_conversion.u16_val[0] + MIN_DLMS_BYTES;
      if (bytes_to_read < pkt_length)
      {
        // Packet incomplete, wait for more data
        *error_code = UART_DLMS_INCOMPLETE_DATA;
        return;
      }
      //            uart_dlms_data_t ud;
      //            ud = (uart_dlms_data_t*)app_bm_alloc(sizeof(uart_dlms_data_t));
      //            if(ud == NULL)
      //            {          
      //                *error_code = UART_DLMS_MEMEORY_OVERFLOW;
      //		return NULL;
      //            }
      //            ud->data = (uint8_t*)app_bm_alloc(pkt_length);
      //            if (ud->data == NULL)
      //            {
      //                *error_code = UART_DLMS_MEMEORY_OVERFLOW;
      //				return NULL;
      //            }
      ud_dlms.data_len = pkt_length + 1 +16; //1 for 0x07
      ud_dlms.association = association;
      
      ud_dlms.data[0]=0x07;
            for (int ind = 0; ind < 16; ind++) {
        ud_dlms.data[1 + ind] = global_addr_device[ind];  // Fill IP address starting from index 1
    }
      for (index = 0; index < pkt_length; index++)
      {
        ud_dlms.data[index+17] = r_buff.data[r_buff.read_ptr];
        if (0 != increment_read_ptr(&r_buff))
        {
          *error_code = UART_DLMS_INCOMPLETE_DATA;
          //                    app_bm_free(ud->data);
          //                    app_bm_free((uint8_t *)ud);
          return;
        }
      }
       #if 1
if(!initialisation_done){
char buf[51]={0x00, 0x01, 0x00, 0x01, 0x00, 0x10, 0x00, 0x2B, 0x61, 0x29, 0xA1, 0x09, 0x06, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x01, 0x01, 0xA2, 0x03, 0x02, 0x01, 0x00, 0xA3, 0x05, 0xA1, 0x03, 0x02, 0x01, 0x00, 0xBE, 0x10, 0x04, 0x0E, 0x08, 0x00, 0x06, 0x5F, 0x1F, 0x04, 0x00, 0x00, 0x00, 0x10, 0x03, 0x40, 0x00, 0x07};
      if(memcmp(&ud_dlms.data[17],buf,(43))==0){
        char data_read[21]={0x00, 0x01, 0x00, 0x10, 0x00, 0x01, 0x00, 0x0D, 0xC0, 0x01, 0xC1, 0x00, 0x01, 0x00, 0x00, 0x60, 0x01, 0x00, 0xFF, 0x02, 0x00};
        uart_hal_write((uint8_t *)data_read,21);
      }
#if 1
      if((ud_dlms.data[10+17]==0xC1) || (ud_dlms.data[11+17]==0xC1)){
        // Shift data to make space for 0x4D and IP at the beginnin
        ud_dlms.data[0]=0x4D;
        for (int i = ud_dlms.data_len - 1; i >= 1; i--) {
        ud_dlms.data[i + 1] = ud_dlms.data[i];  // Shift data right by 1 byte starting from index 1
    }

    // Step 3: Place 0x01 in the first byte (index 1)
    ud_dlms.data[1] = 0x11;
    ud_dlms.data_len+=1;
    initialisation_done=1;
    
      }

}
#endif 
      
#endif
      *error_code = UART_DLMS_OK;
      
      //return ud;
    }
  }
  
  *error_code = UART_DLMS_NO_DATA;
  //return NULL;
}




//Meter data processing
static void process_meter_data_cb(void *ptr)
{
  uart_dlms_data_e error_code = UART_DLMS_NO_DATA;
  
  if(join_state == 0x06)
  {
    if(!flag_set){
        char buf[39]={0x00, 0x01, 0x00, 0x10, 0x00, 0x01, 0x00, 0x1F, 0x60, 0x1D, 0xA1, 0x09, 0x06, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x01, 0x01, 0xBE, 0x10, 0x04, 0x0E, 0x01, 0x00, 0x00, 0x00, 0x06, 0x5F, 0x1F, 0x04, 0x00, 0x62, 0x1E, 0x5D, 0xFF, 0xFF};
        uart_hal_write((uint8_t *)buf,39);
        flag_set=1;
  }
    get_dlms_data(&error_code);
    if (ud_dlms.data_len > 8 )
    {
      //send data to udp
#ifdef DEBUG_INFO_COUNT
      send_count++;
#endif
      
      UART_Data_send_udp(ud_dlms.data, ud_dlms.data_len);   
      ud_dlms.data_len = 0;
    }   
    else
    {
      if (error_code == UART_DLMS_MEMEORY_OVERFLOW)
      {
#ifdef DEBUG_INFO_COUNT
        memory_overflow_count++;
#endif
        ;//MEMORY OVERFLOW
      }
    }
  }
  
  
  l3_ctimer_set (&meter_uart_timer, APP_CALLBACK_TIMER, process_meter_data_cb, NULL);
}

//Meter data + info data processing
static void process_meter_and_info_data_cb(void *ptr)
{
  uart_dlms_data_e error_code = UART_DLMS_NO_DATA;
  
  if(join_state == 0x06)
  {
    get_dlms_data(&error_code);
    if (ud_dlms.data_len > 8 )
    {
      //send data to udp
#ifdef DEBUG_INFO_COUNT
      send_count++;
#endif
      send_meter_plus_info_data(ud_dlms.data, ud_dlms.data_len, &m_ds_buff); 
      //uart_hal_write((uint8_t *)m_ds_buff.data_buffer, m_ds_buff.data_size);
      UART_Data_send_udp((uint8_t *)m_ds_buff.data_buffer, m_ds_buff.data_size);
      ud_dlms.data_len = 0;
    }   
    else
    {
      if (error_code == UART_DLMS_MEMEORY_OVERFLOW)
      {
#ifdef DEBUG_INFO_COUNT
        memory_overflow_count++;
#endif
        ;//MEMORY OVERFLOW
      }
    }
  }
  l3_ctimer_set (&meter_uart_timer, APP_CALLBACK_TIMER, process_meter_and_info_data_cb, NULL);
}
