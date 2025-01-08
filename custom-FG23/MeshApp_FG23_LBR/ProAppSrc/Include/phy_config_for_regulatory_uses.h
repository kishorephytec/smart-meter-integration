/** \file phy_config_for_regulatory_uses.h
 *******************************************************************************
 ** \brief  Provides the different PHY Configurtations for the regulatory regions
 **
 ** \cond STD_FILE_HEADER
 **
 ** COPYRIGHT(c) 2010-11 Procubed Technology Solutions Pvt Ltd. 
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
This file regulates the PHY configuration for 
different regulatory domain.
This PHY is for universal uses and we have to define the 
regulatory domain to be used in.

*/

//-----------------------------------------------------------------------------
// Header File Preprocessor Directive
//-----------------------------------------------------------------------------

#ifndef PHY_CONFIG_DEFS_H
#define PHY_CONFIG_DEFS_H

// Define  CFG_ONE_WATT_MODULE ==1 if you want to test 1Watt module for 800 MHz Band
#define CFG_ONE_WATT_MODULE                             0

// Define  CFG_800_MHZ_BAND ==1 if you want to test 20mw module for 800 MHz Band
#define CFG_800_MHZ_BAND                                1


#define MAX_REGULATORY_PHY_MODES_SUPPORTED              3  // FOR INDIA ..


#define CFG_GLOBAL_PHY_MODE_1                           1
#define CFG_GLOBAL_PHY_MODE_2                           1
#define CFG_GLOBAL_PHY_MODE_3                           1
#define CFG_GLOBAL_PHY_MODE_4                           1
#define CFG_GLOBAL_PHY_MODE_5                           1
#define CFG_GLOBAL_PHY_MODE_6                           1
#define CFG_GLOBAL_PHY_MODE_7                           1
#define CFG_GLOBAL_PHY_MODE_8                           1
#define CFG_GLOBAL_PHY_MODE_9                           1
#define CFG_GLOBAL_PHY_MODE_10                          1
#define CFG_GLOBAL_PHY_MODE_11                          1
#define CFG_GLOBAL_PHY_MODE_12                          1
#define CFG_GLOBAL_PHY_MODE_13                          1
#define CFG_GLOBAL_PHY_MODE_14                          1
#define CFG_GLOBAL_PHY_MODE_15                          1
#define CFG_GLOBAL_PHY_MODE_16                          1
#define CFG_GLOBAL_PHY_MODE_17                          1
#define CFG_GLOBAL_PHY_MODE_18                          1
#define CFG_GLOBAL_PHY_MODE_19                          1

   

#define MAX_GLOBAL_PHY_MODES_SUPPORTED			( CFG_GLOBAL_PHY_MODE_1\
                                                          + CFG_GLOBAL_PHY_MODE_2\
                                                          + CFG_GLOBAL_PHY_MODE_3\
                                                          + CFG_GLOBAL_PHY_MODE_4\
                                                          + CFG_GLOBAL_PHY_MODE_5\
                                                          + CFG_GLOBAL_PHY_MODE_6\
                                                          + CFG_GLOBAL_PHY_MODE_7\
                                                          + CFG_GLOBAL_PHY_MODE_8\
                                                          + CFG_GLOBAL_PHY_MODE_9\
                                                          + CFG_GLOBAL_PHY_MODE_10\
                                                          + CFG_GLOBAL_PHY_MODE_11\
                                                          + CFG_GLOBAL_PHY_MODE_12\
                                                          + CFG_GLOBAL_PHY_MODE_13\
                                                          + CFG_GLOBAL_PHY_MODE_14\
                                                          + CFG_GLOBAL_PHY_MODE_15\
                                                          + CFG_GLOBAL_PHY_MODE_16\
                                                          + CFG_GLOBAL_PHY_MODE_17\
                                                          + CFG_GLOBAL_PHY_MODE_18\
                                                          + CFG_GLOBAL_PHY_MODE_19)




#define   CFG_REGULATORY_DOMAIN_USA_CANADA                      0  
#define   CFG_REGULATORY_DOMAIN_USAV2_MEXICO                    0   
#define   CFG_REGULATORY_DOMAIN_BRAZIL                          0
#define   CFG_REGULATORY_DOMAIN_ANZ                             0
#define   CFG_REGULATORY_DOMAIN_EU3                             0
#define   CFG_REGULATORY_DOMAIN_PHILIPPINES                     0
#define   CFG_REGULATORY_DOMAIN_MALAYSIA                        0
#define   CFG_REGULATORY_DOMAIN_HK_SIG1_THAI_VIET               0
#define   CFG_REGULATORY_DOMAIN_KOREA                           0
#define   CFG_REGULATORY_DOMAIN_CHINA                           0
#define   CFG_REGULATORY_DOMAIN_JAPAN                           0
#define   CFG_REGULATORY_DOMAIN_EU1                             0
#define   CFG_REGULATORY_DOMAIN_EU2                             0
#define   CFG_REGULATORY_DOMAIN_INDIA                           1
#define   CFG_REGULATORY_DOMAIN_SINGAPORE2                      0



// This MACRO Definition will depends on how many country we support in single PHY build
// Suppose we define only one country than it will be deifned as 1 
#define MAX_REGULATORY_DOMAIN_SUPPORTED                 ( CFG_REGULATORY_DOMAIN_USA_CANADA\
                                                          + CFG_REGULATORY_DOMAIN_USAV2_MEXICO\
                                                          + CFG_REGULATORY_DOMAIN_BRAZIL\
                                                          + CFG_REGULATORY_DOMAIN_ANZ\
                                                          + CFG_REGULATORY_DOMAIN_EU3\
                                                          + CFG_REGULATORY_DOMAIN_PHILIPPINES\
                                                          + CFG_REGULATORY_DOMAIN_MALAYSIA\
                                                          + CFG_REGULATORY_DOMAIN_HK_SIG1_THAI_VIET\
                                                          + CFG_REGULATORY_DOMAIN_KOREA\
                                                          + CFG_REGULATORY_DOMAIN_CHINA\
                                                          + CFG_REGULATORY_DOMAIN_JAPAN\
                                                          + CFG_REGULATORY_DOMAIN_EU1\
                                                          + CFG_REGULATORY_DOMAIN_EU2\
                                                          + CFG_REGULATORY_DOMAIN_INDIA\
                                                          + CFG_REGULATORY_DOMAIN_SINGAPORE2)


#define PROFILE_CFG_WISUN_ROUREB                                1
#define PROFILE_CFG_WISUN_HAN                                   1
#define PROFILE_CFG_WISUN_EXTHAN                                1

#define PROFILE_CFG_WISUN_FAN                                   1

//#define PROFILE_CFG_DEFAULT                                     1


#if  (( PROFILE_CFG_WISUN_ROUREB == 1 ) || ( PROFILE_CFG_WISUN_HAN == 1 ) || ( PROFILE_CFG_WISUN_EXTHAN == 1 ))
#define PROFILE_CFG_WISUN_ENET_INTERFACE                        0
#endif



//-----------------------------------------------------------------------------
// Header File PreProcessor Directive
//-----------------------------------------------------------------------------
#endif                                 // #define PHY_CONFIG_DEFS_H

//-----------------------------------------------------------------------------
// End Of File
//------------------------------
