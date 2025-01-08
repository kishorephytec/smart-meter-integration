/** \file common_function.h
 *******************************************************************************
 ** \brief  Contains all the Processor architecture selection macros for selection 
 ** of processor Architecture.
 **
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


#ifndef COMMON_FUNCTION_H
#define COMMON_FUNCTION_H

#include "contiki-net.h"

typedef struct comp_rssi{
	int val;
	linkaddr_t addr;
}comp_rssi_t;

void quickSort(comp_rssi_t arr[], int low, int high);
int max_payload(void);
#endif //COMMON_FUNCTION_H