/** \file mac_uplink.h
 *******************************************************************************
 ** \brief Processes the packets received from the UART
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

#ifndef UPLINK_H
#define UPLINK_H

#ifdef __cplusplus
extern "C" {
#endif

/*
** ============================================================================
** Public Macro definitions
** ============================================================================
*/

/* None */

/*
** ============================================================================
** Public Structures, Unions & enums Type Definitions
** ============================================================================
*/

/* None */

/*
** ============================================================================
** Public Variable Declarations
** ============================================================================
*/

/* None */

/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/

/* None */

/*
** =============================================================================
** Public Variables Definitions
** =============================================================================
**/

/* None */

/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/

/* None */

/*
**=========================================================================
**  Public Function Prototypes
**=========================================================================
*/

/**
 *******************************************************************************
 ** \brief Function to create and send the mcps_data_confirm primitive to the NHL
 ** \param msdu_handle - handle of the message
 ** \param status - status of the data request
 ** \param timestamp - time at which the data confirmation is received
 ** \retval - 1 on success and 0 on failure
 ******************************************************************************/
uchar send_mcps_data_confirm( uchar msdu_handle, uchar status, ulong timestamp, uchar nb );

#ifdef WISUN_FAN_MAC
uchar send_ws_async_confirm( uchar status ,uint8_t frame_type );
#endif

/**
 *******************************************************************************
 ** \brief Function to create and send the MCPS data indication primitive to
 **        the NHL
 ** \param *rxmsg -  pointer to received structure
 ** \retval - 0 on success and 1 on failure
 ******************************************************************************/
uchar send_mcps_data_indication( mac_rx_t *rxmsg );
uchar send_mcps_ack_indication(mac_rx_t *rxmsg );
void send_mcps_no_ack_indication (void);

#if(CFG_MCPS_PURGE_REQ_CONF == 1)
/**
 *******************************************************************************
 ** \brief Function to create and send a purge confirm to the NHL
 ** \param msdu_handle - handle of the message to purge
 ** \param status - status of the purge request
 ** \retval - None
 ******************************************************************************/

void send_mcps_purge_confirm( uchar msdu_handle, uchar status );

#endif	/*(CFG_MCPS_PURGE_REQ_CONF == 1)*/

#if(CFG_MLME_ASSOCIATE_REQ_CONF == 1)
/**
 *******************************************************************************
 ** \brief Function to create and send associate confirm primitive to NHL
 ** \param short_address - short address of the device assigned by the Coord
 ** \param status - status of the associate request
 ** \param *sec_params - pointer to the security material 
 ** \retval - None
 ******************************************************************************/
void send_mlme_associate_confirm ( 
									ushort short_address, 
									uchar status, 
									security_params_t *sec_params 
								 );

#endif	/*(CFG_MLME_ASSOCIATE_REQ_CONF == 1)*/
#if(CFG_MLME_ASSOCIATE_IND_RESP == 1)
/**
 *******************************************************************************
 ** \brief Function to create and send associate indication mac primitive to NHL
 ** \param *rxb - pointer to received structure
 ** \param *params - pointer to the capability information of the joining device
 ** \param *sec_params - pointer to the security material 
 ** \retval - None
 ******************************************************************************/
void send_mlme_associate_indication (
										 uchar *rxb,
										 uchar *params,
										 security_params_t *sec_params
                                     );
#endif	/*(CFG_MLME_ASSOCIATE_IND_RESP == 1)*/
#if(CGF_MLME_BEACON_NOTIFY_IND == 1)
/**
 *******************************************************************************
 ** \brief Function to create and send beacon notify indication mac primitive 
 **        to NHL
 ** \param *rxmsg - pointer to received structure
 ** \param *pandesc - pointer to the pandescriptor 
 ** \retval - 1 on success and 0 on failure
 ******************************************************************************/
uchar send_mlme_beacon_notify( mac_rx_t *rxmsg, pandesc_t *pandesc );
#endif	/*(CGF_MLME_BEACON_NOTIFY_IND == 1)*/

#if(CGF_MLME_COMM_STATUS_IND == 1)
/**
 *******************************************************************************
 ** \brief Function to create and send the comm status indication mac primitive 
 **        to the NHL
 ** \param *src - source address 
 ** \param *dst - destination address
 ** \param status - status as a result of reponse or security failure
 ** \param *sec_params - pointer to the security material 
 ** \retval - 1 on success and 0 on failure
 ******************************************************************************/
uchar send_mlme_comm_status_indication (
											mac_address_t *src,
											mac_address_t *dst,
											uchar status,
											security_params_t *sec_param
                                        );
#endif	/*(CGF_MLME_COMM_STATUS_IND == 1)*/

#if(CFG_MLME_DISASSOCIATE_REQ_CONF == 1)
/**
 *******************************************************************************
 ** \brief Function to create and send the disassociate confirmation mac primitive 
 **        to the NHL
 ** \param status - status of disassociate request 
 ** \param *addr - address of the disassociated device 
 ** \retval - None
 ******************************************************************************/
void send_mlme_disassociate_confirm(
										uchar status,
										mac_address_t *addr
                                    );
#endif	/*(CFG_MLME_DISASSOCIATE_REQ_CONF == 1)*/

#if(CFG_MLME_DISASSOCIATE_IND == 1)
/**
 *******************************************************************************
 ** \brief Function to create and send the disassociate indication primitive to 
 **         the NHL
 ** \param *rxb - pointer to received structure
 ** \param *params - pointer to the disassociate reason
 ** \param *sec_params - pointer to the security material 
 ** \retval - None
 ******************************************************************************/
void send_mlme_disassociate_indication (
											uchar *rxb,
											uchar *params,
											security_params_t *sec_params
                                        );
#endif	/*(CFG_MLME_DISASSOCIATE_IND == 1)*/

#if(CFG_MLME_GET_REQ_CONF == 1)
/**
 *******************************************************************************
 ** \brief Function to create and send the get confirmation mac primitive to 
 **         the NHL
 ** \param status - status of get request 
 ** \param id - primitive id of the MAC PIB
 ** \param attribute_index - index of the table
 ** \param length - length of the requested PIB primitive
 ** \param *value - pointer to the value of the requested PIB primitive
 ** \retval - 1 on success and 0 on failure
 ******************************************************************************/
uchar send_mlme_get_confirm ( 
								 uchar status,
								 uchar id,
								 uchar attribute_index,
								 ushort length,
								 void *value 
						    );
#endif	/*(CFG_MLME_GET_REQ_CONF == 1)*/


/**
 *******************************************************************************
 ** \brief Function to create and send the gts confirmation mac primitive 
 **        to the NHL
 ** \param gts_characteristics - gts characteristics
 ** \param status - status of gts request 
 ** \retval - None
 ******************************************************************************/
void send_mlme_gts_confirm( uchar gts_characteristics, uchar status );

/**
 *******************************************************************************
 ** \brief Function to create and send the gts indication mac primitive 
 **        to the NHL
 ** \param src_pan_id - source panid
 ** \param *src_addr - pointer to the source address
 ** \param src_addr_mode - source addressing mode
 ** \param address - short address of the device that has been allocated or 
 **                  deallocated a GTS
 ** \param gts_info - characteristics of the GTS 
 ** \param security_level - pointer to security material
 ** \param key_id_mode -
 ** \param *key_identifier -
 ** \retval - None
 ******************************************************************************/
void send_mlme_gts_indication(
								  ushort src_pan_id,
								  address_t *src_addr,
								  uchar src_addr_mode,
								  ushort address,
								  uchar gts_info,
								  uchar security_level,
								  uchar key_id_mode,
								  uchar *key_identifier
                              );
#if(CFG_MLME_ORPHAN_IND_RESP == 1)

/**
 *******************************************************************************
 ** \brief Function to create and send the orphan indication primitive to 
 **         the NHL
 ** \param *rxb - pointer to received structure
 ** \param *sec_params - pointer to the security material 
 ** \retval - None
 ******************************************************************************/
void send_mlme_orphan_indication(
									 uchar *rxb,
									 security_params_t *sec_params
                                 );
#endif	/*(CFG_MLME_ORPHAN_IND_RESP == 1)*/
#if(CFG_MLME_POLL_REQ_CONF == 1)
/**
 *******************************************************************************
 ** \brief Function to create and send the poll confirmation mac primitive to 
 **         the NHL
 ** \param status - status of poll request
 ** \retval - 1 on success and 0 on failure
 ******************************************************************************/
uchar send_mlme_poll_confirm( uchar status );
#endif	/*(CFG_MLME_POLL_REQ_CONF == 1)*/

#if(CGF_MLME_RESET_REQ_CONF == 1)
/**
 *******************************************************************************
 ** \brief Function to create and send the reset confirmation mac primitive to 
 **         the NHL
 ** \param status - status of reset request
 ** \retval - None
 ******************************************************************************/
void send_mlme_reset_confirm( uchar status );
#endif	/*(CGF_MLME_RESET_REQ_CONF == 1)*/

#if(CFG_MLME_RX_ENABLE_REQ_CONF == 1)
/**
 *******************************************************************************
 ** \brief Function to create and send the rxenable confirmation mac primitive to 
 **         the NHL
 ** \param status - status of rxenable request
 ** \retval - None
 ******************************************************************************/
void send_mlme_rx_enable_confirm( uchar status );

#endif

#if(CFG_MLME_SCAN_REQ_CONF == 1 )
/**
 *******************************************************************************
 ** \brief Function to create and send the scan confirmation mac primitive to 
 **         the NHL
 ** \param status - status of scan request
 ** \param param - scan parameters
 ** \retval - None
 ******************************************************************************/
void send_mlme_scan_confirm( mac_status_t status, scan_param_t param );
#endif	/*(CFG_MLME_SCAN_REQ_CONF == 1 )*/

#if(CFG_MLME_SET_REQ_CONF == 1)
/**
 *******************************************************************************
 ** \brief Function to create and send the set confirmation mac primitive to 
 **         the NHL
 ** \param status - status of set request
 ** \param id - primitive id of the requested PIB
 ** \param attribute_index - index to the table
 ** \retval - 1 on success and 0 on failure 
 ******************************************************************************/
uchar send_mlme_set_confirm(
								uchar status,           
								uchar id,               
								uchar attribute_index   
                            );
#endif	/*(CFG_MLME_SET_REQ_CONF == 1)*/
#if(CFG_MLME_START_REQ_CONF == 1)
/**
 *******************************************************************************
 ** \brief Function to create and send the start confirmation mac primitive to 
 **         the NHL
 ** \param status - status of start request
 ** \retval - None
 ******************************************************************************/
void send_mlme_start_confirm( uchar status );
#ifdef WISUN_FAN_MAC 
  void send_mlme_set_mac_channel_info_confirm( uchar status );
  void send_mlme_set_fan_mac_confirm( uchar status,uchar subie_val );
  
#endif /*(end of WISUN_FAN_MAC)*/
#endif	/*(CFG_MLME_START_REQ_CONF == 1)*/

#if(CFG_MLME_SYNC_LOSS_IND == 1)
/**
 *******************************************************************************
 ** \brief Function to create and send the sync loss indication mac primitive to 
 **         the NHL
 ** \param loss_reason - the reason that synchronization was lost
 ** \param *sec_params - pointer to the security material
 ** \retval - None
 ******************************************************************************/
void send_mlme_sync_loss_indication(
										uchar loss_reason,
										security_params_t *sec_params
                                    );
#endif	/*(CFG_MLME_SYNC_LOSS_IND == 1)*/

#if(CGF_MLME_BEACON_REQUEST_IND == 1)
/**
 *******************************************************************************
 ** \brief Function to create and send the sync loss indication mac primitive to 
 **         the NHL
 ** \param bcn_type - type of beacon 1 - enhanced beacon and 0 - regular beacon
 ** \param *mrp - pointer to the received structure
 ** \param ebfie_len - eb filter length 
 ** \param *eb_filter_ie - pointer to the ebfilter 
 ** \retval - None
 ******************************************************************************/
void send_mlme_beacon_request_indicaiton(
											uchar bcn_type,
											mac_rx_t *mrp,
											ushort ebfie_len,
											uchar* eb_filter_ie
										);
                                         
#ifdef WISUN_FAN_MAC
void send_mlme_async_frame_indicaiton(uint8_t* src_mac_addr , uint8_t frame_type , uint8_t status );
#endif                                                                                          
                                                                                        
#endif	/*(CGF_MLME_BEACON_REQUEST_IND == 1)*/

#if(CGF_MLME_BEACON_REQ_CONF == 1)
/**
 *******************************************************************************
 ** \brief Function to create and send the beacon confirmation mac primitive to 
 **         the NHL
 ** \param status - status of beacon request
 ** \retval - None
 ******************************************************************************/
void send_mlme_beacon_confirm( uchar status );
#endif	/*(CGF_MLME_BEACON_REQ_CONF == 1)*/

/**
 *******************************************************************************
 ** \brief Function to create and send the set phy mode confirmation mac primitive  
 **        to the NHL
 ** \param status - status of set phy mode request
 ** \retval - None
 ******************************************************************************/
void send_mac_set_phy_mode_confirm( uchar status );

#ifdef __cplusplus
}
#endif
#endif /* UPLINK_H */

