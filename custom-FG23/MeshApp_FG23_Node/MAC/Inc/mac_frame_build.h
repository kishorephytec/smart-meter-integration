/** \file mac_frame_build.h
 *******************************************************************************
 ** \brief Provides APIs for building MAC frames
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

#ifndef MAC_FRAME_BUILD_H
#define MAC_FRAME_BUILD_H

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

/**
 *******************************************************************************
 ** \struct phy_data_params_t
 ** Data structure to store phy data request params
 *******************************************************************************
 **/
typedef struct phy_data_params_struct
{
	uint16_t tx_channel;			/**< Holds the channel on which tx should be done */
	uint8_t PPDUCoding;				/**< Indicates if the PPDU should be FEC encoded or not*/
	uint8_t FCSLength;				/**< The size of the FCS in the passed PPDU. True for 32-bit CRC, false for 16-bit CRC */
	uint8_t ModeSwitch;				/**< Indicates if PPDU should be transmitted in a different mode */
	uint8_t NewModeSUNPage;			/**< The new SUN page if mode switching is required */
	uint8_t ModeSwitchParameterEntry;
}phy_data_params_t;
        
typedef struct seq_no
{
    uint8_t src_addr[8];
    uint8_t dest_addr[8];
    uint8_t dev_join_status;
    uint8_t dev_ack_status;
}seq_no_t ;

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

#if(CFG_MLME_ASSOCIATE_REQ_CONF == 1)
/**
 *******************************************************************************
 ** \brief Function to build an association request frame for transmission 
 ** \param **p[out] - output pointer to the buffer 
 ** \param capinfo - capability information of the child device
 ** \param *dst - destination address of the device to which the association request 
 **               command has to be sent
 ** \param *phy_params - pointer to the phy parameter details
 ** \param *sec_param - pointer to the security materials
 ** \retval - status
 ******************************************************************************/
mac_status_t mac_frame_build_association_request( 
                                                      mac_tx_t** p,
                                                      uchar capinfo,
                                                      mac_address_t *dst,
                                                      phy_data_params_t* phy_params,
                                                      security_params_t *sec_param 
                                                  );
#endif	/*(CFG_MLME_ASSOCIATE_REQ_CONF == 1)*/

#if(CFG_MLME_ASSOCIATE_IND_RESP == 1)
/**
 *******************************************************************************
 ** \brief Function to build an association response frame for transmission 
 ** \param **p[out] - output pointer to the buffer 
 ** \param *src - source address of the device from which the association response 
 **               command has to be sent
 ** \param *dst - destination address of the device to which the association response 
 **               command has to be sent
 ** \param *short_addr - pointer to the short addres assigned by the Coord
 ** \param status - status of association
 ** \param *sec_param - pointer to the security materials
 ** \retval - status
 ******************************************************************************/
mac_status_t mac_frame_build_association_response( 
													mac_tx_t** p,
													mac_address_t *src,
													mac_address_t *dst,
													uchar *short_addr,
													uchar status,
													security_params_t *sec_param 
												 );
#endif	/*(CFG_MLME_ASSOCIATE_IND_RESP == 1)*/

#if(CFG_MLME_DISASSOCIATE_REQ_CONF == 1)
/**
 *******************************************************************************
 ** \brief Function to build an association response frame for transmission 
 ** \param **p[out] - output pointer to the buffer 
 ** \param *src - source address of the device from which the disassociation  
 **               notification command has to be sent
 ** \param *dst - destination address of the device to which the disassociation  
 **               notification command has to be sent
 ** \param reason - reason of disaasociation
 ** \param *sec_param - pointer to the security materials
 ** \retval - status
 ******************************************************************************/
mac_status_t mac_frame_build_disassociation_notification( 
														  mac_tx_t** p,
                                                          mac_address_t *src,
                                                          mac_address_t *dst,
                                                          uchar reason,
                                                          security_params_t *sec_param 
														);
#endif	/*(CFG_MLME_DISASSOCIATE_REQ_CONF == 1)*/


/**
 *******************************************************************************
 ** \brief build a data frame fro transmission
 ** \param **p[out] - place to store the address of resulting packet
 ** \param *src[in] - source address
 ** \param *dst[in] - destination address
 ** \param tx_options[in] - transmit options
 ** \param fc_options[in] - frame control option
 ** \param *payload[in] - pointer to payload
 ** \param payload_length[in] - length of payload
 ** \param enc_offset[in] - encryption offset
 ** \param *p_hdr_ie_ids[in] - pointer to the header ie ids
 ** \param *phy_params[in] - pointer to the phy parameters
 ** \param *sec_params[in] - security parameters for encryption
 ** \retval status
 ******************************************************************************/
mac_status_t mac_frame_build_data(  mac_tx_t** p, 
                                    mac_address_t *src, 
                                    mac_address_t *dst, 
                                    uchar tx_options, 
									uchar fc_options,
                                    uchar *payload, 
                                    uint16_t payload_length,
                                    uchar enc_offset,
									IE_ids_list_t* p_hdr_ie_ids,
									phy_data_params_t* phy_params,
                                    security_params_t *sec_params
								  );

/**
 *******************************************************************************
 ** \brief Function to build an data request frame for transmission 
 ** \param **p[out] - output pointer to the buffer 
 ** \param *src - source address of the device from which the data request
 **               command has to be sent
 ** \param *coord - coordinator address of the device to which data request
 **               command has to be sent
 ** \param *sec_param - pointer to the security materials
 ** \param cmd_id - indicates type of data request cmd frame
 ** \retval - status
 ******************************************************************************/
mac_status_t mac_frame_build_data_request( 
										   mac_tx_t** p,
                                           mac_address_t *src,
                                           mac_address_t *coord,
                                           security_params_t *sec_param,
										   uchar cmd_id 
										 );

/**
 *******************************************************************************
 ** \brief Function to build panid conflict command frame for transmission 
 ** \param **p[out] - output pointer to the buffer 
 ** \param *src - source address of the device from which the panid conflict
 **               notification command has to be sent
 ** \param *dst - destination address of the device to which panid conflict
 **               notification command has to be sent
 ** \param *sec_param - pointer to the security materials
 ** \retval - status
 ******************************************************************************/
mac_status_t mac_frame_build_panid_conflict_notification( 
														  mac_tx_t** p,
                                                          mac_address_t *src,
                                                          mac_address_t *dst,
                                                          security_params_t *sec_param 
														);

#if	(CFG_ORPHAN_SCAN == 1)
/**
 *******************************************************************************
 ** \brief Function to build orphan notification command frame for transmission 
 ** \param **p[out] - output pointer to the buffer
 ** \param *sec_param - pointer to the security materials
 ** \retval - status
 ******************************************************************************/
mac_status_t mac_frame_build_orphan_notification( 
													mac_tx_t** p,
													security_params_t *sec_param 
												);
#endif	/*(CFG_ORPHAN_SCAN == 1)*/

/**
 *******************************************************************************
 ** \brief Function to build beacon request frame for transmission 
 ** \param **p[out] - output pointer to the buffer
 ** \param br_type - beacon type
 ** \retval - status
 ******************************************************************************/
mac_status_t mac_frame_build_beacon_request( mac_tx_t** p, uchar br_type );

/**
 *******************************************************************************
 ** \brief Function to build beacon command frame for transmission 
 ** \param **txp[out] - output pointer to the buffer
 ** \param sub_type - sub type of beacon
 ** \param *dest - destination address of the device to which beacon command
 **                frame has to be sent
 ** \param *p_hdr_ie_ids - pointer to the header ie ids
 ** \param *p_pld_ie_ids - pointer to the payload ie ids
 ** \retval - status
 ******************************************************************************/
mac_status_t mac_frame_build_beacon(
										mac_tx_t **txp, 
										uchar sub_type,
										mac_address_t *dest,
										uchar* p_hdr_ie_ids,
										uchar* p_pld_ie_ids
								   );


/**
 *******************************************************************************
 ** \brief Function to build coordinator realignment command frame for transmission 
 ** \param **p[out] - output pointer to the buffer 
 ** \param *src - source address of the device from which the coordinator realignment
 **               command has to be sent
 ** \param *dst - destination address of the device to which coordinator realignment
 **               notification command has to be sent
 ** \param *ieee_addr - extended address when called as a response of orphan notification
 ** \param short_addr - short address when called as a response of orphan notification
 ** \param channel - channel on which to send the coordinator realignment cmd frame
 ** \param channel_page - channel page on which to send the coordinator realignment 
 **                       cmd frame
 ** \param *sec_param - pointer to the security materials
 ** \retval - status
 ******************************************************************************/

#if((CFG_MLME_ORPHAN_IND_RESP == 1) || (CFG_MLME_START_REQ_CONF == 1))
mac_status_t mac_frame_build_coordinator_realignment( 
													  mac_tx_t** p,
                                                      mac_address_t *src,
                                                      mac_address_t *dst,
                                                      uchar *ieee_addr,
                                                      ushort short_addr,
                                                      uchar channel,
                                                      uchar channel_page,
                                                      security_params_t *sec_param 
													 );
#endif	/*((CFG_MLME_ORPHAN_IND_RESP == 1) || (CFG_MLME_START_REQ_CONF == 1))*/


#ifdef ENHANCED_ACK_SUPPORT
mac_status_t mac_frame_build_enhanced_ack
(
	mac_tx_t **txp,
	mac_rx_t *rxp
);
#endif

/**
 *******************************************************************************
 ** \brief Function to identify the security material length based on the keyid
 **        mode and secuirty level.
 ** \param *p - pointer to the buffer
 ** \retval - length of the security material
 ******************************************************************************/
#ifdef MAC_CFG_SECURITY_ENABLED
int sec_stuff_length( security_params_t *p );
#endif

#ifdef WISUN_FAN_MAC 
mac_status_t build_mac_pan_solicit_request( mac_tx_t** p,uint32_t chan_value);
mac_status_t build_mac_pan_advt( mac_tx_t **txp,uint32_t chan_value);
mac_status_t build_mac_pan_conf_solicit_request( mac_tx_t** p,uint32_t chan_value);
mac_status_t build_mac_pan_conf( mac_tx_t **txp,uint32_t chan_value);

#if(FAN_EAPOL_FEATURE_ENABLED == 1)
mac_status_t build_eapol_pkt(mac_tx_t** p);
mac_status_t Build_MCPS_EAPOL_FRAME_Request(mac_tx_t** p, 
                                               uint32_t chan_value,
                                               uint8_t fan_frame_type,
                                               uint8_t *hdr_ie_list,
                                               uint8_t *payload_ie_list );

  mac_status_t  MAC_Build_MCPS_EAPOL_FRAME_Request( 
                                              uchar frame_type,
                                              mac_tx_t **p, /* place to store the address of resulting packet */
                                              uchar type, /* type of request      */
                                              uchar sub_type,
                                              mac_address_t *src, /* source address       */
                                              mac_address_t *dst, /* destination address  */
                                              uchar tx_options,   /* transmit options     */
                                              uchar fc_options,   /* frame control options */
                                              uchar *payload, /* pointer to payload   */
                                              uint16_t payload_length,   /* length of payload    */
                                              uchar enc_offset,
                                              IE_ids_list_t* p_hdr_ie_ids,
                                              phy_data_params_t *phy_params,
                                              security_params_t *sec_params,
                                              uint8_t sn
						 ); 
#endif //#if(FAN_EAPOL_FEATURE_ENABLED == 1)

mac_status_t Build_ACK_FRAME_Request(mac_tx_t** p,
                           mac_rx_t *mrp,
                           uint32_t sub_hdr_bitmap,
                           uint32_t sub_pld_bitmap);



mac_status_t Build_MLME_WS_ASYNC_FRAME_Request(mac_tx_t** p, 
                                               uint32_t chan_value,
                                               uint8_t fan_frame_type,
                                               uint32_t hdr_bitmap,//uint8_t *hdr_ie_list,
                                               uint32_t pld_bitmap);//uint8_t *payload_ie_list );
//mac_status_t build_mac_ulad_pkt(mac_tx_t** p );
void build_mac_ulad_pkt(uint8_t *dst_addr);
uchar build_mac_pan_advt_req(uint32_t chan_value);
uchar build_mac_pan_conf_solicit_req(uint32_t chan_value);
uchar build_mac_pan_conf_req(uint32_t chan_value);
#endif 
//-----------------------------------------------------------------------//
     
     mac_status_t MAC_Build_ACK_FRAME_Request(
                               /* uchar frame_type,*/
                               mac_tx_t **p, /* place to store the address of resulting packet */
                               uchar type, /* type of request      */
                               uchar sub_type,
                               mac_address_t *src, /* source address       */
                               mac_address_t *dst, /* destination address  */
                               uchar tx_options,   /* transmit options     */
                               uchar fc_options,   /* frame control options */
                               uchar *payload, /* pointer to payload   */
                               uint16_t payload_length,   /* length of payload    */
                               uchar enc_offset,
                               IE_ids_list_t* p_hdr_ie_ids,
                               phy_data_params_t *phy_params,
                               security_params_t *sec_params,
                               uint8_t sn
                            );
//------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif /* MAC_FRAME_BUILD_H */

