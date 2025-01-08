/** \file Fan_factorycmd.h
 *******************************************************************************
 ** \brief This file breifly describes Command details of the RADIO PHY testing
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

#ifndef FAN_FACTORYCMD_API_H
#define FAN_FACTORYCMD_API_H

#ifdef __cplusplus
extern "C" {
#endif


/*
** =============================================================================
** Public Macro definitions
** =============================================================================
*/
	
  
/*
** =============================================================================
** Public Structures, Unions & enums Type Definitions
** =============================================================================
**/


enum
{
	CONTINUOUS_PACKET_TX_MODE = 0x1,
	CONTINUOUS_STREAM_TX_MODE = 0x2,
};

enum
{
	CMD_SUCCESS			= 0x0,		
	FAN_CMD_NOT_SUPPORTED 	        = 0x1,
	FAN_CMD_INVALID			= 0x2,
	NODE_BUSY			= 0x03,
	MOD_TYPE_NOT_CONFIGURED	        = 0x04,
	DEVICE_ROLE_NOT_SET             = 0x05,
	CONTINUOUS_TX_NOT_IN_PROGRESS   = 0x06,
	PKT_OUT_OF_RANGE = 0x07,
	PKT_TX_CONFIG_NOT_SET	=0x08,
	PKT_RX_NOT_IN_PROGRESS  = 0x09,
	PKT_TX_NOT_IN_PROGRESS  = 0x0a,
	CONTINUOUS_RX_NOT_IN_PROGRESS = 0x0b,
        OPERATING_COUNTRY_NOT_SUPPORTED = 0x0C
};
typedef struct rx_mac_frame_info_tag
{
  uint16_t msduLength;
  uint8_t* pMsdu;
  int32_t rssival;
  uint16_t FCS_Length;
}rx_mac_frame_info_t;


typedef enum {
    FACTORY_SM_NONE,
    FACTORY_SM_SUCCESS,
    FACTORY_SM_CCA_SUCCESS,
    FACTORY_SM_CCA_FAILURE
} factory_sm_result_t;



/**
 *******************************************************************************
 ** \enum ccasm_trigger_t
 ** Specific triggers for CCA state machines
 *******************************************************************************
**/ 
/******************************************************************************
                        Factory mode Command ID
*******************************************************************************/
#define SWITCH_OPERATIONAL_MODE                 0xC0
#define SWITCH_OPERATIONAL_MODE_CONF            0xC1
#define GET_OPERATIONAL_MODE                    0xC2
#define SEND_OPERATINAL_MODE                    0xC3  
#define SET_TX_PKT_CONFIG                       0xC4
#define SET_TX_PKT_CONFIG_CONF                  0xC5
#define START_PACKET_TX_REQ                     0xC6
#define START_PACKET_TX_REQ_CONF                0xC7
#define TX_PACKET_CONF                          0xC8
#define STOP_PACKET_TX_REQ                      0xC9
#define STOP_PACKET_TX_REQ_CONF                 0xCA
#define START_PACKET_RX_REQ                     0xCB
#define START_PACKET_RX_CONF                    0xCC
#define STOP_PACKET_RX_REQ                      0xCD
#define STOP_PACKET_RX_REQ_CONF                 0xCE
#define RX_FRAME_IND                            0xCF
#define RX_FRAME_IND_DISPLAY_OFF                0xD0
#define START_START_CONTINUOUS_TX_REQ           0xD1
#define START_START_CONTINUOUS_TX_REQ_CONF      0xD2
#define STOP_CONTINUOUS_TX_REQ                  0xD3
#define STOP_CONTINUOUS_TX_CONF                 0xD4
#define START_CONTINUOUS_RX_REQ                 0xD5
#define START_CONTINUOUS_RX_CONF                0xD6
#define STOP_CONTINUOUS_RX_REQ                  0xD7
#define STOP_CONTINUOUS_RX_CONF                 0xD8
#define GET_RX_COUNT_DETAILS                    0xD9
#define SEND_RX_COUNT_DETAILS                   0xDA
#define PHY_ENC_TEST_TX                         0xDB
#define ACTIVE_CMD_NOT_SUPPORTED                0xDC

#define SET_RSSI_THRESHOLD                      0xDD
#define SET_RSSI_THRESHOLD_CONF                 0xDE

#define CMD_FACTROY_MODE_CHANNEL_SCAN_REQ      0xDF
#define CMD_FACTROY_MODE_CHANNEL_SCAN_CONF     0xE0


//#define SET_SERIAL_BAUDRATE                     0xE1
//#define SET_SERIAL_BAUDRATE_CONF                0xE2
#define GET_CONFIG_INFO_REQ                     0xE3
#define GET_CONFIG_INFO_RESP                    0xE4
  



void process_set_pkt_tx( uint8_t *buf, uint16_t length );
void process_start_tx( uint8_t *buf, uint16_t length );
void process_stop_tx( uint8_t *buf, uint16_t length );
void process_start_rx( uint8_t *buf, uint16_t length );
void process_stop_rx( uint8_t *buf, uint16_t length );
void process_start_continuous_tx( uint8_t *buf, uint16_t length );
void process_stop_continuous_tx( uint8_t *buf, uint16_t length );
void process_start_continuous_rx( uint8_t *buf, uint16_t length );
void process_stop_continuous_rx( uint8_t *buf, uint16_t length );
void process_get_rx_details( uint8_t* buf, uint16_t length );
void process_test_phy_enc( uint8_t* buf, uint16_t length );
void process_set_rssi_threshold ( uint8_t* buf, uint16_t length );
void process_factory_mode_ch_scanning_req( uint8_t* buf, uint16_t length );
//void process_set_adjust_freq ( uint8_t* buf, uint16_t length );
//void process_set_serial_baudrate ( uint8_t* buf, uint16_t length );
void process_get_config_info_req (void);
#ifdef __cplusplus
}
#endif
#endif /* FAN_FACTORYCMD */
