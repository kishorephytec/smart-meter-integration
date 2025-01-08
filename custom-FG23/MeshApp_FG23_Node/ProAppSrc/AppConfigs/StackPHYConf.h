/** \file StackPHYConf.h
 *******************************************************************************
 ** \brief  Contains all the phy profile for selection of macros according to 
 ** phy profile going to use.
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

#ifndef _CONFIG_PROJECT_PHY_H_
#define _CONFIG_PROJECT_PHY_H_

/*Inclusion of files*/
#include "StackBoardArchConf.h"
#include "StackCommonDefConf.h"

//#define XTAL_FREQUENCY                  30000000
//#define SMD_MODULE


#if (CONFIG_FOR_DEBUG_PHY)      /*Configuration for PHY_Debug*/

        /*None*/

#elif (CONFIG_FOR_ENET_PHY)     /*Configuration for PHY_ENET*/

        #define WISUN_ENET_PROFILE
        #define ENET_MAC_FOR_GU
        #define CFG_PHY_BAND_920_928

#else   /*NONE PHY_configuration defined*/

/*ERROR NO CONFIGURATION DEFINED*/  

#endif //End of Configuration

#endif  //_CONFIG_PROJECT_PHY_H_
