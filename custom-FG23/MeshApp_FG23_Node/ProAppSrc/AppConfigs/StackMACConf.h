/** \file StackMACConf.h
 *******************************************************************************
 ** \brief  Contains all the MAC profile for selection of macros according to 
 ** MAC profile going to use.
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

#ifndef _CONFIG_PROJECT_MAC_H_
#define _CONFIG_PROJECT_MAC_H_

/*Inclusion of files*/
#include "StackBoardArchConf.h"
#include "StackCommonDefConf.h"






#define RUNTIME_SENDING_CONTROL_CONFIG

#if  (CONFIG_FOR_ENET_MAC)      /*Configuration for ENET_MAC*/

        #define WISUN_ENET_PROFILE

#elif (CONFIG_FOR_ENET_MAC_GU)  /*Configuration for ENET_MAC_GU*/

        #define WISUN_ENET_PROFILE
        #define ENET_MAC_FOR_GU
        #define PANA_FOR_SINGLE_HOP_HAN

#elif (CONFIG_FOR_DEBUG_MAC)    /*Configuration for DEBUG_MAC*/

		/*None*/

#elif (CONFIG_FOR_FAN_MAC)/*Configuration for FAN_MAC*/
        //#define WISUN_ENET_PROFILE
//        #define SICSLOWPAN_CONF_FRAG
        #define WISUN_FAN_MAC 

#elif (CONFIG_FOR_802_15_4_MAC)

        #define SICSLOWPAN_CONF_FRAG

#else   /*NONE MAC_configuration defined*/

/*ERROR NO CONFIGURATION DEFINED*/

#endif  


#endif  //_CONFIG_PROJECT_MAC_H_


