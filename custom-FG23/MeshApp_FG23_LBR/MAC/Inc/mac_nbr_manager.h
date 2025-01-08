/** \file mac_nbr_manager.c
 *******************************************************************************
 ** \brief 
 **
 ** \cond 
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

#ifndef MAC_NBR_MGR_H
#define MAC_NBR_MGR_H

#ifdef __cplusplus
extern "C" {
#endif

uint8_t* store_mac_nbr_descriptor_entry_in_pib
( 
  uint8_t ie_type,
  uint8_t *mrp,
  uint16_t msdu_length_recv,
  /*uint8_t* up_addr,*/
  mac_address_t*  pSrcaddr,
  uint16_t* msduLength
);
#define PRC_WEIGHT_FACTOR               APP_CFG_PRC_WEIGHT_FACTOR
#define PS_WEIGHT_FACTOR                APP_CFG_PS_WEIGHT_FACTOR
#ifdef WISUN_FAN_MAC
uchar store_mac_nbr_descriptor_entry_for_async_pkt( mac_rx_t *mrp );
uint8_t add_device_fan_mac_nbr(uint8_t* mac_addr);
#endif
#endif