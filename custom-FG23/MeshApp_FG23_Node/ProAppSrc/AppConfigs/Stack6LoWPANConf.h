/** \file Stack6LoWPANConf.h
 *******************************************************************************
 ** \brief Contains all the SicsLowPan profile for selection of macros according 
 ** to SicsLowPan profile going to use.
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

#ifndef _CONFIG_PROJECT_SICSLOWPAN_H_
#define _CONFIG_PROJECT_SICSLOWPAN_H_

/*Inclusion of files*/
#include "StackBoardArchConf.h"
#include "StackCommonDefConf.h"

#define SICSLN
#define WISUN_ENET_PROFILE
#define SICSLOWPAN_CONF_FRAG							1					

#if (CONFIG_FOR_DEBUG)  /*Configuration for SicsLowPAN_Debug*/

				/*None*/

#elif (CONFIG_FOR_ENET_IPv6_GU) /*Configuration for SicsLowPAN_ENET_IPv6*/
     
        #define ENET_GOLDEN_UNIT
        #define WISUN_FAN_MAC

#elif (CONFIG_FOR_IPv6_RPL_LEAF_NODE)   /*Configuration for SicsLowPAN_LEAF_NODE*/


        #define RPL_CONF_LEAF_ONLY      1
        #define ENET_GOLDEN_UNIT


#else   /*NONE SICSLOWPAN_configuration defined*/

/*ERROR NO CONFIGURATION DEFINED*/

#endif  


#endif  //_CONFIG_PROJECT_SICSLOWPAN_H_