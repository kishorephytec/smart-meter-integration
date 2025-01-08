/** \file fan_mac_ie.h
 *******************************************************************************
 ** \brief  Provides the different structure definitions required for Information Elements
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
#ifndef _FAN_MAC_IE_MANAGER_
#define _FAN_MAC_IE_MANAGER_

#ifdef __cplusplus
extern "C" {
#endif
  
/*
** =============================================================================
** Public Macro definitions
** =============================================================================
*/
#define MAX_IE_BUFF_LEN                                                 10
  
#define HIE_TERMINATION                                                 0 
/*! Defines IE Header Type */
#define IE_TYPE_HDR							0
/*! Defines IE Payload Type */
#define IE_TYPE_PLD							1
#define IE_DISCRIPTOR_LEN                                               2  
  
  //WH_IE
#define WH_IE                                                           0x2A
#define WH_IE_SUBID_UTT_IE_SHORT                                        0x01
#define WH_IE_SUBID_BT_IE_SHORT                                         0x02
#define WH_IE_SUBID_FC_IE_SHORT                                         0x03
#define WH_IE_SUBID_RSL_IE_SHORT                                        0x04
#define WH_IE_SUBID_MHDS_IE_SHORT                                       0x05
#define WH_IE_SUBID_VH_IE_SHORT                                         0x06
#define WH_IE_SUBID_EA_IE_SHORT                                         0x09
#define UNKNOWN_SUB_HDR_IE_SHORT                                        0xAB

#define MAX_WH_IE_SUBID_PRESENT                                         7
  
/*Element IDs, Header IEs*/
/*! Defines HeaderIE ID for LE_CSL*/
#define HIE_LE_CSL							0x1A
/*! Defines HeaderIE ID for LE_RIT*/
#define HIE_LE_RIT							0x1B
/*! Defines HeaderIE ID for DSME_PAN_DESC*/
#define HIE_DSME_PAN_DESC					        0x1C
/*! Defines HeaderIE ID for RZ_TIME*/
#define HIE_RZ_TIME							0x1D
/*! Defines HeaderIE ID for ACK_NACK_TIME_CORR*/
#define HIE_ACK_NACK_TIME_CORR				                0X1E
/*! Defines HeaderIE ID for GACK*/
#define HIE_GACK							0x1F
/*! Defines HeaderIE ID for LLN_INFO*/
#define HIE_LLN_INFO						        0x20
/*! Defines HeaderIE ID for TERMINATION*/
#define HIE_TERMINATION_IE1						0x7E
#define HIE_TERMINATION_IE2						0x7F


/*! Defines total number of header IEs supported*/
#define TOTAL_HIEs_SUPPORTED				                0x02

/* Payload IE ID's*/
/*! Defines PayloadIE ID for SDU*/
#define PIE_ESDU							0x00
/*! Defines PayloadIE ID for MLME_NESTED*/
#define PIE_MLME_NESTED							0x01

  
 //WP_IE 
#define WP_IE                                                           0x04
#define WP_IE_SUBIE_SUBID_US_IE_LONG                                    0x01
#define WP_IE_SUBIE_SUBID_BS_IE_LONG                                    0x02
#define WP_IE_SUBIE_SUBID_VP_IE_LONG                                    0x03
#define WP_IE_SUBIE_SUBID_PAN_IE_SHORT                                  0x04
#define WP_IE_SUBIE_SUBID_NETNAME_IE_SHORT                              0x05
#define WP_IE_SUBIE_SUBID_PANVER_IE_SHORT                               0x06
#define WP_IE_SUBIE_SUBID_GTKHASH_IE_SHORT                              0x07
#define UNKNOWN_SUB_PLD_IE_SHORT                                        0x20
  
#define MAX_WP_IE_SUBID_PRESENT                                         7

  //MPX-IE : Defined in 802.15.9
#define MPX_IE                                                          0x03 

/*0x02 to 0x09 are unmanaged Payload IE Group IDs*/
/*0x0a to 0x0e are reserved Payload IE Group IDs*/

/*! Defines PayloadIE ID for TERMINATION*/
#define PIE_TERMINATION							0x0F

/*! Defines total number of payload IEs supported*/
#define TOTAL_PIEs_SUPPORTED						0x03

/* MLME Nested IE Sub IDs: short*/
/*! Defines Payload SUBIE of Short Type*/
#define SHORT_SUB_IE							0x00
/*! Defines Payload SUBIE of Long Type*/
#define LONG_SUB_IE							0x01

/*BIT MASK for all Header and Payload IEs.
  lower 2 bytes are assigned for short IEs and
  higher 2 bytes are assigned for long IEs*/
#define UTT_IE_MASK                                                   0x00000001
#define BT_IE_MASK                                                    0x00000002
#define FC_IE_MASK                                                    0x00000004
#define RSL_IE_MASK                                                   0x00000008
#define MHDS_IE_MASK                                                  0x00000010
#define VH_IE_MASK                                                    0x00000020
#define EA_IE_MASK                                                    0x00000040

#define US_IE_MASK                                                    0x00010000
#define BS_IE_MASK                                                    0x00020000
#define VP_IE_MASK                                                    0x00040000
#define PAN_IE_MASK                                                   0x00000001
#define NETNAME_IE_MASK                                               0x00000002
#define PAN_VER_IE_MASK                                               0x00000004
#define GTK_HASH_IE_MASK                                              0x00000008

#define WH_IE_MASK                                                    0x00008000
#define HEADER_TIE1_MASK                                              0x00020000
#define HEADER_TIE2_MASK                                              0x00040000

#define MLME_IE_MASK                                                  0x00000002
#define WP_IE_MASK                                                    0x00000008
#define PAYLOAD_TIE_MASK                                              0x00000010
#define MPX_IE_MASK                                                   0x00000020

#define NO_IE_MASK                                                    0x00000000

#define UNKNOWN_SUB_HDR_MASK                                          0x00000100
#define UNKNOWN_SUB_PLD_MASK                                          0x00000010
    
    
struct mpx_data_st
{
  uint8_t *msdu;
  uint16_t msdu_len;
  uint16_t multiplex_id;
  uint8_t transfer_type;
  uint8_t kmp_id;
};

/*0x00 to 0x19 are reservedf short type Sub-IDs*/

/*! Defines Payload SUBIE ID of Short Type for TSCH_SYNC*/
#define MLME_NESTED_SHORT_SUBID_TSCH_SYNC				0x1A
/*! Defines Payload SUBIE ID of Short Type for TSCH_SF_LINK*/
#define MLME_NESTED_SHORT_SUBID_TSCH_SF_LINK			        0x1B
/*! Defines Payload SUBIE ID of Short Type for TSCH_TIMESLOT*/
#define MLME_NESTED_SHORT_SUBID_TSCH_TIMESLOT			        0x1C
/*! Defines Payload SUBIE ID of Short Type for HOPPING_TIME*/
#define MLME_NESTED_SHORT_SUBID_HOPPING_TIME			        0x1D
/*! Defines Payload SUBIE ID of Short Type for EBFILTER*/
#define MLME_NESTED_SHORT_SUBID_EBFILTER				0x1E
/*! Defines Payload SUBIE ID of Short Type for MACMETRICS1*/
#define MLME_NESTED_SHORT_SUBID_MACMETRICS1				0x1F
/*! Defines Payload SUBIE ID of Short Type for MACMETRICS2*/
#define MLME_NESTED_SHORT_SUBID_MACMETRICS2				0x20

#ifdef WISUN_ENET_PROFILE
  #define MLME_NESTED_SHORT_SUBID_PAIRING_ID     			0x68
  #define MLME_NESTED_SHORT_SUBID_SRNIE_ID     			        0x66 //0x69 // raka : SRN IE Value 0x77
  #define MLME_NESTED_SHORT_SUBID_ROUTING_IE_ID     			0x65 //0x6A // raka : Routing IE Value 0x78
  #define MLME_NESTED_SHORT_SUBID_CAPABILITY_NOTIFICATION_IE_ID     	0x67 // raka : Capability Notification 0x79
#endif

/*! Defines Payload SUBIE ID of Short Type for COEX_SPEC*/
#define MLME_NESTED_SHORT_SUBID_COEX_SPEC				0x21
/*! Defines Payload SUBIE ID of Short Type for SUN_PHY_CAP*/
#define MLME_NESTED_SHORT_SUBID_SUN_PHY_CAP				0x22
/*! Defines Payload SUBIE ID of Short Type for MR_FSK_GEN_PHY_DESC*/
#define MLME_NESTED_SHORT_SUBID_MR_FSK_GEN_PHY_DESC			0x23
/*! Defines Payload SUBIE ID of Short Type for MODE_SW_PARAM_ENTRY*/
#define MLME_NESTED_SHORT_SUBID_MODE_SW_PARAM_ENTRY			0x24

/*0x25 to 0x3f are reserved short type Sub-IDs*/
/*0x40 to 0x7f are Unmanaged short type Sub-IDs*/

/*! Defines total number of short payload IEs supported*/
#define TOTAL_NESTED_PIE_SHORT_SUB_IEs_SUPPORTED		        0x04
/* MLME Nested IE Sub IDs: Long*/

/*0x00 to 0x08 are unmanaged long type Sub-IDs*/

/*! Defines Payload SUBIE ID of Long Type for CHAN_HOP_SEQ*/
//#define MLME_NESTED_LONG_SUBID_CHAN_HOP_SEQ			0x09

/*0x0a to 0x0f are reserved long type Sub-IDs*/

/*! Defines total number of long payload IEs supported*/
#define TOTAL_NESTED_PIE_LONG_SUB_IEs_SUPPORTED			0x03


/*different macros used for specifying the IE flags while constructing the formatted IEs
so specify that the termination IE has to be inserted as the last IE in the IE list*/

/*! Defines termination IEs to be included */
#define INCLUDE_TIE						0x01
/*! Defines IE requests*/
#define IE_REQUEST						0x02

#define NOT_INCLUDE_TIE						0x00  
  
/*macros for constructing the EB Filter IE */
/*! Macro to indicate the bit value for PERMIT_JOIN_ON*/
#define PERMIT_JOIN_ON_BIT_ENABLE				0x01
/*! Macro to indicate the bit value for LQI_FILTER*/
#define INCLUDE_LQI_FILTER					0x02
/*! Macro to indicate the bit value for PERCENT_FILTER*/
#define INCLUDE_PERCENT_FILTER					0x04
/*! Macro to indicate the mask value for PIBS_FIELD*/
#define NUM_PIBS_FIELD_MASK					0x38
/*! Macro defining maximum length of EB Filter content*/
#define MAX_EB_FILTER_IE_CONTENT_SIZE				0x08


/*! Defines IE type field length in bits*/
#define IE_TYPE_FIELD_LEN_IN_BITS				1
/*! Defines length for IE Header Type feild in bits*/
#define HDR_IE_ID_FLD_LEN_IN_BITS				8
/*! Defines length of the IE Payload Type feild in bits*/
#define PLD_IE_ID_FLD_LEN_IN_BITS				4

/*! Defines length of the Header IE data length field in bits*/
#define HDR_IE_LENGTH_FLD_LEN_IN_BITS				7
/*! Defines length of the paylod IE content Length field in bits*/
#define PLD_IE_LENGTH_FLD_LEN_IN_BITS				11


/*! Defines IE type mask value*/
#define IE_TYPE_MASK						0x8000
/*! Defines Payload IE ID mask value*/
#define PAYLOAD_IE_ID_MASK					0x7800
/*! Defines masking value for Payload IE content length*/
#define PAYLOAD_IE_CONTENT_LEN_MASK			        0x07FF
/*! Defines masking value for Header IE ID*/
#define HEADER_IE_ID_MASK					0x7F80
/*! Defines masking value for Header IE content length*/
#define HEADER_IE_CONTENT_LEN_MASK			        0x007F

/*! Defines length of sub IE type feild in bits*/
#define SUB_IE_TYPE_FIELD_LEN_IN_BITS		                0x01
/*! Defines length of short sub IE type feild in bits*/
#define SHORT_SUB_ID_FLD_LEN_IN_BITS		                0x07

/*! Defines length of short sub IE type feild in bits*/
#define SHORT_SUB_IE_LENGTH_FLD_LEN_IN_BITS		        0x08

/*! Defines length of short sub IE type feild in bits*/
#define LONG_SUB_IE_LENGTH_FLD_LEN_IN_BITS		        0x0B


/*! Defines length of long sub IE type feild in bits*/
#define LONG_SUB_ID_FLD_LEN_IN_BITS			        0x04
/*! Defines masking value for length of short sub IE descriptor*/
#define CONT_LEN_MASK_IN_SHORT_SUB_IE_DESC	                0x00FF		
/*! Defines masking value for length of long sub IE descriptor*/
#define CONT_LEN_MASK_IN_LONG_SUB_IE_DESC	                0x07FF
/*! Defines masking value for short sub IE ID feild*/
#define SHORT_SUB_IE_ID_FIELD_MASK			        0x7F00
/*! Defines masking value for long sub IE ID feild*/
#define LONG_SUB_IE_ID_FIELD_MASK			        0x7800


/*! Defines number of bits for frequency bands*/
#define ALL_FREQ_BANDS_BIT_POS				        4
/*! Defines number of PIB IDs feild mask value*/
#define NUM_OF_PIB_IDS_FLD_MASK				        0x38
/*! Defines Mode Scheme mask value*/
#define MOD_SCHEME_FIELD_MASK				        0x00300000	



/*
** ============================================================================
** Public Structures, Unions & enums Type Definitions
** ============================================================================
*/

/**
 *******************************************************************************
 ** \struct coex_spec_ie_t
 ** Representation of the coex specification IE
 *******************************************************************************
 **/
typedef struct coex_spec_ie_struct coex_spec_ie_t;

/**
 *******************************************************************************
 ** \struct coex_spec_ie_struct
 ** Data structure to store a Coexspec
 *******************************************************************************
 **/
struct coex_spec_ie_struct
{
    uchar bo_so;					
    uchar final_cap_slot_ebo;		
    uchar ots_cap_backoff_offset;
    uchar channel_page[4];	
    uchar nbpan_ebo[2];
};

/**
 *******************************************************************************
 ** \struct sun_features_bits_t
 ** Structure to indicate bits of sun features
 *******************************************************************************
 **/
typedef struct sun_features_bits_struct
{
	uchar enhanced_ack	            : 1;
	uchar data_whitening                : 1;
	uchar interleaving	            : 1;
	uchar sfd_group1	            : 1;
	uchar nrnsc_fec                     : 1;
	uchar rsc_fec			    : 1;
	uchar mode_switch	            : 1;
	uchar reserved			    : 1;
} sun_features_bits_t;

/**
 *******************************************************************************
 ** \union sun_features_t
 ** Union to indicate sun features
 *******************************************************************************
 **/
typedef union sun_features_struct
{
	sun_features_bits_t sun_feature_bits;
	uchar val;
}sun_features_t;

/**
 *******************************************************************************
 ** \struct phy_descriptor_t
 ** Structure to store phy descriptor details
 *******************************************************************************
 **/
typedef struct phy_descriptor_struct
{
	ushort type_and_modes;
	ushort freq_bands_supported;
}phy_descriptor_t;

/**
 *******************************************************************************
 ** \struct IE_ids_list_t
 ** Structure to store IE Id list
 *******************************************************************************
 **/

typedef struct IE_ids_list_struct
{
	uint8_t ie_list_len;
	uint8_t ie_list[MAX_IE_BUFF_LEN];
	uint8_t ie_flags;
}IE_ids_list_t;

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
** ============================================================================
** Public Function Prototypes
** ============================================================================
*/

/**
 *****************************************************************************
 * @brief This function is used to parse the IE list.
 * @param type[in] - to indicate the type of IE 0 for Header IE and 1 for 
 *                   Payload IE
 * @param *data[in] - input pointer pointing at the byte after aux 
 * (if secured frame) or src address (if unsecured frame) 
 * @param *IEListLen[out] - number of IEs present in the IE list 
 * @param *IEList[out] - pointer to where the IEList begins
 * @param *p_ie_fld_size[out] - actual length of the payload IE
 * @retval - length of the payload size
 *****************************************************************************/
void mac_frame_parse_ie_list(
                                uint8_t type,
                                uchar *data, 
                                uchar* IEListLen,		 
                                uchar** IEList,		 
                                uint16_t* p_ie_fld_size
                            );

void mac_frame_parse_ie_list_new(
                              uint8_t type,
                              uchar *data,
                              uint16_t data_len, // data length or payload lenthg
                              uchar* IEListLen,		 
                              uchar** IEList,		 
                              uint16_t* p_ie_fld_size
                           );

/**
 *****************************************************************************
 * @brief This function used for constructing both PLD and HDR IEs with 
 *         contents if they are supported 
 * @param *buf - input pointer pointing to the IE list
 * @param type - to indicate the type of IE 0 for Header IE and 1 for 
 *                   Payload IE
 * @param ie_list_len - length of IE list 
 * @param *p_ie_ids_list - pointer to where the actual payload IE ids are
 *                              present
 * @param ie_flags - flag to indicate wether termination IEs to be included while
 *                  building IE list
 * @retval - length of the payload size
 *****************************************************************************/
ushort build_ie_list(
                        uchar* buf,
                        uchar type, 
                        uchar ie_list_len, 
                        uchar* p_ie_ids_list,
                        uchar ie_flags 
                      );
ushort build_MPX_IE_list(
                            uchar* buf,
                            uchar type, 
                            uchar ie_list_len, 
                            uchar* p_ie_ids_list,
                            uchar ie_flags ,
                            uchar* msdu,
                            ushort msdu_length,
                            ushort multiplex_id,
                            uchar treanfer_type,
                            uchar kmp_id
					);


/**
 *****************************************************************************
 * @brief This function used for constructing both PLD and HDR IEs descriptor
 *        list without contents
 * @param *buf - input pointer pointing to the IE list
 * @param type - to indicate the type of IE 0 for Header IE and 1 for 
 *                   Payload IE
 * @param ie_list_len - length of IE list 
 * @param *p_ie_ids_list - pointer to where the actual payload IE ids are
 *                              present
 * @param ie_flags - flag to indicate wether termination IEs to be included while
 *                  building IE list
 * @retval - length of the payload size
 *****************************************************************************/
ushort build_ie_desc_list(
							uchar* buf,
							uchar type, 
							uchar ie_list_len, 
							uchar* p_ie_ids_list,
							uchar ie_flags 
						 );

/**
 *****************************************************************************
 * @brief This function used for constructing HDR IEs list without contents
 * @param *buf - input pointer pointing to the IE list
 * @param ie_list_len - length of IE list 
 * @param *p_ie_ids_list - pointer to where the actual header IE ids are
 *                              present
 * @retval - length of the header size
 *****************************************************************************/
ushort build_header_ie_list(
								uchar* buf, 
								uchar ie_list_len, 
								uchar* p_ie_ids_list 
							);

/**
 *****************************************************************************
 * @brief This function used for extract the coex spec IE from the IE list
 * @param *p_mlme_pld_ie - input pointer pointing to the payload IE list
 * @param count - length of payload IE list 
 * @param *coex[out] - output pointer to store the coex spec details
 * @retval - length of the header size
 *****************************************************************************/
uchar ie_mgr_extract_coex_spec_ie(  
									uchar* p_mlme_pld_ie, 
									uchar count, 
									coex_spec_ie_t* coex 
								 );
uchar ie_mgr_extract_pairing_id_ie(  
					uchar* p_pld_data, 
                         ushort pld_len,
					uint8_t* p_pairingid 
				 );

uchar ie_mgr_extract_Capability_Notification_ie(  
					uchar* p_pld_data, 
                                        ushort pld_len,
					uint8_t* p_pairingid 
				 );
/**
 *****************************************************************************
 * @brief This function is used to extract the MLME nested payload IE
 * @param sub_ie_type - to indicate it is a short or long payload IE type
 * @param sub_ie_id - sub IE id of the payload IE
 * @param payloadIEListLen - length of the payload IE list
 * @param *payloadIEList - pointer to the payload IE list
 * @param payloadIEFieldLen - length of the payload IE field
 * @param *p_out - output pointer to point where the MLME nested payload IE begins
 * @param *p_out_len - length of the output pointer
 * @retval - 0 on success
 *****************************************************************************/
uchar extract_mlme_pld_ie(
							uchar sub_ie_type,
							uchar sub_ie_id,	
							uchar payloadIEListLen,	 
							uchar *payloadIEList,
							ushort payloadIEFieldLen,
							uchar* p_out,
							ushort* p_out_len
						 );
/**
 *****************************************************************************
 * @brief This function used to check if the requested IE is supported or not
 * @param type - type of IE
 * @param num_of_ids - number of IE IDs beased on the type
 * @param *p_ids_list - pointer to the IE IDs
 * @retval - true if the IE is supported 
 * @retval - false if the IE is not supported
 *****************************************************************************/
bool any_ie_supported
(
	uchar type,
	ushort num_of_ids,
	uchar* p_ids_list
);

/**
 *****************************************************************************
 * @brief This function is used to extract the IE IDs
 * @param ie_type - indicates the type of IE
 * @param IEListLen - length of the IE list
 * @param *IEList - pointer to the IE list
 * @param IEFieldLen - length of the IE field
 * @param *p_ie_ids - pointer to the payload IE IDs here first byte indicates 
 *                    the number of IE ids present
 * @param * p_total_size - indicates the count of significant bytes 
 *                         present in the list
 * @retval - 1 on success
 *****************************************************************************/
uchar extract_ie_ids
(
	uchar ie_type,	
	uchar IEListLen,	 
	uchar *IEList,
	ushort IEFieldLen,
	uchar* p_ie_ids,
	ushort* p_total_size
);



extern uint8_t  PAN_ADVERT_SOLICIT_HEDR_IE_LIST [4];
extern uint8_t  PAN_ADVERT_SOLICIT_PAYLOAD_IE_LIST[6];

extern uint8_t PAN_ADVERT_HEDR_IE_LIST[4];
extern uint8_t PAN_ADVERT_PAYLOAD_IE_LIST[7];

extern uint8_t PAN_CONFIG_SOLICIT_HEDR_IE_LIST[4];
extern uint8_t PAN_CONFIG_SOLICIT_PAYLOAD_IE_LIST[6];
  
extern uint8_t PAN_CONFIG_HEDR_IE_LIST[5];
extern uint8_t PAN_CONFIG_PAYLOAD_IE_LIST[8];

extern uint8_t ULAD_PKT_HEDR_IE_LIST[7];
extern uint8_t ULAD_PKT_PAYLOAD_IE_LIST[8];

#if(FAN_EAPOL_FEATURE_ENABLED == 1)
extern uint8_t EAPOL_HEADR_IE_LIST[4];
extern uint8_t EAPOL_PAYLOAD_IE_LIST[5];
#endif

extern uint8_t ACK_HEADR_IE_LIST[6];
extern uint8_t ACK_PAYLOAD_IE_LIST[1]; 

extern void mem_rev_cpy(uint8_t* dest, uint8_t* src, uint16_t len );


#ifdef __cplusplus
}
#endif
#endif /* _FAN_MAC_IE_MANAGER_*/
