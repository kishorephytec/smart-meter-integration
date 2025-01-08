/** \file hif_utility.c
 *******************************************************************************
 ** \brief 
 ** Implements the Host interface functionality
 **
 ** \cond STD_FILE_HEADER
 **
 ** COPYRIGHT(c) 2023-24 Procubed Innovations Pvt Ltd.
 ** All rights reserved.
 **
 ** THIS SOFTWARE IS PROVIDED BY "AS IS" AND ALL WARRANTIES OF ANY KIND,
 ** INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR USE,
 ** ARE EXPRESSLY DISCLAIMED.  THE DEVELOPER SHALL NOT BE LIABLE FOR ANY
 ** DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE. THIS SOFTWARE
 ** MAY NOT BE USED IN PRODUCTS INTENDED FOR USE IN IMPLANTATION OR OTHER
 ** DIRECT LIFE SUPPORT APPLICATIONS WHERE MALFUNCTION MAY RESULT IN THE DIRECT
 ** PHYSICAL HARM OR INJURY TO PERSONS. ALL SUCH IS USE IS EXPRESSLY PROHIBITED.
 **
 *******************************************************************************
 **  \endcond
 */
#if 0 
/*******************************************************************************
* File inclusion
*******************************************************************************/
#include "StackPHYConf.h"
#include "common.h"
#include "queue_latest.h"
#include "buff_mgmt.h"
#include "uart_hal.h"
#include "list_latest.h"
#include "hw_tmr.h"
#include "sw_timer.h"
#include "sm.h"
#include "hif_utility.h"
#include "event_manager.h"
#include "timer_service.h"



/*******************************************************************************
* Private macro definitions
*******************************************************************************/

// Raka [13-Nov-2018] ::  The Length field is only changed in NEW HIF Format
/* Raka  ::::: 12 Nov 2018...................
            For referance ............................

          uint8_t synccompak[2] = { 0x41,0x42 };
          uint16_t Len = 0x0001;
          uint16_t NewLen = 0x00;
          *(uint16_t*)&synccompak[0] = Len; //0x01 ,0x00 = 0x0001
          NewLen = *(uint16_t*)&synccompak[0] ;
*/


/**< index to the SOF field in the received buffer*/
#define SOF				        0xA5

/**< index to the class id field in the received buffer*/
#define PROTOCOL_ID_INDEX			0x03

/**< index to the group id field in the received buffer*/
#define LAYER_ID_INDEX				0x04   

/**< index to the primitive id field in the received buffer*/
#define COMMAND_ID_INDEX			0x05   

/**< index to the length field in the received buffer.It is of 2 bytes. */
#define LEN_FLD_INDEX				0x06

#define CRC_FLD_INDEX				0x08

/**< index to the upper layer memory field */
#define NHLE_HEAP				0xFFFF0000

/**< index to the lower layer memory field*/
#define NLLE_HEAP				0xFFFFFF00

/**< default hif header length*/
#define HIF_HDR_LEN				0x09

/**< default reserved area*/
#define RESERVED_AREA				0x04


#define PALDCKSUM_COMPORT_LEN                   2



/******************************************************************************
* Private Structures, Unions & enums Type Definitions
******************************************************************************/
enum 
{                                              
    RX_INIT,
    RX_BUFFER_SET,
    RX_SOF_CHECKING,
    READING_HDR,
    READING_PLD,
    READING_HDR_WITHOUT_HIF_FORMAT,
    READING_PLD_LENGTH_WITHOUT_HIF_FORMAT,
    READING_PLD_WITHOUT_HIF_FORMAT
    
};

/******************************************************************************
* Private Variable Definitions
******************************************************************************/

static hif_t* p_hif;
//uint32_t stop_time;
//uint32_t start_time;
//uint32_t buffer_stop_time;
/*
** ============================================================================
** Private Function Definitions
** ============================================================================*/ 

// call back to be registerd with the UART HAL module which gets invoked when the HALs 
//callback is invoked by the driver
//uint8_t hif2enetCommonTestInterface( uint8_t* pBuff,uint16_t len);

uint32_t uart_debug_rx_count = 0;/*Umesh 12/12/2108*/
uint32_t pkt_drop_count_sw_tmr = 0;/*Umesh 12/12/2108*/
uint32_t uart_drop_else_bytes = 0;/*Umesh 12/12/2108*/

uint8_t get_node_type( void );
static uint8_t hif_receive_in_process = 0;

/*
** ============================================================================
** Public Variable Definitions
** ============================================================================
*/
#if APP_LBR_ROUTER
sw_tmr_t uart_debug;//@Umesh 05_12_2018
#endif


sw_tmr_t uart_rx_data_poll_tmr;

#if(APP_USE_HIF_FOR_RAW_COMMUNICARION == 1)
extern void UART_poll_timer_expire_callback_to_App(void *uart_data);
#endif


/*
** ============================================================================
** External Variable and Function Declarations
** ============================================================================
*/

extern void signal_event_to_mac_task(uint8_t event);

#ifdef MAC_CFG_SECURITY_ENABLED	
	extern void cleanup_security_queues(uchar SetDefaultPIBValue);
#endif


extern void * app_bm_alloc( uint16_t length );    
extern void app_bm_free( uint8_t *pMem   );
//extern void App_exit_continuous_mode(void);
extern const uint16_t max_buffer_size;
extern void hif_process_tx (void *data);
/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/
static uint8_t Generate_Checksum(hif_buff_t* pBuff);
static uint8_t Generate_Checksum_payload(uint8_t* pBuff , uint16_t payld_len);
static bool set_receive_buffer( hif_t* p_hif_data, bool new_buff );
static void hif_call_back (void *AppHandle, uint32_t  Event,void * p_hif_data );

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

/******************************************************************************/
/******************************************************************************/

bool hif_module_init 
	( 
		hif_t* p_hif_data, 
		app_call_back_t app_call_back,
		drv_calls_t* driver_if
	)
{
	
	p_hif = p_hif_data;
	
	queue_initialise ( &p_hif_data->service_q );
	queue_initialise ( &p_hif_data->send_q );
	queue_initialise ( &p_hif_data->recv_q );

	p_hif_data->driver.send = driver_if->send;
	p_hif_data->driver.recv = driver_if->recv;
	p_hif_data->driver.init = driver_if->init;
	p_hif_data->driver.deinit = driver_if->deinit;
	p_hif_data->app_call_back = app_call_back; 
	
	/*register hif call back along with initializing the driver*/
	p_hif_data->driver.init();

	p_hif_data->rx_state = RX_INIT;

        set_receive_buffer( p_hif_data, true );

	uart_hal_register_back( hif_call_back,(void*)p_hif_data );

    return TRUE;
}

/******************************************************************************/
 
bool hif_register_parser 
	( 
		hif_t* p_hif_data, 
        hif_service_t *p_hif_service, 
        uint8_t group_id, 
        hif_reveice_cb_t hif_recv_cb 
	)
{
	p_hif_service->group_id = group_id;
	p_hif_service->hif_recv_cb = hif_recv_cb;
	queue_item_put( &( p_hif_data->service_q ),( queue_item_t* )p_hif_service );
	
	return true;
}

 /*****************************************************************************/
// RAka ::: Embedded code -->>> to ->>>> External Interface
uint8_t hif_send_msg_up(uint8_t* p_Msg, uint16_t msg_len, uint8_t layer_id, uint8_t protocol_id )
{
    /*1) Allocatte a new UART Buffer of type hif_buff_t* 
    2) Copy the packet from p_Msg, msg_len number of bytes into the newly 
    allocated buffs from the place holder "data[0]" after putting class_id and
    sub_class_id in  0th and 1st index of data array 
    3) put the newly allocated buffer in the UART TX Q
    4) send an TX_event to the UART thread */
    hif_buff_t* pBuffer = NULL;

    uint8_t* p_data = (uint8_t*) app_bm_alloc( msg_len + HIF_HDR_LEN + RESERVED_AREA );

    if(  NULL != p_data )
    {
          // Raka added for meter communication .....
          // hif group ID : 0xAA is used for RAW UART Communication with Meter...
          if( protocol_id == 0xAA)
          {
            
                pBuffer = (hif_buff_t*)(p_data-sizeof(queue_item_t *));
                
                //memcpy( &(pBuffer->data[0]), ((uint8_t*)&msg_len), 2);     //Change by shubham 
                
                *(uint16_t*)&pBuffer->data[0] = msg_len; 
                 
		memcpy( &(pBuffer->data[2]), p_Msg, msg_len);                         
		
          }
          else
          {
          // Raka  ....
            
          pBuffer = (hif_buff_t*)(p_data-sizeof(queue_item_t *));
          /* start of frame== 0xA5,0xA5,0xA5*/
          pBuffer->data[0] = SOF;
          pBuffer->data[1] = SOF;
          pBuffer->data[2] = SOF;
          /*class id for zigbee or Wi-fi*/
          pBuffer->data[PROTOCOL_ID_INDEX] = protocol_id;
          /*sub_class id eg.mac or phy   */
          pBuffer->data[LAYER_ID_INDEX] = layer_id;
          /* command id i.e, present at the first byte of the buffer p_Msg*/
          pBuffer->data[COMMAND_ID_INDEX] = *p_Msg++;
          /* length of payload,(the payload begins one byte after the lenght field)*/          
          // substracted 1 for COM port , Application should send the data with COM port .. Raka 
          msg_len -= 1; 
                  
          /* Raka [13-Nov-2018] :: Changed to the below 
          pBuffer->data[LEN_FLD_INDEX] = (uint8_t)msg_len; 
          pBuffer->data[LEN_FLD_INDEX+1] = (uint8_t)((msg_len & 0x0000FF00)>>0x08);          */
          
          *(uint16_t*)&pBuffer->data[LEN_FLD_INDEX] = msg_len;            
          /* calculate the checksum value using the function	
          store the result at the sixth byte of the pBuffer*/
          pBuffer->data[CRC_FLD_INDEX] =  Generate_Checksum(pBuffer);
          /* copy the source buffer(p_Msg+1) into dest buffer(pBuffer)                
          p_Msg+1 is done because the payload starts after the Command ID*/          
          memcpy( &(pBuffer->data[HIF_HDR_LEN]),p_Msg, msg_len);
          pBuffer->data[HIF_HDR_LEN+msg_len] =  Generate_Checksum_payload(&(pBuffer->data[HIF_HDR_LEN]) , msg_len);
          // Add the com port to the UART tx Buff...
          
          // +1 for increment of the index by  payload checksum filed....
          
          pBuffer->data[HIF_HDR_LEN+msg_len +1] = *(p_Msg + msg_len);
       // Raka added for meter communication .....
		
          }
      // Raka Changes ends here ....
          
        /*put the allocated buffer in the send_q*/
        queue_item_put(&(p_hif->send_q),(queue_item_t*)pBuffer);

        /*send an event to the HIF thread */

#if ((RADIO_VALIDATION || SNIFFER) || (APP_HIF_PROCESS_FEATURE_ENABLED == 0))
        event_set(HIF_TX_EVENT);
#else 
        hif_process_tx (NULL);
#endif
        return HIF_SUCCESS;
    }
    return HIF_BUFF_FAILED;	
}

/******************************************************************************/
/******************************************************************************/
// called from thread when RX event is recieved/ Packet has everything from SOF
bool hif_proc_packet_from_host( hif_t* p_hif_data )
{
	hif_service_t* p_curr_ser = NULL;
	hif_buff_t* p_hif_buff = NULL;
	queue_t* p_service_q = &( p_hif_data->service_q );
	
	uint8_t grp_id, status;
	uint8_t* p_data = NULL;
      
	/*read the buffer from the rx queue*/
	p_hif_buff = ( hif_buff_t* )queue_item_get( &( p_hif_data->recv_q ) );
        
	p_data = p_hif_buff->data;
	
	if ( NULL != p_hif_buff )
	{
          // Meter Data Validation ...
          if (p_data[0] == 0x7E && (p_data[1] == 0xA0 ||p_data[1] == 0xA8)) // as per dlms- next packet continue
          {
		/*go thru the service q to get the appropriate handler 
		and invoke the same*/
		
                grp_id = 0xAA;

		p_curr_ser = ( hif_service_t* )queue_item_scan_next( p_service_q,NULL );
		while ( NULL != p_curr_ser )
		{
			if ( grp_id ==  p_curr_ser->group_id )
			{
			   uint8_t len =  (0xFF & (uint8_t)p_data[2]) ;
			   //call the hif2mac or hif2phy function based on grp_id
                            status = p_curr_ser->hif_recv_cb(&p_data[0],len+2); // Header 2 byte in dlms protocol - shubham
                            
                                if ( HIF_SEND_CONF == status )
				{
					// the same buffer is used to fill conf primitive and 
					//put in the send_q using hif_send_msg_up()
					p_hif_data->app_call_back( RX_PACKET_EVENT, 0 );
					return true;
				}
				else
				{
					app_bm_free((((uint8_t*)p_hif_buff) + sizeof(queue_item_t *)) );
					return true;
				}
			}

			p_curr_ser = ( hif_service_t* )queue_item_scan_next( p_service_q,( queue_item_t* )p_curr_ser);
                }
          }  
		/*go thru the service q to get the appropriate handler 
		and invoke the same*/
              else
              { 
		/*go thru the service q to get the appropriate handler 
		and invoke the same*/
		grp_id = p_data[ LAYER_ID_INDEX ];

		p_curr_ser = ( hif_service_t* )queue_item_scan_next(p_service_q,NULL);
		while ( NULL != p_curr_ser )
		{
			if ( grp_id ==  p_curr_ser->group_id )
			{
                          
                          
                                  /* Raka [13-Nov-2018] :: Changed to the below 

				uint16_t len =  ((0x00FF & (uint16_t)p_data[LEN_FLD_INDEX]) << 8) |   (0x00FF & (uint16_t)p_data[LEN_FLD_INDEX+1]);
				*/
                                
                                uint16_t len =  *(uint16_t*)&p_data[LEN_FLD_INDEX];
                            
                                //call the hif2mac or hif2phy function based on grp_id
                                
				status = p_curr_ser->hif_recv_cb(&p_data[COMMAND_ID_INDEX],len);
                                
                              
				if ( HIF_SEND_CONF == status )
				{
					// the same buffer is used to fill conf primitive and 
					//put in the send_q using hif_send_msg_up()
                                        
					p_hif_data->app_call_back( RX_PACKET_EVENT, 0 );
					return true;
				}
				else
				{
					app_bm_free((((uint8_t*)p_hif_buff) + sizeof(queue_item_t *)));
					return true;
				}
			}
                        
			p_curr_ser = ( hif_service_t* )queue_item_scan_next(p_service_q,( queue_item_t* )p_curr_ser);
                        
		}//while
            }//if
        }
	return false;
}

/******************************************************************************/
// called from thread when TX event is recieved. The packet is already having 
//all the contents as per the protocol so just send it

bool hif_send_packet_to_host( hif_t* p_hif_data ) 
{
	hif_buff_t* p_hif_buff = NULL;
	
	uint16_t len = 0;
	
	p_hif_buff = ( hif_buff_t* )queue_item_get( &(p_hif_data->send_q) );


	if( NULL != p_hif_buff )
	{
            if((p_hif_buff->data[0] == 0xA5)&&(p_hif_buff->data[1] == 0xA5) && (p_hif_buff->data[2] == 0xA5))
            {
              
               /* Raka [13-Nov-2018] :: Changed to the below 
              if(p_hif_buff->data[LAYER_ID_INDEX] == GROUP_ID_BOARD_TO_BOARD_IF)
              {
                len =  ((0x00FF & (uint16_t)p_hif_buff->data[LEN_FLD_INDEX]) << 8) |   
                        (0x00FF & (uint16_t)p_hif_buff->data[LEN_FLD_INDEX + 1]);
                
              }
              else
              {
                len =  ((0x00FF & (uint16_t)p_hif_buff->data[LEN_FLD_INDEX+1]) << 8) |   
                        (0x00FF & (uint16_t)p_hif_buff->data[LEN_FLD_INDEX]);
              }
              */
              
              len = *(uint16_t*)&p_hif_buff->data[LEN_FLD_INDEX];
                
		if( (p_hif_data->driver.send( p_hif_buff->data, (len + HIF_HDR_LEN + PALDCKSUM_COMPORT_LEN )) ) < 0 )
		{
			/*could not submit the buffer UART TX, so requeue the message back 
			into the sendQ*/
			queue_front_put(&(p_hif_data->send_q),(queue_item_t*)p_hif_buff );
			
#if (((RADIO_VALIDATION == 1 ) || ( SNIFFER == 1) ) || (APP_HIF_PROCESS_FEATURE_ENABLED == 0))
        event_set(HIF_TX_EVENT);
#else 
        hif_process_tx (NULL);
#endif
			
			return false;
		}

		p_hif_data->p_curr_tx_buff = p_hif_buff;
		return true;
          
            }
            else
               {
                   // len = p_hif_buff->data[0];
                    
//                    len =  ((0x00FF & (uint16_t)p_hif_buff->data[0]) << 8) |   
//                        (0x00FF & (uint16_t)p_hif_buff->data[1]);
                    
                 
                 len = *(uint16_t*)&p_hif_buff->data[0];
                 
                 if( (p_hif_data->driver.send((&p_hif_buff->data[2]), len) ) < 0 )
                 {
                   /*could not submit the buffer UART TX, so requeue the message back 
                   into the sendQ*/
                   queue_front_put (&(p_hif_data->send_q),  (queue_item_t*)p_hif_buff );
                   
#if (((RADIO_VALIDATION == 1 ) || ( SNIFFER == 1) ) || (APP_HIF_PROCESS_FEATURE_ENABLED == 0))
                   event_set(HIF_TX_EVENT);
#else 
                   hif_process_tx (NULL);
#endif
                   return false;
                 }

		p_hif_data->p_curr_tx_buff = p_hif_buff;
                return true;
                
               }

            

	}
	return false;
}

/******************************************************************************/

uint8_t is_hif_receive_in_process (void)
{
  if (get_node_type() == 0x00)
    return hif_receive_in_process;
  else
    return 0;
}

/******************************************************************************/

#if(APP_USE_HIF_FOR_RAW_COMMUNICARION == 1)


static void hif_call_back (
                            void *AppHandle,
                            uint32_t  Event,
                            void *pArg
                          )
{
  hif_t* p_hif_data = ( hif_t* )pArg;
  //uint8_t crc = 0;
 // static uint16_t payload_len = 0;
 // uint8_t *buffer_ptr = NULL;
  
  //invoke the app call back which hif will have access as the app would 
  //have registered during init
  if( TX_COMPLETE_EVENT == Event )
  {
    app_bm_free(((uint8_t*)p_hif_data->p_curr_tx_buff)+sizeof(queue_item_t *));
    p_hif_data->p_curr_tx_buff = NULL;
    p_hif_data->app_call_back( TX_PACKET_SENT_EVENT, 0);
    //set_receive_buffer( p_hif, false );
  }
  else if ( RX_COMPLETE_EVENT == Event )
  {
    uint8_t* p_data = p_hif_data->p_curr_rx_buff->data;
    uint16_t len = 0x00;
    
    tmr_stop( &( uart_rx_data_poll_tmr ) );	
    tmr_start_relative( &( uart_rx_data_poll_tmr ) );		
    
    switch ( p_hif_data->rx_state )
    {
    case RX_BUFFER_SET:
        hif_receive_in_process = 1;        
        len = 0x01;        
        p_hif_data->driver.recv(p_data ,len);
        p_hif_data->app_call_back( RX_COMPLETE_EVENT, *p_data );
        p_data += 0x01;
        p_hif_data->rx_state = READING_PLD_WITHOUT_HIF_FORMAT;
      break;
    case READING_PLD_WITHOUT_HIF_FORMAT:
        len = 0x01;
        p_hif_data->driver.recv(p_data ,len);    
        p_hif_data->app_call_back( RX_COMPLETE_EVENT, *p_data);
        p_data += 0x01;
      break;    
            
    default:
      break;
    }
  }
}



#else
static void hif_call_back (
                            void *AppHandle,
                            uint32_t  Event,
                            void *pArg
                          )
{
  hif_t* p_hif_data = ( hif_t* )pArg;
  uint8_t crc = 0;
  static uint16_t payload_len = 0;
  uint8_t *buffer_ptr = NULL;
  
  //invoke the app call back which hif will have access as the app would 
  //have registered during init
  if( TX_COMPLETE_EVENT == Event )
  {
    app_bm_free(((uint8_t*)p_hif_data->p_curr_tx_buff)+sizeof(queue_item_t *));
    p_hif_data->p_curr_tx_buff = NULL;
    p_hif_data->app_call_back( TX_PACKET_SENT_EVENT, 0);
    
  }
  else if ( RX_COMPLETE_EVENT == Event )
  {
    uint8_t* p_data = p_hif_data->p_curr_rx_buff->data;
    uint16_t len = 0x00;
    
    switch ( p_hif_data->rx_state )
    {
    case RX_BUFFER_SET:
      if( p_data[ 0 ] == SOF )
      {
        hif_receive_in_process = 1;
        p_data += 0x01;
        len = 0x02;
        p_hif_data->driver.recv(p_data ,len);
        p_hif_data->rx_state = RX_SOF_CHECKING;
#if APP_LBR_ROUTER        
        tmr_start_relative( &(uart_debug));//@Umesh 05_12_2018
#endif        
        //start_time = timer_current_time_get();
      }
      else
      {
        uart_drop_else_bytes++;/*Umesh 12/12/2108*/
//        hif_receive_in_process = 0;
        p_hif_data->rx_state = RX_INIT;				
        set_receive_buffer( p_hif, false );
      }
      break;
    case RX_SOF_CHECKING:
      if( (p_data[ 0 ] == SOF )&&
         (p_data[ 1] == SOF ))
      {
//        hif_receive_in_process = 1;
        p_data += 0x02;
        len = 0x06;
        // +1 Because we have read one byte out of 3 byte of the SOF series of 0xA5,0xA5,0xA5
        p_hif_data->driver.recv((p_data + 1) ,len);
        p_hif_data->rx_state = READING_HDR;
      }
      else
      {
//        hif_receive_in_process = 0;
        p_hif_data->rx_state = RX_INIT;				
        set_receive_buffer( p_hif, false );
#if APP_LBR_ROUTER        
        tmr_stop(&uart_debug);//@Umesh 05_12_2018
#endif        
      }
      break;
      
    case READING_HDR:
      crc = Generate_Checksum( p_hif_data->p_curr_rx_buff );      
      if (crc == p_data[CRC_FLD_INDEX])
      {
        
         /* Raka [13-Nov-2018] :: Changed to the below 
        len =  ((0x00FF & (uint16_t)p_data[LEN_FLD_INDEX]) << 8) | 
          (0x00FF & (uint16_t)p_data[LEN_FLD_INDEX+1]);
        payload_len = len ; 
        */
        
        payload_len = len  = *(uint16_t*)&p_data[LEN_FLD_INDEX];
          
        /*check if the length filed has a value greater 
        than the remaining buffer 
        size which is MAX_BUFF_SIZE - 4 */
        
        /*reading the remaing packet which is payload part*/
        if ( len < ( max_buffer_size - 4 ) )
        {
//          hif_receive_in_process = 1;
          p_hif_data->driver.recv(( p_data + HIF_HDR_LEN ) ,( len +PALDCKSUM_COMPORT_LEN));//Payload Checksum + USB Port [ COM Port]  =2 
          p_hif_data->rx_state = READING_PLD;                                          
        }
        else
        {
//          hif_receive_in_process = 0;
          p_hif_data->rx_state = RX_INIT;
          set_receive_buffer( p_hif, false );
#if APP_LBR_ROUTER          
          tmr_stop(&uart_debug);//@Umesh 05_12_2018
#endif          
        }
      }
      else
      {
//        hif_receive_in_process = 0;
        p_hif_data->rx_state = RX_INIT;
        set_receive_buffer( p_hif, false );
#if APP_LBR_ROUTER        
        tmr_stop(&uart_debug);//@Umesh 05_12_2018
#endif        
      }
      break;
      
    case READING_PLD:
      {
        hif_buff_t* ptr = NULL;
        uint8_t* data_ptr = NULL;
        
        buffer_ptr = (uint8_t*)app_bm_alloc (payload_len + HIF_HDR_LEN + PALDCKSUM_COMPORT_LEN + 5 ); //5 bytes Extra to safeguard
        crc = 0;
        if(buffer_ptr!=NULL)
        {
          ptr = (hif_buff_t*)(buffer_ptr - sizeof(queue_item_t *));
          data_ptr = (p_hif_data->p_curr_rx_buff->data + HIF_HDR_LEN); // 9 should replace as MACRO
          
          crc = Generate_Checksum_payload(data_ptr,payload_len );
          if (crc == p_data[ payload_len + HIF_HDR_LEN])
          {
            memcpy(ptr->data,p_hif_data->p_curr_rx_buff->data, (payload_len+HIF_HDR_LEN+PALDCKSUM_COMPORT_LEN));// check  ::(payload_len+9+2)
            queue_item_put(&( p_hif_data->recv_q ), (queue_item_t* )( ptr ) );
            p_hif_data->app_call_back( RX_PACKET_EVENT, 0 );
            hif_receive_in_process = 0;
          }
          else
          {
            hif_receive_in_process = 0;
            p_hif_data->rx_state = RX_INIT;
            set_receive_buffer( p_hif, false );	
          }
        }
        hif_receive_in_process = 0;
        p_hif_data->rx_state = RX_INIT;				
        set_receive_buffer( p_hif, false ); 
        //stop_time = timer_current_time_get();
#if APP_LBR_ROUTER        
        tmr_stop(&uart_debug);//@Umesh 05_12_2018
#endif        
        
      }
      break;
      
    default:
      break;
    }
  }
}


#endif

/******************************************************************************/
/*@umesh for hif cleanup after timeup*/
void clear_buffer_and_state(void *uart_data)
{
    pkt_drop_count_sw_tmr++;
//    hif_t* p_hif_data = ( hif_t* )uart_data;
//    p_hif_data->rx_state = RX_INIT;				
    set_receive_buffer( p_hif, false ); 
    //buffer_stop_time = timer_current_time_get();
    hif_receive_in_process = 0;
}

/******************************************************************************/

static uint8_t Generate_Checksum( hif_buff_t* pBuff )
{
	return ~((  pBuff->data[PROTOCOL_ID_INDEX]  + 
		    pBuff->data[LAYER_ID_INDEX]     +
                    pBuff->data[COMMAND_ID_INDEX]   +
                    pBuff->data[LEN_FLD_INDEX]      + 
                    pBuff->data[LEN_FLD_INDEX+1] ));
}

/******************************************************************************/

static uint8_t Generate_Checksum_payload( uint8_t* pBuff ,uint16_t payld_len )
{
  uint8_t calc_chksum  = 0 ;
  uint16_t iCnt  =0;
  
  
  for (iCnt = 0; iCnt<payld_len;iCnt++)
  {
      calc_chksum += *pBuff++;
  }
    
	return ~(calc_chksum);
}

/******************************************************************************/

static bool set_receive_buffer( hif_t* p_hif_data, bool new_buff )
{
	//allocate a buffer and set it for recieve using driver function.
	uint8_t* p_data;
#if APP_LBR_ROUTER        
	uart_debug_rx_count = 0; /*Umesh 12/12/2018*/
#endif	
	if ( new_buff )
	{
		p_data = (uint8_t*)app_bm_alloc( max_buffer_size );
		
		if ( NULL != p_data )
		{
			p_hif_data->p_curr_rx_buff = (hif_buff_t*)(p_data - sizeof(queue_item_t *));
			p_hif_data->rx_state = RX_BUFFER_SET;
			uart_hal_read(p_hif_data->p_curr_rx_buff->data,0x01);
			return true;
		}
		else
		{
#ifdef MAC_CFG_SECURITY_ENABLED				
			cleanup_security_queues(1); // Raka :: why the hell we are clearing the security pib's
#endif			
			p_data = (uint8_t*)app_bm_alloc( max_buffer_size );
			if ( NULL != p_data )
			{
				p_hif_data->p_curr_rx_buff = (hif_buff_t*)(p_data - sizeof(queue_item_t *));
				p_hif_data->rx_state = RX_BUFFER_SET;
				uart_hal_read(p_hif_data->p_curr_rx_buff->data,0x01);
				return true;
			}
		}
		
	}
	else
	{
		p_hif_data->rx_state = RX_BUFFER_SET;
                memset((uint8_t *)p_hif_data->p_curr_rx_buff->data, 0xFF, max_buffer_size);
		uart_hal_read(p_hif_data->p_curr_rx_buff->data,0x01);
	}
	
	return false;
}

/******************************************************************************/
/******************************************************************************/

void Indicate_Debug_Event( uint8_t event )
{
	//char event_msg[20] = {0xED,};
	//event_msg[1] = event;
	//const char msg[] = {"0x%x,0x%x \r\n\n "};
	//sprintf((char*)&(event_msg[1]),msg,0xED,event);
	//uart_hal_write((u8*)event_msg,sizeof(msg));
	//hif_send_msg_up((uint8_t*)event_msg, 1, 2);
	
}
#endif //#if 0 