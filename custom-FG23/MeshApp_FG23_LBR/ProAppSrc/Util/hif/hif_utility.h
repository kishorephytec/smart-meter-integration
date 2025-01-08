/** \file hif_utility.h
 *******************************************************************************
 ** \brief  Provides interface funcitons to the HIF module
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

#ifndef _HIF_UTILITY_H
#define _HIF_UTILITY_H

#ifdef __cplusplus
extern "C" {
#endif


#define GROUP_ID_BOARD_TO_BOARD_IF              0x09
/**
 *****************************************************************************
 * @ingroup sysdoc
 *
 * @{
 *****************************************************************************/

/**
 *****************************************************************************
 * @defgroup Hif HIf Utility Module 
 * @brief  This file provides different APIs for configuring and using the host 
 *           interface module. 
 *
 *  This module provides the interface between the host application and the 
 *  lower layer modules.
 *		
 *	First create an object on the hif_t structure and then call hif_module_init() 
 *  to initialise the queues(service_q,recv_q,send_q),the appliaction call back 
 *  and the driver
 * 
 *  Once the hif is initialised register the parser for each layers(mac/phy)API's
 *  to be invoked for parsing packets in the service_q.
 *  
 *	When the host receives packet it stores it in the recv_q by allocating memory 
 *  from the upper layer(NHLE)and invokes hif_proc_packet_from_host() function.  
 *  This function processes the packet and calls hif2mac or hif2phy function  
 *  based on grp_id.
 *
 *  When the lower layers(mac/phy) wants to send the packet to the host it first 
 *  allocates memory from the lower layer(NLLE) and calls the hif_send_msg_up()
 *  function.This function builds the packet in the form the host expect 
 *  ie; from SOF feild and then stores the newly constructed packet in the 
 *  send queue and invokes hif_send_packet_to_host().This function sends 
 *  the packet to the host by invoking driver function.
 *******************************************************************************/

/**
 *******************************************************************************
 * 
 * @}    
 ******************************************************************************/


/*
** ============================================================================
** Public Macro definitions
** ============================================================================
*/

/**
 ** \defgroup hif_defs  HIF Utility Definitions 
 ** \ingroup Hif
 */

/*@{*/

/**
 *******************************************************************************
 * @brief macro,defining the memory location for the upper layer.
 * @note The location pointed by the two macros should not overlap.
 *    
 ******************************************************************************/
#define NHLE_HEAP					0xFFFF0000

/**
 *******************************************************************************
 * @brief macro,defining the memory location for the lower layer.
 * @note The location pointed by the two macros should not overlap.
 *    
 ******************************************************************************/
#define NLLE_HEAP					0xFFFFFF00

/*
** ============================================================================
** Public Structures, Unions & enums Type Definitions
** ============================================================================
*/
/*Function Prototypes*/


/**
 *******************************************************************************
 ** \enum STATAUS
 **  Enumeration used by the application to indicate the STATAUS.
 *******************************************************************************
 **/
enum
{ 
    HIF_SUCCESS				= 0x00,
	HIF_SEND_CONF,
	HIF_NLLE_BUFF_FAILED,
	HIF_BUFF_FAILED,
};

/**
 *******************************************************************************
 ** \enum event
 **  Enumeration used by the application to indicate the events.
 *******************************************************************************
 **/
enum
{ 
    TX_PACKET_EVENT		= 0x01,
	TX_PACKET_SENT_EVENT,
    RX_PACKET_EVENT 
};

/**
 *******************************************************************************
 ** \struct drv_calls
 ** Structure for storing the driver entry functions.
 *******************************************************************************
 **/
typedef struct drv_calls_tag
{
    void *p_drv_ctx; /**< driver context to tbe passed in the below APIs*/
    drv_send_t send; /**< driver Send routine */
    drv_recv_t recv; /**< driver Receive routine */
	drv_init_t init; /**< driver Initialization routine */
	drv_deinit_t deinit; /**< driver de-initialization routine */
}drv_calls_t;

/**
 *******************************************************************************
 ** \struct Structure for the hif buffer data.
 *	To create an UART_Buffer module which contains information about the packet 
 *  buffer.
 *******************************************************************************
 **/
typedef struct hif_buff_tag
{
	queue_item_t * pNext;  /**< link to the next item */
	unsigned char data[1]; /**< place holder for storing the packets 
						   received/to be transmitted over the HIF*/
}hif_buff_t;

/**
 *******************************************************************************
 * \struct Structure for the host interface module data.
 ******************************************************************************/
typedef struct hif_tag
{
	queue_t service_q;/**< queue holding the service APIs for different Layers*/
	queue_t send_q; /**< Queue holding the packets to be txed tp HOST via UART*/
	queue_t recv_q;   /**< Queue holding the packets rxed from HOST via UART*/
	app_call_back_t app_call_back; // to be implemented by app(hif service module)
	drv_calls_t driver;
	uint8_t* p_hif_heap;
	uint8_t rx_state;
	hif_buff_t* p_curr_rx_buff;
	hif_buff_t* p_curr_tx_buff;
}hif_t;

/**
 *******************************************************************************
 * @brief  Call back type definition.
 * 
 ******************************************************************************/
typedef uint8_t (*hif_reveice_cb_t)( uint8_t* pbuffer,uint16_t len);

/**
 *******************************************************************************
 * \struct Structure for holding information about the function entry point which 
 *  processes the received host packet.
 *    This structure definition is used by the application for creating an 
 *    instance and registering with HIF module.\n
 *     Following inforamtion are registerd with the HIF:\n
 *    1) Group ID of the Layer to be tested \n 
 *    2) The corresponding function to be called when a packet with the grp id 
 *       is received by the HIF\n
 *    3) The function parameter to be passed when the above function is invoked 
 *       by the HIF. \n   
 ******************************************************************************/
typedef struct hif_service_t
{
    struct hif_service_t *p_next;   /**< queue holding the service APIs for 
								         different layers*/
    uint8_t group_id;				/**< holds the group id used for identifying 
											the layer under test*/
    void *p_ctx;					/**< holds the function parameter*/
    hif_reveice_cb_t hif_recv_cb;	/**<holds the function pointer */
}hif_service_t;

/**
 *******************************************************************************
 * @enum Group Ids  assigned to different layers to be tested.
 *    Enum used by the application to indicate the Layer to be tested using 
 *    the hardware interface module.
 ******************************************************************************/
typedef enum
{ 
	PHY_LAYER,
	MAC_LAYER    
} Group_ID_t;

/**
 *******************************************************************************
 * @enum Event assigned to be tested.
 *   Enum used by the application to indicate the type of the event to be tested 
 *   using the hardware interface module.
 ******************************************************************************/
//typedef enum
//{ 
//    HIF_RX_EVENT    = 0x80,
//    HIF_TX_EVENT    = 0x40
//} HIF_Event_t;

/*@}*/

/*
** ============================================================================
** Public Variable Declarations
** ============================================================================
*/

/*None*/

/*
** ============================================================================
** Public Function Prototypes
** ============================================================================
*/

/** \defgroup hif_req_functions HIF Utility APIs
 ** \ingroup Hif
 */

/*@{*/
 
/**
 *******************************************************************************
 * @brief Initialises the queues(service_q,send_q,recv_q),application call back 
 *         and the UART driver
 * @param *p_hif_data[in] - pointer to hif module context
 * @param app_call_back - application call back for hif event
 * @param *driver_if - holds all the UART driver entry function pointers
 * @retval true or false.
 * @note - 1)This function should be called atleast once before using any of the 
 *          hif APIs.\n 2)This should be called during system initialisation.
 * 
 ******************************************************************************/
bool hif_module_init 
	( 
		hif_t* p_hif_data, 
		app_call_back_t app_call_back,
		drv_calls_t* driver_if
	);

 /**
 *******************************************************************************
 * @brief Function to hook up packet handler based on the grp_id.
 * @param *p_HIF_Data[in] - pointer to hif module context
 * @param *p_hif_service[in] - pointer to add the grp_id and pointer to
 *							    the function to be called.
 * @param group_id - specifies the id of set of related commands
 * @param hif_recv_cb - packet handler call back
 * @retval true or false.
 * @note - Create a new instance of hif_service_t for every item which should be
 * register in the service_q.
 * 
 ******************************************************************************/
bool 
hif_register_parser( hif_t* p_HIF_Data, hif_service_t *p_hif_service,
					 uint8_t group_id, hif_reveice_cb_t hif_recv_cb
					);

 /**
 *******************************************************************************
 * @brief This function takes the group_id parameter and calculates the checksum 
 *     value then adds the SOF,class_id and calculated checksum field to the 
 *     newly allocated UART buffer and put it to the send_q.
 * @param  *p_Msg[in] - pointer to buffer which holds the packet to be transmitted.
 *                      The first byte sholud hold the cmd id.
 * @param  msg_len[in] - it is the length of the packet payload to be transmitted.
 * @param  group_id - the id to which the packet belongs.
 * @retval status
 * @note - None
 ******************************************************************************/
uint8_t hif_send_msg_up(uint8_t* p_Msg, uint16_t msg_len, uint8_t layer_id, uint8_t protocol_id );

/**
 *******************************************************************************
 * @brief When an packet is recevied from host via UART this function processes it 
 *      and calls the respective packet handler based on the grp_id present in the
*  received packet.
 * @param  *p_hif_data[in] - pointer to hif module context
 * @retval TRUE or FALSE
 * @note  Called whenever there is a packet in the rx_queue
 ******************************************************************************/
bool hif_proc_packet_from_host( hif_t* p_hif_data );

/**
 *******************************************************************************
 * @brief This function is used for sending a packet to the host application ovet the UART. 
 * @param  *p_hif_data[in] - pointer to hif module context
 * @retval TRUE or FALSE
 * @note - None
 ******************************************************************************/
bool hif_send_packet_to_host( hif_t* p_hif_data );

/**
 *******************************************************************************
 * @brief This function is used indicate the debug event
 * @param  event - event type name 
 * @retval - None
 * @note - This function is used during debugging issues.
 ******************************************************************************/
void Indicate_Debug_Event( uint8_t event );

/*@}*/

#ifdef __cplusplus
}
#endif
#endif /* _HIF_UTILITY_H */
