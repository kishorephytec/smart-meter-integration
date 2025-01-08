/** \file mac_frame_parse.h
 *******************************************************************************
 ** \brief Provides APIs for parsing MAC frames
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

#ifndef MAC_FRAME_PARSE_H
#define MAC_FRAME_PARSE_H

#ifdef __cplusplus
extern "C" {
#endif

/*
** ============================================================================
** Public Macro definitions
** ============================================================================
*/

#ifdef WISUN_FAN_MAC
#define ACK_FIELD_UTIE		        19
#define MAX_ACK_HDR_IE_LIST_SIZE	50
#endif

/*
** ============================================================================
** Public Structures, Unions & enums Type Definitions
** ============================================================================
*/

/**
 *******************************************************************************
 ** \enum parse_result_t
 ** Structure to parse the result of the packet
 *******************************************************************************
 **/
typedef enum
{
    PARSE_STORED,		/**< parsing succeeded and message stored */
    PARSE_DISCARD,		/**< parsing succeeded, message to be discarded */
    PARSE_ERROR			/**< parsing error, message to be discarded */
} parse_result_t;

/*
** ============================================================================
** Public Variable Declarations
** ============================================================================
*/

#ifdef ENET_MAC_FOR_GU
  extern uint8_t set_wrong_dsn_in_ack;
  extern uint16_t delay_in_us;
#endif

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
 ** \brief Function called by the MAC to process a received packet
 ** \param *trxsm - pointer to the TRX sm buffer
 ** \param *pdp - pointer to the PHY buffer
 ** \param *mrp - pointer to the MAC buffer to store parsed data
 ** \retval - parse status
 ******************************************************************************/
parse_result_t mac_frame_parse(
                               trxsm_t *trxsm,
                               phy_rx_t *pdp, 
                               mac_rx_t *mrp
                               );


/**
 *******************************************************************************
 ** \brief Function to parse the addresses of the packet received
 ** \param *data - pointer pointing at 1st byte of MAC frame control header
 ** \param *dst - destination address 
 ** \param *src - source address 
 ** \retval - offset 
 ******************************************************************************/
int mac_frame_parse_addresses(
                              mac_rx_t *mrp,
                              uchar *data, 
                              mac_address_t *dst,
                              mac_address_t *src
                              );

#ifdef __cplusplus
}
#endif
#endif /* MAC_FRAME_PARSE_H */

