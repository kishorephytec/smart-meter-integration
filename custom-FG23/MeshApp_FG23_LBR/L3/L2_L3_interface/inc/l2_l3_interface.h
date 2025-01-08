/** \file l2_l3_interface.h
 *******************************************************************************
 ** \brief Provides information about the MAC Layer
 **
 ** \cond STD_FILE_HEADER
 **
 ** COPYRIGHT(c) 2010-11 Procubed Technology Solutions Pvt Ltd. 
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


#ifndef MAC_2_6LP_DATA_HANDLER_H
#define MAC_2_6LP_DATA_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "phy.h" 
#include "fan_config_param.h"
#include "mac_interface_layer.h" 


void mac_2_6lp_data_ind_cb
(
	mac_address_t*  pSrcaddr,
	mac_address_t*  pDstaddr,
	uint16_t msduLength,
	uint8_t* pMsdu,
	uint8_t mpduLinkQuality,
	uint8_t DSN,
        uint8_t pld_ies_present,
	uint32_t Timestamp,
	security_params_t* pSec 
);
void mac_2_6lp_data_conf_cb
(
	uint8_t msduHandle, 
	uint8_t status, 
	uint8_t NumBackoffs,
	uint32_t Timestamp
);

void mac_2_6lp_ack_ind_cb
(
  mac_address_t*  pSrcaddr,
  mac_address_t*  pDstaddr,
  uint8_t DSN,
  uint8_t rsl_value,
  uint8_t security_status
);

void mac_2_6lp_no_ack_ind_cb (void);


  
#ifdef __cplusplus
}
#endif
#endif /* MAC_2_6LP_DATA_HANDLER_H */
