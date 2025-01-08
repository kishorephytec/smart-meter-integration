/** \file wisun_l3_modified.c
 *******************************************************************************
 ** \brief Provides APIs for MAC beacons
 **
 ** \cond STD_FILE_HEADER
 **
 ** COPYRIGHT(c) 2019-24 Procubed innovation pvt ltd. 
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

/*
********************************************************************************
* File inclusion
********************************************************************************
*/
#include "contiki-net.h"
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#include "contiki-net.h"


#if WISUN_PROCUBED_DEVIATION
void send_event_to_tcpip_process(void)
{
    l3_process_post_synch(&tcpip_process, NBR_SEND_NS_ARO_EVENT, NULL);  
}
#endif  //#if WISUN_PROCUBED_DEVIATION