/** \file StackAppConf.h
 *******************************************************************************
 ** \brief  Contains all the application selection macros to select macro going 
 ** to use by selected aapplication.
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
 

#ifndef _CONFIG_PROJECT_APP_H_
#define _CONFIG_PROJECT_APP_H_

/*Inclusion of files*/
#include "StackBoardArchConf.h"
#include "StackCommonDefConf.h"

//#define EFM32_HFXO_FREQ                 	32000000




#if (APP_LBR_ROUTER) /*Application LBR and Router*/

        #define SICSLN
        #define CLOCK_USING_MAC_SYSTEM_TIMER
//        #define SICSLOWPAN_CONF_FRAG
        #define WISUN_FAN_MAC                                
        #define ROLL_RPL_UPPER_LAYER_DEFINED

#elif (RADIO_VALIDATION) /*Radio validation Application (T108)*/

        #define RUNTIME_SENDING_CONTROL_CONFIG
#if CONFIG_FOR_ENET_MAC 

#else
#if CONFIG_FOR_DEBUG_MAC
    //Not use WISUN_FAN_MAC
#else

     #define WISUN_FAN_MAC 
#endif
#endif


#elif (HIF_APP) /*HIF Application*/

#elif (AT_CMD) /*AT_Cmd Appliction*/

#elif (SNIFFER)	/*Sniffer Application*/
	
#elif (MAC_TEST) /*Mac test Application*/	
	
        #define RUNTIME_SENDING_CONTROL_CONFIG
        #define WISUN_FAN_MAC

#else   /*NONE Application defined*/

/*ERROR NO CONFIGURATION DEFINED*/

#endif 

void enter_crtical_section(void);
void exit_critical_section(void);

#endif //_CONFIG_PROJECT_APP_H_