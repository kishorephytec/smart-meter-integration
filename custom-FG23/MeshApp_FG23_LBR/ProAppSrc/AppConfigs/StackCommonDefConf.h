/** \file StackCommonDefConf.h
 *******************************************************************************
 ** \brief  Contains all the common arch. configuration macros. 
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
 
 #ifndef _STACK_COMMON_DEF_H_
#define _STACK_COMMON_DEF_H_

/*** Raka .. 28-Sept-2023 ********************/

//#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
#define FAN_FRQ_HOPPING_FEATURE_ENABLED         0


//#if(FAN_EDFE_FEATURE_ENABLED == 1)
#define FAN_EDFE_FEATURE_ENABLED         0

//#if(FAN_EAPOL_FEATURE_ENABLED == 1)
#define FAN_EAPOL_FEATURE_ENABLED  0

/*
Raka : Normal feature 

Configs
**************
FAN_UNICAT_SCEDULING_ENABLE_DISABLE
FAN_BROADCAST_SCHEDULING_ENABLE_DISABLE
FAN_CHANNEL_HOPPING_ENABLE_DISABLE
FAN_EAPOL_ENABLE_DISABLE
FAN_DHCPV6_ENABLE_DISABLE
802154_MAC_SECURITY_ENABLE_DISABLE
*/

//#if(APP_HIF_PROCESS_FEATURE_ENABLED == 1)
#define APP_HIF_PROCESS_FEATURE_ENABLED		0

//#if(APP_NWK_MANAGER_PROCESS_FEATURE_ENABLED == 1)
#define APP_NWK_MANAGER_PROCESS_FEATURE_ENABLED  0


//#if(APP_NVM_FEATURE_ENABLED == 1)
#define APP_NVM_FEATURE_ENABLED  0

//#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 1)
#define APP_EXTERNAL_FLASH_FEATURE_ENABLED  1




/*** Ending of Configs ******************/
/*** Raka .. 28-Sept-2023 ********************/

/*Selecting maximum frame size*/
#define MAX_FRAME_SIZE_CAPABILITY       			512


///*Enableing and Dissabling Hif interface for linux to run EAPOL and used in applicaton ,mac and eapol lib*/
///*'1' for EFM32GG512 & EFM32LG256 board & '0' for EFM32GG1024 board & without eapol*/
//#define HIF_INTERFACE_FOR_LINUX_RUNNING_EAPOL                   0       
//
//#if EFM32GG1024
//#define HIF_INTERFACE_FOR_LINUX_RUNNING_EAPOL                   0 
//#endif

                                                                            
/*this used for security and used in mac ,sicslowpan and eapol lib*/
#define ENABLE_FAN_MAC_WITHOUT_SECURITY                         0       /*0 for with security ,1 for without security*/

/*this is used for security in Mac ,sicslowpan and app */
#define MAC_CFG_SECURITY_ENABLED				1

/*this Macros used only for without eapol and without eapol with security*/
//#define WITHOUT_EAPOL	                                        1 /* 1 for without EAPOL 0 for WITH EAPOL*/

#define WITH_SECURITY                                           1 /*0 for without security ,1 for with security*/

#define WISUN_PROCUBED_DEVIATION                                1/*suneet :: */  

#define LOG_MAC_ADDRESS(addr) "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",addr[0],addr[1],addr[2],addr[3],addr[4],addr[5],addr[6],addr[7]

#define NONE_DEBUG              0
#define ERROR_DEBUG             1
#define CONSOLE_ERROR_DEBUG     2
#define RUNTIME_LOG_FILE        3
#define SHORT_PRINT             4

#define PRINT_DEBUG_LEVEL       NONE_DEBUG

#if (PRINT_DEBUG_LEVEL != NONE_DEBUG)
void stack_print_debug(const char *format, ...);
void print_mac_address (unsigned char *addr);
#endif

#if (PRINT_DEBUG_LEVEL == RUNTIME_LOG_FILE)
void send_runtime_log (char *format, ...);
#endif

#endif  // _STACK_COMMON_DEF_H_
