/** \file StackBoardArchConf.h
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
 

#ifndef _ARCH_CONFIG_H_
#define _ARCH_CONFIG_H_

#define EFM32_TARGET_IAR


/*******************************************************************************
                          Application Profile Configuration
*******************************************************************************/


/*Select Configuration Profile For Application*/

//#define SUPPORT_EAPOL_SECURITY                                  0  /* 0: No EAPOL Security :: 1 : EAPOL Security Present*/
#define APP_LBR_ROUTER                                          1       /*Application LBR and Router*/
#define RADIO_VALIDATION                                        0       /*Radio validation Application (T108)*/
#define APP_GTK_HARD_CODE_KEY                                   1


/*******************************************************************************
                      Application Profile Configuration Ends.....
*******************************************************************************/

/*----------------------------------------------------------------------------*/

/*******************************************************************************
                          EAPOL Profile Configuration
*******************************************************************************/

/*Select Configuration Profile For EAPOL LIB which is used by WI-SUN Profile*/

//#define UART_BAUDRATE                                                   460800

/*******************************************************************************
                      EAPOL Profile Configuration Ends.....
*******************************************************************************/

/*----------------------------------------------------------------------------*/

/*******************************************************************************
                          DHCPv6 Profile Configuration
*******************************************************************************/

/*Select Configuration Profile For DHCPv6 LIB which is used by WI-SUN Profile*/

                                  /*None*/     

/*******************************************************************************
                      DHCPv6 Profile Configuration Ends.....
*******************************************************************************/

/*----------------------------------------------------------------------------*/

/*******************************************************************************
                          SicsLowPAN Profile Configuration
*******************************************************************************/

/*Select Configuration Profile For SicsLowPAN LIB which is used by WI-SUN Profile*/

/*Configuration for SicsLowPAN_Debug which is used for Wi-SUN FAN Profile*/
#define CONFIG_FOR_DEBUG                                        0       
/*Configuration for SicsLowPAN_ENET_IPv6 which is used for Wi-SUN FAN Profile*/
#define CONFIG_FOR_ENET_IPv6_GU                                 1       
/*Configuration for SicsLowPAN_LEAF_NODE which is used for Wi-SUN FAN Profile*/
#define CONFIG_FOR_IPv6_RPL_LEAF_NODE                           0            

/*******************************************************************************
                      SicsLowPAN Profile Configuration Ends.....
*******************************************************************************/

/*----------------------------------------------------------------------------*/

/*******************************************************************************
                          MAC Profile Configuration
*******************************************************************************/

/*Select Configuration Profile For MAC LIB which is used by WI-SUN Profile*/

/*Configuration for ENET_MAC which is used for Wi-SUN FAN Profile*/
#define CONFIG_FOR_ENET_MAC                               0     
/*Configuration for ENET_MAC_GU which is used for Wi-SUN FAN Profile*/
#define CONFIG_FOR_ENET_MAC_GU                            0     
/*Configuration for DEBUG_MAC which is used for Wi-SUN FAN Profile*/
#define CONFIG_FOR_DEBUG_MAC                              0    
/*Configuration for FAN_MAC which is used for Wi-SUN FAN Profile*/
#define CONFIG_FOR_FAN_MAC                                1     
/*Configuration for 802_15_4_MAC which is used for Wi-SUN FAN Profile*/
#define CONFIG_FOR_802_15_4_MAC                           0    

/*******************************************************************************
                      MAC Profile Configuration Ends.....
*******************************************************************************/

/*----------------------------------------------------------------------------*/

/*******************************************************************************
                          PHY Profile Configuration
*******************************************************************************/

/*Select Configuration Profile For PHY LIB which is used by WI-SUN Profile*/

/*Configuration for PHY_Debug which is used for Wi-SUN FAN Profile*/
#define CONFIG_FOR_DEBUG_PHY                              1     
/*Configuration for PHY_ENET which is used for Wi_SUN ENET Profile */
#define CONFIG_FOR_ENET_PHY                               0     

/*******************************************************************************
                      PHY Profile Configuration Ends.....
*******************************************************************************/

/*----------------------------------------------------------------------------*/


#endif //_ARCH_CONFIG_H_