/** \file rail_timer_interface.h
 *******************************************************************************
 ** \brief Provides details about the MAC PIBs
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
 
 
#ifndef RAIL_TIMER_INTERFACE_H
#define RAIL_TIMER_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif
  
#include "rail.h"   
#include "rail_types.h"
extern RAIL_Handle_t grailHandlePHY;

bool rail_start_timer(uint32_t period, void *call_back);
uint32_t get_currunt_time_from_rail();

#ifdef __cplusplus
}
#endif
#endif /* RAIL_TIMER_INTERFACE_H */