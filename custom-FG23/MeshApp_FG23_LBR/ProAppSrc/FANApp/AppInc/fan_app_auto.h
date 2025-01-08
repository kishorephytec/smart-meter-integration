/** \file fan_app_auto.h
 *******************************************************************************
 ** \brief It starts the network authomatically by hardcoding all the variables.
 **
 **  This file contains the public APIs and structures for FAN Testharness header 
 **   module.
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

#ifndef _FAN_APP_AUTO_HARNESS_H_
#define _FAN_APP_AUTO_HARNESS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
** ============================================================================
** Public Macro definitions
** ============================================================================
*/
  
/* None */
  
/*
** ============================================================================
** Public Structures, Unions & enums Type Definitions
** ============================================================================
*/ 



#if(AUTO_CONFIG_ENABLE == 1)
  


/*
** ============================================================================
** Public Variable Declarations
** ============================================================================
*/



/*
** ============================================================================
** Public Function Prototypes
** ============================================================================
*/
  
void configure_device_run_param();
  
#endif

#ifdef __cplusplus
}
#endif
#endif /* _FAN_APP_AUTO_HARNESS_H_ */
