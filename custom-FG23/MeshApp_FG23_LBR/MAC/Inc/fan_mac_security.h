/** \file fan_mac_security.h
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

#ifndef FAN_MAC_SECURITY_H
#define FAN_MAC_SECURITY_H

#ifdef __cplusplus
extern "C" {
#endif



#define MAX_MAC_SECURITY_KEY            4  
 
typedef struct mac_secrity_info_tag
{
  uint8_t MAC_KEY_INDEX;
  uint8_t MAC_SECURITY_KEY[16];
}mac_secrity_info_t;
  
typedef struct fan_mac_security_tag
{
  mac_secrity_info_t MAC_SECURITY_KEY_LIST[MAX_MAC_SECURITY_KEY];
  uint8_t active_key_index;
}fan_mac_security;

void reset_mac_frame_counter_cmd ( void );



#endif  //FAN_MAC_SECURITY_H