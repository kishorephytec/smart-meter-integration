/** \file fan_config_param.h
 *******************************************************************************
 ** \brief It Configures the parameter used in the fan network.
 **
 **  This file contains the administrative configured parameter for FAN Network 
 **   
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

#ifndef _FAN_CONFIG_PARAM_H_
#define _FAN_CONFIG_PARAM_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
** ============================================================================
** FAN Config Parameter defination...
** ============================================================================
*/
  
/*
6.3	Data Link Layer
6.3.1 	Constants

DEFAULT_DIO_REDUNDANCY_CONSTANT The default value for the DIO  redundancy constant	1
DISC_K	Discovery Trickle timer k parameter	1
GTK_LIFETIME_DEFAULT	The default lifetime for GTKs	30 days. NOTE: estimating a 1 ms frame transmission rate by an ideal node, a 32 bit frame counter provides for a 49 day maximum lifetime.
PRC_WEIGHT_FACTOR 	Routing Cost Weighting factor	256
PS_WEIGHT_FACTOR 	PAN Size Weighting factor	64
PCS_MAX 	PAN Configuration Solicit Attempts	5
Wi-SUN-OUI	Organizationally Unique Identifier of the Wi-SUN Alliance. 	 0x0C5A9E
*/
#define EFR32FG13P_LBR                                          1
#define APP_CFG_DEFAULT_DIO_REDUNDANCY_CONSTANT                 0	/* Debdeep :: DIO should not be suppressed */
#define APP_CFG_DISC_K                                          1
#define APP_CFG_GTK_LIFETIME_DEFAULT                            30 /*In Days*/
#define APP_CFG_PRC_WEIGHT_FACTOR                               256
#define APP_CFG_PS_WEIGHT_FACTOR                                64
#define APP_CFG_PCS_MAX                                         5
#define APP_CFG_WiSUN_OUI                                      0x0C5A9E
#define APP_CFG_PAN_TIMEOUT                                     (30*60) /* Seconds */

/* Debdeep added these macro for parent selection in EAPOL and RPL */
#define APP_CFG_CAND_PARENT_THRESHOLD                           10
#define APP_CFG_CAND_PARENT_HYSTERESIS                          3
  
#define APP_CFG_RSSI_BAND                                       40

#if (EFM32GG512 == 1)
#define APP_CFG_MAX_DEV_SUPPORT                                 15
#elif(EFR32FG13P_LBR == 1)
#define RPL_CFG_MAX_ROUTE_SUPPORT                               40 
#define APP_CFG_MAX_DEV_SUPPORT                                 7
#else
#define APP_CFG_MAX_DEV_SUPPORT                                 2
#endif

#define APP_CFG_MAC_MAX_DEV_SUPPORT                             7
#define ENABLE_DISABLE_DHCP_SERVICE                             0
  
/*  
 6.3.1.1	Configuration Parameters 
  

Name	                                Description                                     Range	           Recommended Default Value
DEFAULT_DIO_INTERVAL_DOUBLINGS 	The DIO trickle timer Imax value.                       1-8 doublings        1
                            Large network - 1 (an Imax of approximately 16 minutes)
                            Small network - 2 (an Imax of approximately 128 seconds)
		
DEFAULT_DIO_INTERVAL_MIN  The DIO trickle timer Imin value is defined                   1-255                19
                          as 2 to the power of this value in millseconds.
                          Large network - 19 (an Imin of approximately 8 minutes)
                          Small network - 15 (an Imin of approximtety 32 seconds)
		
DISC_IMIN	          Discovery Trickle Timer Imin                                  1-255 seconds        60
                          Large network - 60 sec
                          Small network - 15 sec	
	
DISC_IMAX	        Discovery Trickle timer Imax                                    1-8 doublings        4
                        Large network - 4  (max Trickle interval = approximately 16 minutes)
                        Small network - 2 (max Trickle interval = approximately 60 sec).		
	
DATA_MESSAGE_IMIN	[MPL] trickle timer Imin value	                                1-255 seconds	      10

DATA_MESSAGE_IMAX	[MPL] trickle Imax value	                                1-8 doublings	      3 (3 doublings of Imin == 80 seconds).

PAN ID	                The PAN ID of the PAN, configured at the Border Router	        0-65535	              None – must be unique per PAN in a network

Network Name	        Configured on each node to identify a given network.  	        0-32 ASCII characters	None – must be unique per network
Routing Method	        The routing method of the PAN	                                0 – Layer 2 routing     1
                                                                                        1 - Layer 3 routing
*/

#define APP_CFG_DEFAULT_DIO_INTERVAL_DOUBLINGS        2
#define APP_CFG_DEFAULT_DIO_INTERVAL_MIN              15
#define APP_CFG_DISC_IMIN                             15
#define APP_CFG_DISC_IMAX                             2
#define APP_CFG_DATA_MESSAGE_IMIN                     10
#define APP_CFG_DATA_MESSAGE_IMAX                     3

#ifdef __cplusplus
}
#endif
#endif /* _FAN_CONFIG_PARAM_H_ */
