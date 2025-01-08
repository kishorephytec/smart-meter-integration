/** \file aap_mac_security_interface.c
 *******************************************************************************
 ** \brief Application Interface Layer to set the 802.15.4 MAC Security .
 ** \defines  
 **
 ** \cond STD_FILE
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

/*
********************************************************************************
* File inclusion
********************************************************************************
*/

#include "StackAppConf.h"
#include "common.h"
//#include "board.h"
#include "list_latest.h"
#include "queue_latest.h"
#include "mac.h"
#include "mac_interface_layer.h"
#include "sm.h"
#include "ie_element_info.h"
#include "network-manager.h"
#include "fan_mac_security.h"

/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/

#ifdef MAC_CFG_SECURITY_ENABLED
#define MAX_NO_OF_MAC_SECURITY_DEVICE_LIST  5

#define DEF_KEY_SRC                      0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF

#define DEF_KEY_USAGE_DESC               0x00, 0x00, 0x01, 0x00, 0x02, 0x00, \
                                         0x03, 0x01, 0x03, 0x02, 0x03, 0x03, \
                                         0x03, 0x04, 0x03, 0x05, 0x03, 0x06, \
                                         0x03, 0x07, 0x03, 0x08, 0x03, 0x09


#define MAX_KET_TABLE_DATA_BUFF 120 

/*
** =============================================================================
** Private Structures, Unions & enums Type Definitions
** =============================================================================
**/

/* None */

/*
** =============================================================================
** Private Variable Definitions
** =============================================================================
*/

/* None */

/*
** =============================================================================
** Private Function Prototypes
** =============================================================================
*/

/*Umesh :31-01-2018*/
//static uint8_t indexofDeviceDes = 0;
/*this varriable is not used*/

/*
** =============================================================================
** Private Function Definitions
** =============================================================================
*/

/* None */

/*
** =============================================================================
** External Variable Declarations
** =============================================================================
*/
extern fan_mac_security mac_key_list;
extern uint8_t key_id_index;

/*
** =============================================================================
** External Function Prototypes
** =============================================================================
*/

/* None */

/*
** =============================================================================
** Public Variables Definitions
** =============================================================================
**/

/* None */

/*
** =============================================================================
** Public Function Prototypes
** =============================================================================
*/
    
void add_dev_desc_on_MAC_for_security(uint8_t* macAddrOfNeighbour);
/*Umesh : 31-01-2018*/
void  set_mac_frame_counter_cmd ( uint32_t frameCntr );

extern uint8_t find_device_already_added_or_not(uint8_t *device_addr, uint8_t *dev_index);
/*this function is not called from anywhere*/
/*
** =============================================================================
** Public Function Definitions
** =============================================================================
*/

/*----------------------------------------------------------------------------*/
/*\desc
**\param
**\return
*/
/*----------------------------------------------------------------------------*/
void reset_mac_frame_counter_cmd ( void )
{		
    uint32_t frameCntr = 0;
    /*set the SECURITY to ON or OFF*/
    MLME_SET_Request
    (
            macFrameCounter,		
            0,
            0x0004,	
            &frameCntr	
    );
    		
}

/*----------------------------------------------------------------------------*/
/*\desc
**\param
**\return
*/
/*----------------------------------------------------------------------------*/      
void  set_mac_frame_counter_cmd ( uint32_t frameCntr )
{		
  //  uint32_t frameCntr = 0;
    /*set the SECURITY to ON or OFF*/
    MLME_SET_Request
    (
            macFrameCounter,		
            0,
            0x0004,	
            &frameCntr	
    );  		
}
/*----------------------------------------------------------------------------*/
/*\desc
**\param
**\return
*/
/*----------------------------------------------------------------------------*/
void  enable_disable_mac_security_cmd ( uint32_t enable_security_flag )
{		

    /*set the SECURITY to ON or OFF*/
    MLME_SET_Request
    (
            macSecurityEnabled,		
            0,
            0x0001,	
            &enable_security_flag	
    );
    		
}
/*----------------------------------------------------------------------------*/
/*\desc
**\param
**\return
*/
/*----------------------------------------------------------------------------*/
void add_security_key_descriptor_on_MAC()     
{
  /*these macros are moved to private macro portion*/                                          
        uint8_t key_table_data[MAX_KET_TABLE_DATA_BUFF] = {0};    
       // uint8_t key_usage_desc[24] = {DEF_KEY_USAGE_DESC};
      uint16_t mac_sec_keylen = 0x00;
      uint8_t total_mac_key_available = 0;
          //uint8_t iCnt = 0;
          uint8_t keyIDX = mac_key_list.active_key_index;///* 0x01;*/ (key_id_index +1) ; 
          uint8_t* pBuff = NULL;
          uint8_t key_source[8] = {DEF_KEY_SRC};       
          pBuff = key_table_data;
          
          for(int i = 0; i<MAX_MAC_SECURITY_KEY;i++)
          {
            if(mac_key_list.MAC_SECURITY_KEY_LIST[i].MAC_KEY_INDEX != 0)
            {
              total_mac_key_available++;
            }
          }
          
          /* KeyIdLookupList  */
          *pBuff++ = total_mac_key_available; 
          mac_sec_keylen++;
          for(int i = 0; i<total_mac_key_available; i++)
          {
             /* 8 Bytes of key source :: LookupData*/
            mem_rev_cpy(pBuff, key_source, KEY_SOURCE_LENGTH);
            pBuff+= KEY_SOURCE_LENGTH;
            mac_sec_keylen += KEY_SOURCE_LENGTH;
             /* Key index  */
            *pBuff++ = keyIDX;//temprary patch for without eapol with security
            keyIDX++;
            if (keyIDX > total_mac_key_available)
              keyIDX = 1;
            mac_sec_keylen++;
          }
           /* LookupDataSize
            0x000 :: for 5 Bytes keysource 
            0x01 ::: for 9 Bytes keysource */
//          *pBuff++ = 0x01; 
//          mac_sec_keylen++;
          /* KeyDeviceList :: No of key device descriptors */
          /*KeyIdLookupListEntries*/
            *pBuff++ = total_mac_key_available; //[ Raka : 19-Nov-2017]
            mac_sec_keylen++;
          /* Key device desc */
//          *pBuff++ = 0x00; //Device descriptor handle
//          *pBuff++ = 0x00;// Unique device
//          *pBuff++ = 0x00; //Black listed device or not
          
          
//          for(iCnt = 0; iCnt < MAX_NO_OF_MAC_SECURITY_DEVICE_LIST; iCnt++)
//          {
//            *pBuff++ = iCnt;  //0x00; //Device descriptor handle
//            *pBuff++ = 0x00; // Unique device
//            *pBuff++ = 0x00; //Black listed device or not
//          }
//          
          /* No of key usage descriptor */
//          *pBuff++ = 0x0C; // Frame type and cmd id
          
//          memcpy(pBuff, key_usage_desc, 0x18); // Frame type(1Byte) and cmd id(1Byte)
//          pBuff += 0x18;
         for(int i = 0; i<total_mac_key_available; i++)
         {
          memcpy(&pBuff[i*17],&mac_key_list.MAC_SECURITY_KEY_LIST[i].MAC_KEY_INDEX, 17);
          mac_sec_keylen += 17;
         }
          
          
          MLME_SET_Request
          (
              macKeyTable,
              0x00,
              mac_sec_keylen,
              &key_table_data[0]
          );    

}
/*----------------------------------------------------------------------------*/
/*\desc: updating the mac_device_table
**\param: no param
**\return: nothing
*/

/*----------------------------------------------------------------------------*/
void print_mac_address (uint8_t *addr);
void add_dev_desc_on_MAC_for_security(uint8_t* macAddrOfNeighbour)
{  
    uint8_t dev_desc_table[20] = {0};
    uint8_t device_idx = 0;

    if (find_device_already_added_or_not (macAddrOfNeighbour, &device_idx) == 1)
    {
      return;
    }

//    uint32_t short_addr = 0x0102; // Any Dummy Value
    uint8_t *pBuff = dev_desc_table ; 
//    memcpy(pBuff, (uint8_t*)&fan_nwk_manager_app.node_basic_cfg.selected_pan_id, 0x02);  

//    pBuff += 2;
//    memcpy(pBuff, (uint8_t*)&short_addr, 0x02);
//    pBuff += 2;
    mem_rev_cpy (pBuff, macAddrOfNeighbour, 0x08);
    pBuff += 8;
    memset (pBuff, 0x00, 0x04); // Frame counter - 4 bytes 
    MLME_SET_Request
    (
        macDeviceTable,
        device_idx,
        0x0001,
        &dev_desc_table[0]
    );
        

}
      
#endif
      
/*----------------------------------------------------------------------------*/


