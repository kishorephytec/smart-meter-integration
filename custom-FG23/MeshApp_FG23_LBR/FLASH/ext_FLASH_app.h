/** \file ext_FLASH_app.h
 *******************************************************************************
 ** \brief This file has the function to store and retrieve data from External Flash
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

#ifndef EXT_FLASH_APP_API_H
#define EXT_FLASH_APP_API_H
   
#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 1)
   
#include "mx25flash_spi.h"

#ifdef __cplusplus
extern "C" {
#endif
  

  
  uint8_t FlashID_Test( void );
  uint8_t FlashReadWrite_Test( void );
  
  
  
  
  
#endif //#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 1) 
  
#ifdef __cplusplus
}
#endif



#endif /* EXT_FLASH_APP_API_H */
