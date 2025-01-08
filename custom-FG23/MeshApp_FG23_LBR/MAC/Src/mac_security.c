/** \file mac_security.c
 *******************************************************************************
 ** \brief 
 ** Implements the PHY main function
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

/*******************************************************************************
* File inclusion
*******************************************************************************/
#include "StackMACConf.h"
#include "common.h"
#include "queue_latest.h"
#include "buff_mgmt.h"
#include "event_manager.h"
#include "phy.h"
#include "mac_config.h"
#include "mac.h"
#include "mac_defs_sec.h"

#ifdef MAC_CFG_SECURITY_ENABLED
#include "ie_element_info.h"
#include "mac_defs.h"
#include "mac_pib.h"
#include "ccmstar.h"
#include "mac_security.h"
#include "fan_mac_security.h"
#include "sm.h"
#include "fan_mac_interface.h"


#ifdef WISUN_FAN_MAC

#if ((FAN_EAPOL_FEATURE_ENABLED == 1) || (APP_GTK_HARD_CODE_KEY == 1))
#include "sha256.h"     /****RAka**Temperory Since EAPOL is not present [ 14-June-2017]*/
#endif

#endif

/*
** ============================================================================
** Private Macro definitions
** ============================================================================
*/

/* None */

/*
** ============================================================================
** Private Structures, Unions & enums Type Definitions
** ============================================================================
*/

/* None */

/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/

const uint8_t m_lut[4] = {0, 4, 8, 16};
//int sec_stuff_length( security_params_t* );
static int count = 0;

/*
** ============================================================================
** Public Variable Definitions
** ============================================================================
*/

//uint8_t MACSecKey1[16] = {0};/****RAka**Temperory Since EAPOL is not present [ 14-June-2017]*//*Umesh : 15-01-2018 moved to \
                                  fan_mac_security.c file*/
mac_security_data_t mac_security_data;
uint8_t send_wrong_mic = 0x00;

/*
** ============================================================================
** Public Function Prototypes
** ============================================================================
*/

/****RAka**Temperory Since EAPOL is not present [ 14-June-2017]*/
uint8_t generate_MAC_Security_Key (uint8_t live_gtk_key_index,uint16_t len);

/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/
#ifdef WISUN_FAN_MAC 
extern self_info_fan_mac_t mac_self_fan_info;
#if(FAN_EDFE_FEATURE_ENABLED == 1)
extern edfe_info_t edfe_information;  
#endif
#endif
extern mac_pib_t mac_pib;
extern uchar heap[];
extern void add_dev_desc_on_MAC_for_security(uint8_t* macAddrOfNeighbour);
extern void mem_rev_cpy(uint8_t* dest, uint8_t* src, uint16_t len );
extern void * app_bm_alloc(
    uint16_t length//base_t length       
    );

#ifdef WISUN_FAN_MAC     
extern fan_mac_security mac_key_list;
#endif

extern void app_bm_free(
    uint8_t *pMem      
    );
    
/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/

static uint8_t Apply_CCM_Star_Encrypt( security_struct_t* p_security_data );
static uint8_t Apply_CCM_Star_Decrypt( security_struct_t* p_security_data );
uint8_t find_device_already_added_or_not(uint8_t *device_addr, uint8_t *dev_index);
/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

void sec_list_initialise( void )
{  
    queue_initialise(&mac_security_data.pib.mac_key_table);
    queue_initialise(&mac_security_data.pib.mac_device_table);
    queue_initialise(&mac_security_data.pib.mac_security_level_table);
}

/******************************************************************************/

//uchar device_descriptor_match_procedure( device_descriptor_t *pDD, uchar *lookup_data, uchar lookup_size )
//{
//    uchar buf[4] = {0};
//
//    if ((pDD == NULL_POINTER) || (lookup_data == NULL_POINTER))
//        return(FALSE);
//
//    /* Compare 4 bytes of Lookup_data with the PAN ID and Short Address */
//    buf[0] = pDD->pan_id >> 8;
//    buf[1] = pDD->pan_id & 0xff;
//    buf[2] = pDD->short_addr >> 8;
//    buf[3] = pDD->short_addr & 0xff;
//
//    if (( lookup_size == 4 ) && (!memcmp(lookup_data,buf,4)))
//	{
//		return(TRUE);
//	}
//
//    if (( lookup_size == IEEE_ADDRESS_LENGTH ) && (!memcmp(lookup_data,pDD->ieee_addr,IEEE_ADDRESS_LENGTH)))
//	{
//        return(TRUE);
//	}
//       return(FALSE);
//}

/******************************************************************************/

uchar security_level_check( uchar incoming_security_level, uchar frame_type, uchar command_frame_identifier )
{
    uchar i = 0 ;
    security_level_descriptor_t *pSLD = NULL;

    /* Check Every entry in the Security Level Table */
    for (i=0; i<mac_security_data.pib.mac_security_level_table_entries; i++)
    {   
        pSLD = (security_level_descriptor_t*) queue_item_read_from( (queue_t *)&mac_security_data.pib.mac_security_level_table, i );

        if ( pSLD != NULL_POINTER )
        {
        
#define ENCRYPTION_MASK         0x4
#define SECURITY_LEVEL_MASK     0x3

            if ( frame_type != MAC_FRAME_TYPE_MAC_COMMAND ) 
            {
                if (frame_type == pSLD->frame_type)
                {         
                    /* Check if the incoming Security level is greater than or equal to that stored in the table */
                    if ((( incoming_security_level & ENCRYPTION_MASK) >=  (pSLD->security_minimum & ENCRYPTION_MASK))
                        && (( incoming_security_level & SECURITY_LEVEL_MASK) >=  (pSLD->security_minimum & SECURITY_LEVEL_MASK))) 
                    {
					    return(PASSED);
                    }
                    else
                    {
                        if ((pSLD->device_overide_security_minimum == TRUE) && (incoming_security_level == 0))
                        {
                            return(CONDITIONALLY_PASSED);/* conditional Pass */
                        }
                        else
                        {
                            return(FAILED);
                        }
                    }
                }
            }
            else 
            {
                if ((frame_type == pSLD->frame_type) && (command_frame_identifier == pSLD->command_frame_identifier ))
                {
                    /* Check if the incoming Security level is greater than or equal to that stored in the table */
                    if ((( incoming_security_level & ENCRYPTION_MASK) >=  (pSLD->security_minimum & ENCRYPTION_MASK))
                        && (( incoming_security_level & SECURITY_LEVEL_MASK) >=  (pSLD->security_minimum & SECURITY_LEVEL_MASK))) 
                    {
					    return(PASSED);
                    }
                    else
                    {
                        if ((pSLD->device_overide_security_minimum == TRUE) && (incoming_security_level == 0))
                        {
                            return(CONDITIONALLY_PASSED);
                        }
                        else
                        {
                            return(FAILED);
                        }
                    }
                }
            } /* if ( frame_type != 3 ) */
        } /* pSLD != NULL_POINTER */
    }   /* for() */
    return(PASSED);
}


/******************************************************************************/
device_descriptor_t *get_device_descriptior_from_pib (uchar *lookup_data)
{
  uint8_t ii = 0;
  uint8_t entries = mac_security_data.pib.mac_device_table_entries;
  device_descriptor_t *dev = NULL;
  
  if (lookup_data == NULL)
    return NULL;
  
  for (ii = 0; ii < entries; ii++)
  {
    dev = (device_descriptor_t*)queue_item_read_from (&mac_security_data.pib.mac_device_table, ii);
    if (dev == NULL)
      continue; //Should not happen
    
    if (0 == memcmp (dev->ieee_addr, lookup_data, IEEE_ADDRESS_LENGTH))
      return dev;
  }
  
  uint8_t dev_info[20] = {0};
  memcpy (dev_info, lookup_data, IEEE_ADDRESS_LENGTH);
  memset (dev_info + IEEE_ADDRESS_LENGTH, 0, 4);        /* Making frame counter 0 while adding dev to PIB */
  store_device_table_entry_in_pib (dev_info, entries);
  
  dev = (device_descriptor_t*)queue_item_read_from (&mac_security_data.pib.mac_device_table, entries);
  if (dev == NULL)
    return NULL; //Should not happen
  return dev;
}

uchar blacklist_checking_procedure
( 
 key_descriptor_t *pKD, 
 uchar *lookup_data, 
 uchar lookup_size, 
 key_device_descriptor_t **pKDD, /* KeyDeviceDescriptor to be returned */
 device_descriptor_t **pDD      /* DeviceDescriptor to be returned */
 )
{
    uchar i = mac_security_data.pib.mac_device_table_entries;
    
    uchar address_is_available = 0x00;
//    uint8_t add_dev_reg[8] = {0};
    device_descriptor_t *temp;
    if ((pKD == NULL_POINTER) || (lookup_data == NULL_POINTER))
        return(FALSE);

     for(uchar j = 0;j<=i;j++)
     {
             temp =  (device_descriptor_t*) queue_item_read_from( (queue_t *)&mac_security_data.pib.mac_device_table, 
                                                               j);
             if (temp != NULL_POINTER)
             {
               if(!memcmp(lookup_data,
                           temp->ieee_addr,
                           lookup_size))
                {
                  address_is_available = 0x01;
                  *pDD = (device_descriptor_t*) queue_item_read_from( (queue_t *)&mac_security_data.pib.mac_device_table, 
                                             j);
                  break;
                }
              }
     }      
    if(address_is_available == 0x00)
    {
        uint8_t dev_desc_table[20] = {0};
        uint32_t short_addr = 0x0102; // Any Dummy Value
        uint8_t *pBuff = dev_desc_table ; 
#ifdef WISUN_FAN_MAC         
        memcpy(pBuff, (uint8_t*)&mac_self_fan_info.pan_metrics.pan_id, 0x02);  
#endif
        pBuff += 2;
        memcpy(pBuff, (uint8_t*)&short_addr, 0x02);
        pBuff += 2;
        memcpy(pBuff, lookup_data, 0x08);
        pBuff += 8;
        memset(pBuff, 0x00, 0x05); // Frame counter - 4 bytes and Exempt(0-False, 1-True) - 1 Byte   
        store_device_table_entry_in_pib(&dev_desc_table[0],i);
        *pDD = (device_descriptor_t*) queue_item_read_from( (queue_t *)&mac_security_data.pib.mac_device_table, 
                                             i);
    }
#if 0    //Security re  
    /* Check each KeyDeviceDescriptor in the KeyDeviceList of the Key Descriptor */
    for(i=0; i<pKD->key_device_list_entries; i++) //Suneet :: pKD->key_device_list_entries add only one key_device_list_entries loop run only one time 
    {
        /* Get the KeyDeviceDescriptor */
        *pKDD = (key_device_descriptor_t*) queue_item_read_from( (queue_t *)&pKD->key_device_list, i );

        if (*pKDD != NULL_POINTER)
        {
            /* Get the DeviceDescriptor */
            *pDD = (device_descriptor_t*) queue_item_read_from( (queue_t *)&mac_security_data.pib.mac_device_table, 
                                                     (*pKDD)->device_table_entry_handle);

            if (*pDD != NULL_POINTER)
            {
                if ((*pKDD)->unique_device == TRUE)
                {
                    if ((*pKDD)->black_listed == TRUE)
                    {
                        return(FALSE); 
                    }
                    else
                    {
                        return(TRUE); 
                    }
                }
                else
                {
                    /* Check if the device matches the one we want */
                    if (device_descriptor_match_procedure( *pDD, lookup_data, lookup_size))
                    {
                        if ((*pKDD)->black_listed == TRUE)
                        {
                            return(FALSE); 
                        }
                        else
                        {
                            return(TRUE); 
                        }
                    }
                }
            } /* if (*pDD != NULL_POINTER) */
        } /* if (*pKDD != NULL_POINTER) */
    } /* for loop */
     return(FALSE);
#endif 
      if (*pDD == NULL_POINTER)
      {
        return(FALSE);
      }
      
     return(TRUE); 
    
}

/******************************************************************************/

uchar key_descriptor_lookup_procedure
( 
 uchar *key_lookup_data, 
 uchar key_lookup_size, 
 key_descriptor_t **pKD /* KeyDescriptor to be returned */
 )
{
    uchar i = 0,j = 0;
    key_id_lookup_descriptor_t *pKLU = NULL;

    if (key_lookup_data == NULL_POINTER)
        return(FALSE);

    
    /* For each Entry in the Key table */
    for(i=0; i<mac_security_data.pib.mac_key_table_entries; i++)
    {
        *pKD = (key_descriptor_t*) queue_item_read_from( (queue_t *)&mac_security_data.pib.mac_key_table, i );
        if (*pKD != NULL_POINTER)
        {
            /* For each Entry in the Key Lookup List */
            for(j=0; j<(*pKD)->key_id_lookup_list_entries; j++)
            {
                pKLU = (key_id_lookup_descriptor_t*) queue_item_read_from( (queue_t *) &(*pKD)->key_id_lookup_list, j );
                if (pKLU != NULL_POINTER)
                {
                    if ((pKLU->lookup_data_size == key_lookup_size) && (!memcmp(pKLU->lookup_data, key_lookup_data,( key_lookup_size == 1 ) ? 9 : 5 )))
                    {
                        //debug(("Matched KEY Descriptor\r\n"));
                        return(TRUE);
                    }
                } /* if (*pKLU != NULL_POINTER) */
            } 
        } /* if (*pKD != NULL_POINTER) */
    }
    //debug(("KEY Descriptor Lookup failed\n\r"));
  
    return(FALSE);
}

/******************************************************************************/


uchar outgoing_key_retrieval_procedure
( 
 mac_address_t *src_address,
 mac_address_t *dst_address,
 uchar key_id_mode, 
 uchar *key_identifier, /* Key Source and Key Index */
 uchar *key             /* The Key to be Returned if Sucessful */
 )
{
    //uchar key_lookup_data[MAX_LOOKUP_LENGTH];
   // uchar key_lookup_size = 0;
//    key_descriptor_t *pKD =  NULL;

    /* Check we have Valid Addresses otherwise Procedure will fail */
    if (dst_address == NULL_POINTER)
        return(FALSE);

    if (src_address == NULL_POINTER)
        return(FALSE);
    
//    /* Implicit KEY Identification Mode */
//    if ( key_id_mode == 0 ) 
//    {   
//        if (dst_address->address_mode == ADDR_MODE_NONE)
//        {
//            if ( mac_security_data.pib.mac_PAN_coord_short_address < USE_IEEE_ADDRESS )
//            {
//                /* SRC PAN ID + MAC PAN Coord Short Addr + 1 Octet of 0 */
//                key_lookup_data[0] = (src_address->pan_id >> 8);
//                key_lookup_data[1] = (src_address->pan_id & 0xff);
//                key_lookup_data[2] = (mac_security_data.pib.mac_PAN_coord_short_address >> 8);
//                key_lookup_data[3] = (mac_security_data.pib.mac_PAN_coord_short_address & 0xff);
//                key_lookup_data[4] = 0;
//                key_lookup_size = 0;         // 5 Octets
//            }
//            else if ( mac_security_data.pib.mac_PAN_coord_short_address == USE_IEEE_ADDRESS)
//            {
//                /* MAC PAN Coord IEEE Addr + 1 Octet of 0 */
//                memcpy(key_lookup_data, mac_security_data.pib.mac_PAN_coord_extended_address,8);
//                key_lookup_data[8] = 0;
//                key_lookup_size = 1;        // 9 Octets    
//            }
//        }
//        else if (dst_address->address_mode == ADDR_MODE_SHORT)
//        {
//            /* Dst PAN ID + Dst Short Addr + 1 Octet of 0 */
//            key_lookup_data[0] = (dst_address->pan_id >> 8);
//            key_lookup_data[1] = (dst_address->pan_id & 0xff);
//            key_lookup_data[2] = (dst_address->address.short_address >> 8);
//            key_lookup_data[3] = (dst_address->address.short_address & 0xff);
//            key_lookup_data[4] = 0;
//            key_lookup_size = 0;        // 5 Octets
//        }
//        else if (dst_address->address_mode == ADDR_MODE_EXTENDED)
//        {
//            /* Dst IEEE Addr + 1 Octet of 0 */
//            memcpy(key_lookup_data, dst_address->address.ieee_address,8);
//            key_lookup_data[8] = 0;
//            key_lookup_size = 1;        // 9 Octets
//        }
//    }
//    /* EXPLICIT KEY Identification modes */
//    else if ( key_id_mode == 1) 
//    {   /* 8 Octet macDefaultKeySource + Key Index */
//        memcpy(&key_lookup_data[0], mac_security_data.pib.mac_default_key_source, 8); 
//        key_lookup_data[8] = key_identifier[0];
//        key_lookup_size = 1;            // 9 Octets
//    }
//    else if ( key_id_mode == 2)
//    {
//        /* 4 Octet Key Source + Key Index */
//        memcpy(&key_lookup_data[0],key_identifier,5);
//        key_lookup_size = 0;            // 5 Octets
//    }
//    else if ( key_id_mode == 3)
//    {
//        /* 8 Octet Key Source + Key Index */
//        memcpy(&key_lookup_data[0],key_identifier,9);
//        key_lookup_size = 1;            // 9 Octets
//    }
//    else
//    {
//        /* Invalid Key ID Mode */
//        return(FALSE);
//    }

    //debug(("ENC KIM= %x\r\n",key_id_mode));
    //debug_print_bytes(key_lookup_data, 9);
    //debug(("ENC KIM Size = %x\r\n",key_lookup_size));
 #ifdef WISUN_FAN_MAC 
    /* Attempt to find the Key */
//    if (key_descriptor_lookup_procedure( key_lookup_data, key_lookup_size, &pKD) == TRUE)
//    {
//        uchar i = 0; //,j = 0;
//        mac_key_descriptor_t *mkey = NULL;
//
//
//
//
//        /* For each Entry in the Key table */
//        for(i=0;i < pKD->mac_key_list.count; i++)
//        {
//            /* For each Entry in the Key Lookup List */
//              
//                    mkey = (mac_key_descriptor_t*) queue_item_read_from( (queue_t *) &(*pKD).mac_key_list, i );
//                    if (mkey != NULL_POINTER)
//                    {
//                        if (mac_key_list.active_key_index == mkey->index)
//                        {
//                            //debug(("Matched KEY Descriptor\r\n"));
//                            copy_key(&key[0],&mkey->mac_key[0]);
//                        }
//                        }                
//       } 
////        copy_key(&key[0],&pKD->key[0]);
//        return(TRUE);         
//    }
    copy_key(&key[0],mac_key_list.MAC_SECURITY_KEY_LIST[mac_key_list.active_key_index-1].MAC_SECURITY_KEY);
    return(TRUE);
#endif //#ifdef WISUN_FAN_MAC       
    //return(FALSE);
  
}
#if APP_LBR_ROUTER
uint8_t is_security_key_index_changed (uint8_t nbr_key_index)
{
  if (nbr_key_index == mac_key_list.active_key_index)
    return 0;
  else
  {
#if (PRINT_DEBUG_LEVEL == CONSOLE_ERROR_DEBUG)
    stack_print_debug ("Sec index changed :: self = %d, rcv = %d\n", mac_key_list.active_key_index, nbr_key_index);
#endif         
    return 1;
  }
}
#endif
/******************************************************************************/


uchar incoming_key_retrieval_procedure
( 
 mac_address_t *src_address,
 mac_address_t *dst_address,
 uchar key_id_mode, 
 uchar *key_identifier, 
 key_descriptor_t **pKD,
 key_device_descriptor_t **pKDD,
 device_descriptor_t **pDD
 )
{

  uchar key_lookup_data[MAX_LOOKUP_LENGTH] = {0};
    uchar key_lookup_size = 0;
    uchar device_lookup_data[MAX_LOOKUP_LENGTH] = {0};
    uchar device_lookup_size = 0;

    if ((src_address == NULL_POINTER) || (key_identifier == NULL_POINTER) || (dst_address == NULL_POINTER))
    {
        return(FALSE);
    }

    /* Implicit KEY Identification Mode */
    if ( key_id_mode == 0 ) 
    {   
        if (src_address->address_mode == ADDR_MODE_NONE)
        {
            if ( mac_security_data.pib.mac_PAN_coord_short_address < USE_IEEE_ADDRESS )
            {
                /* SRC PAN ID + MAC PAN Coord Short Addr + 1 Octet of 0 */
                key_lookup_data[0] = (dst_address->pan_id >> 8);
                key_lookup_data[1] = (dst_address->pan_id & 0xff);
                key_lookup_data[2] = (mac_security_data.pib.mac_PAN_coord_short_address >> 8);
                key_lookup_data[3] = (mac_security_data.pib.mac_PAN_coord_short_address & 0xff);
                key_lookup_data[4] = 0;
                key_lookup_size = 0;         // 5 Octets
            }
            else if ( mac_security_data.pib.mac_PAN_coord_short_address == USE_IEEE_ADDRESS )
            {
                /* MAC PAN Coord IEEE Addr + 1 Octet of 0 */
                memcpy(key_lookup_data, mac_security_data.pib.mac_PAN_coord_extended_address,8);
                    
                key_lookup_data[8] = 0;
                key_lookup_size = 1;        // 9 Octets 
            }
        }
        else if (src_address->address_mode == ADDR_MODE_SHORT)
        {           
            /* Dst PAN ID + Dst Short Addr + 1 Octet of 0 */
            key_lookup_data[0] = (src_address->pan_id >> 8);
            key_lookup_data[1] = (src_address->pan_id & 0xff);
            key_lookup_data[2] = (src_address->address.short_address >> 8);
            key_lookup_data[3] = (src_address->address.short_address & 0xff);
            key_lookup_data[4] = 0;
            key_lookup_size = 0;        // 5 Octets
        }
        else if (src_address->address_mode == ADDR_MODE_EXTENDED)
        {
            /* Dst IEEE Addr + 1 Octet of 0 */
            memcpy(key_lookup_data, src_address->address.ieee_address,8);
            key_lookup_data[8] = 0;
            key_lookup_size = 1;        // 9 Octets
        }
    }
    /* EXPLICIT KEY Identification modes */
    else if ( key_id_mode == 1 ) 
    {
        memcpy(&key_lookup_data[0],mac_security_data.pib.mac_default_key_source,8); 
        key_lookup_data[8] = key_identifier[0];
        key_lookup_size = 1;        // 9 Octets
    }
    else if ( key_id_mode == 2 )
    {
        memcpy(&key_lookup_data[0],key_identifier,5);
        key_lookup_size = 0;        // 5 Octets
    }
    else if ( key_id_mode == 3 )
    {
        memcpy(&key_lookup_data[0],key_identifier,9);
        key_lookup_size = 1;        // 9 Octets
    }
    else
    {
        /* Unable to determine KeyLookupData */
        return(FALSE);
    }

    //debug(("KIM = %x\r\n",key_id_mode));
    //debug_print_bytes(key_lookup_data, 9);
    //debug(("KLSize = %x\r\n",key_lookup_size));

    /* Obtain the KeyDescriptor */
    if (key_descriptor_lookup_procedure( key_lookup_data, key_lookup_size, pKD) == FALSE)
    {
        /* Unable to get KeyDescriptor */
        return(FALSE);
    }


    /* Determine Device Lookup Data and Lookup Size */
    if (src_address->address_mode == ADDR_MODE_NONE) 
    {
#ifdef WISUN_FAN_MAC    
#if(FAN_EDFE_FEATURE_ENABLED == 1)
      if(edfe_information.edfe_frame_enabled == 0x01)
      {
        mem_rev_cpy(device_lookup_data, edfe_information.edfe_ini_mac_addr, IEEE_ADDRESS_LENGTH);
        device_lookup_size = IEEE_ADDRESS_LENGTH;
      }
      else
#endif
      {
        if ( mac_security_data.pib.mac_PAN_coord_short_address < USE_IEEE_ADDRESS )
        {
            device_lookup_data[0] = (dst_address->pan_id >> 8);
            device_lookup_data[1] = (dst_address->pan_id & 0xff);
            device_lookup_data[2] = (mac_security_data.pib.mac_PAN_coord_short_address >> 8);
            device_lookup_data[3] = (mac_security_data.pib.mac_PAN_coord_short_address & 0xff);
            device_lookup_size = 4;
        }
        else if ( mac_security_data.pib.mac_PAN_coord_short_address == USE_IEEE_ADDRESS)
        {
            memcpy(device_lookup_data, &mac_security_data.pib.mac_PAN_coord_extended_address, IEEE_ADDRESS_LENGTH);
            device_lookup_size = IEEE_ADDRESS_LENGTH;
        }
      }
#endif      
    }
    else if (src_address->address_mode == ADDR_MODE_SHORT) 
    {
        device_lookup_data[0] = (src_address->pan_id >> 8);
        device_lookup_data[1] = (src_address->pan_id & 0xff);
        device_lookup_data[2] = (src_address->address.short_address >> 8);
        device_lookup_data[3] = (src_address->address.short_address & 0xff);
        device_lookup_size = 4;
    }
    else if (src_address->address_mode == ADDR_MODE_EXTENDED) 
    {
        memcpy(device_lookup_data, src_address->address.ieee_address, IEEE_ADDRESS_LENGTH);
        device_lookup_size = IEEE_ADDRESS_LENGTH;
    }
    else
    {
        /* Unable to determine DeviceLookupData */
        return(FALSE);
    }
    //debug_print_bytes(device_lookup_data, device_lookup_size);
    //debug(("DLS = %x\r\n",device_lookup_size));
    
//#if(CFG_MAC_BLACK_LIST_IS_USE == 1) 
//    /* Get KeyDeviceDexcriptor, DeviceDescriptor, and Blacklisted State */
//    if (blacklist_checking_procedure( *pKD, device_lookup_data, device_lookup_size, pKDD,pDD) == FALSE)
//    {   
//        return(FALSE);
//    }
//#else
    if (blacklist_checking_procedure( *pKD, device_lookup_data, device_lookup_size, pKDD,pDD) == FALSE)
    {   
        return(FALSE);
    }
//#endif
    
    /* We must have obtained all the data return True */
    return(TRUE);
}

/******************************************************************************/

uchar incoming_key_usage_check( key_descriptor_t **pKD, uchar frame_type, uchar command_frame_identifier )
{
#if(CFG_MAC_BLACK_LIST_IS_USE == 1) 
  
    key_usage_descriptor_t *pKUD = NULL;
    uchar i = 0;

    /* For each Entry in the Key usage List */
    for(i=0; i < (*pKD)->key_usage_list_entries; i++)
    {
        pKUD = (key_usage_descriptor_t*) queue_item_read_from( (queue_t *) &(*pKD)->key_usage_list, i );
        if (pKUD != NULL_POINTER)
        {
            if (( frame_type != MAC_FRAME_TYPE_MAC_COMMAND) && (frame_type == pKUD->frame_type))
            {
                return(TRUE);
            }
            else if (( frame_type == MAC_FRAME_TYPE_MAC_COMMAND) && (frame_type == pKUD->frame_type) && ( command_frame_identifier == pKUD->command_frame_identifier))
            {
                
            }       
        }       
    }
    return(FALSE);
#else
    return(TRUE);
#endif    
    
}

/******************************************************************************/

void init_nonce( uchar *nonce, ulong frame_counter, uchar security_level, uchar *IEEE_addr )
{
    if ( !(mac_security_data.security_flags & SF_NONCE_IEEE_ADDRESS))
    {
        nonce[0] = IEEE_addr[7];
        nonce[1] = IEEE_addr[6];
        nonce[2] = IEEE_addr[5];
        nonce[3] = IEEE_addr[4];
        nonce[4] = IEEE_addr[3];
        nonce[5] = IEEE_addr[2];
        nonce[6] = IEEE_addr[1];
        nonce[7] = IEEE_addr[0];
    }
    else
    {
        nonce[0] = IEEE_addr[0];
        nonce[1] = IEEE_addr[1];
        nonce[2] = IEEE_addr[2];
        nonce[3] = IEEE_addr[3];
        nonce[4] = IEEE_addr[4];
        nonce[5] = IEEE_addr[5];
        nonce[6] = IEEE_addr[6];
        nonce[7] = IEEE_addr[7];
    }

    if ( !(mac_security_data.security_flags & SF_NONCE_FRAME_COUNTER))
    {
        nonce[8] = frame_counter >> 24;
        nonce[9] = frame_counter >> 16;
        nonce[10] = frame_counter >> 8;
        nonce[11] = frame_counter;
    }
    else
    {
        nonce[8] = frame_counter;
        nonce[9] = frame_counter >> 8;
        nonce[10] = frame_counter >> 16;
        nonce[11] = frame_counter >> 24;
    }
    nonce[12] = security_level;
}

/******************************************************************************/

uchar integrity_code_length( uchar security_level )
{
    switch( security_level )
    {
    case MAC_SECURITY_AES_CCMSTAR_MIC_128:/*fall through*/
    case MAC_SECURITY_AES_CCMSTAR_ENC_MIC_128:
        return 16;

    case MAC_SECURITY_AES_CCMSTAR_MIC_64:/*fall through*/
    case MAC_SECURITY_AES_CCMSTAR_ENC_MIC_64:
        return 8;

    case MAC_SECURITY_AES_CCMSTAR_MIC_32:/*fall through*/
    case MAC_SECURITY_AES_CCMSTAR_ENC_MIC_32:
        return 4;

    default:
        return 0;
    }
}

/******************************************************************************/

void mac_get_security_data( mac_rx_t *rxmsg )
{
    uchar *p = NULL;
      
    /* bufp pointing at length */
    p = &rxmsg->pd_rxp->psdu[0];

    /* Step over Frame Control & Seq Num */
    if(rxmsg->pd_rxp->psdu[1] & MAC_SEQ_NUM_SUPPRESSION)
    {
      p+=2;
    }
    else
    {
      /* Step over Frame Control & Seq Num */
      p+=3;
    }
    /* Step over the Destination Address */
    switch( rxmsg->pd_rxp->psdu[1] & MAC_DST_ADDRESS_MASK )
    {
        case MAC_SHORT_DST_ADDRESS:
            p+=4;
            break;

        case MAC_IEEE_DST_ADDRESS:
           if (rxmsg->pd_rxp->psdu[0] & MAC_INTRA_PAN)
           {
                p+=8;        /* just IEEE address */
           }
           else
           {
                p+=10;
           }
            break;
    }

    /* Step over the Source Address */
    switch( rxmsg->pd_rxp->psdu[1] & MAC_SRC_ADDRESS_MASK )
    {
        case MAC_NO_SRC_ADDRESS:
            /* no change to offset */
            break;

        case MAC_SHORT_SRC_ADDRESS:
            if (rxmsg->pd_rxp->psdu[0] & MAC_INTRA_PAN)
            {
                p+=2;        /* just short address */
            }
            else
            {
                p+=4;        /* pan id and short address */
            }   
            break;

        case MAC_IEEE_SRC_ADDRESS:
            if (rxmsg->pd_rxp->psdu[0] & MAC_INTRA_PAN)
            {   
                p+=8;        /* just IEEE address */
            }
            else
            {
              #ifdef  WISUN_ENET_PROFILE
                  p+=8;       /*ieee address */
              #else              
                p+=10;       /* pan id + ieee address */
              #endif
            }
            break;
    }
   
    /* Get security Level and Key ID Mode */
    rxmsg->sec_param.security_level = *p & INCOMING_FRAME_SECURITY_LEVEL_MASK;
    rxmsg->sec_param.key_id_mode = (*p++ >> 3) & 0x3;

    /* Get the frame Counter */    
    rxmsg->frame_counter = (ulong)p[3] << 24 | (ulong)p[2] <<16 | (ulong)p[1] <<8 | (ulong)p[0];
    /* Step over the Frame Counter */
    p +=4;

    memcpy(&rxmsg->sec_param.key_identifier, p, KEY_IDENTIFIER_MAX_LENGTH);   
}

/******************************************************************************/

uchar get_bcn_payload_offset( uchar *mac_buffer )
{
    uchar *p = NULL;
    uchar gts_count = 0, short_pending = 0, long_pending = 0;


    p = mac_buffer;

    /* Step over the Super Frame Spec */
    p+=2;

    gts_count = (*p++ & 0x7);

    if (gts_count > 0)
    {
        /* Each GTS is 3 bytes + 1 for the Direction Byte */
        p += ((3*gts_count) + 1);
    }

    short_pending = *p & 0x7;
    long_pending = (*p++ >> 4) & 0x7;

    /* Step over any short addresses */
    p += (short_pending * 2);

    /* Step over any long addresses */
    p += (long_pending * 8);

    return( p - mac_buffer );
}

/******************************************************************************/

void copy_key( uchar *d, uchar *s)
{
    if ( !(mac_security_data.security_flags & SF_KEY))
    {
        d[0] = s[0];
        d[1] = s[1];
        d[2] = s[2];
        d[3] = s[3];
        d[4] = s[4];
        d[5] = s[5];
        d[6] = s[6];
        d[7] = s[7];
        d[8] = s[8];
        d[9] = s[9];
        d[10] = s[10];
        d[11] = s[11];
        d[12] = s[12];
        d[13] = s[13];
        d[14] = s[14];
        d[15] = s[15];
    }
    else
    {
        d[0] = s[15];
        d[1] = s[14];
        d[2] = s[13];
        d[3] = s[12];
        d[4] = s[11];
        d[5] = s[10];
        d[6] = s[9];
        d[7] = s[8];
        d[8] = s[7];
        d[9] = s[6];
        d[10] = s[5];
        d[11] = s[4];
        d[12] = s[3];
        d[13] = s[2];
        d[14] = s[1];
        d[15] = s[0];
    }
}

/******************************************************************************/

void copy_IEEE_address( uchar *d, uchar *s)
{
    if ( (mac_security_data.security_flags & SF_IEEE_ADDR))
    {
        d[0] = s[0];
        d[1] = s[1];
        d[2] = s[2];
        d[3] = s[3];
        d[4] = s[4];
        d[5] = s[5];
        d[6] = s[6];
        d[7] = s[7];
    }
    else
    {
        d[0] = s[7];
        d[1] = s[6];
        d[2] = s[5];
        d[3] = s[4];
        d[4] = s[3];
        d[5] = s[2];
        d[6] = s[1];
        d[7] = s[0];
    }
}

/******************************************************************************/

uchar write_key_table_entry_to_buffer( uchar *buf, key_descriptor_t* p_kd, uchar store )
{

#if(CFG_MAC_BLACK_LIST_IS_USE == 1) 
  
  
    uchar i = 0;
    key_id_lookup_descriptor_t *p_ld = NULL;
    key_device_descriptor_t *p_dd = NULL;
    key_usage_descriptor_t *p_ud = NULL;
    uchar *p = NULL;

    p = buf;

    if (store == TRUE)
    {
        /* Store the Index as this is going in NV */
        *p++ = p_kd->index;
    }

    /* Add the Lookup Table entries to the buffer */
	*p++ = p_kd->key_id_lookup_list_entries;

	/* Add each entry of the look up turn */
	for ( i=0; i < p_kd->key_id_lookup_list_entries; i++)
	{
		/* Get the entry from the list */
		p_ld = (key_id_lookup_descriptor_t * ) queue_item_read_from( (queue_t *) &p_kd->key_id_lookup_list, i );
    	if (p_ld != NULL_POINTER)
        {
            /* Store the data */
		    memcpy(p, &(p_ld->lookup_data[0]),MAX_LOOKUP_LENGTH);
		    p += MAX_LOOKUP_LENGTH;
		    *p++ = p_ld->lookup_data_size;
        }
	}

	/* Now add the Key Device List to the buffer */
	*p++ = p_kd->key_device_list_entries;

	/* Add each entry of the look up turn */
	for (i=0; i<p_kd->key_device_list_entries; i++)
	{
		/* Get the entry from the lisy */
		p_dd = (key_device_descriptor_t * ) queue_item_read_from( (queue_t *) &p_kd->key_device_list, i );
       	if (p_dd != NULL_POINTER)
        {
            /* copy the over the data */
	    	*p++ = p_dd->device_table_entry_handle;
		    *p++ = p_dd->unique_device;
		    *p++ = p_dd->black_listed;
        }
	}

	/* Now add the Key usage List to the buffer */
	*p++ = p_kd->key_usage_list_entries;

	/* Add each entry of the look up turn */
	for (i=0; i<p_kd->key_usage_list_entries; i++)
	{
		/* Get the entry from the lisy */
		p_ud = (key_usage_descriptor_t * ) queue_item_read_from( (queue_t *) &p_kd->key_usage_list, i );
       	if (p_ud != NULL_POINTER)
        {
            /* copy the over the data */
	    	*p++ = p_ud->frame_type;
		    *p++ = p_ud->command_frame_identifier;
        }
	}

	/* Finally copy the key in to the buffer */
	memcpy(p, &p_kd->key, KEY_LENGTH);
    p += KEY_LENGTH;

    return( p - buf );
#else
    return 0;
    
    #endif
    
}

/******************************************************************************/

uchar write_device_table_entry_to_buffer( uchar *buf, device_descriptor_t *pdd, uchar store )
{
    uchar *p = NULL;

    p = buf;

//    if (store == TRUE)
//    {
//        /* Store the Index as this is going in NV */
//        *p++ = pdd->index;
//    }

    /* Store the Data LSB first */
//    *p++ = pdd->pan_id & 0xff;
//    *p++ = (pdd->pan_id >> 8) & 0xff;
//    *p++ = pdd->short_addr & 0xff;
//    *p++ = (pdd->short_addr >> 8) & 0xff;
    memcpy(p,&pdd->ieee_addr,IEEE_ADDRESS_LENGTH);
    p += IEEE_ADDRESS_LENGTH;
    if(mac_security_data.pib.mac_sec_frame_counter_perkey == 0)
    {
      *p++ = pdd->frame_count[0] & 0xff;
      *p++ = (pdd->frame_count[0] >> 8) & 0xff;
      *p++ = (pdd->frame_count[0] >> 16) & 0xff;
      *p++ = (pdd->frame_count[0] >> 24) & 0xff;
//      *p++ = pdd->exempt;
    }
    return(p - buf);
}

/******************************************************************************/

uchar write_security_level_table_entry_to_buffer( uchar *buf, security_level_descriptor_t *p_sld, uchar store )
{
    uchar *p = NULL;

    p = buf;

    if (store == TRUE)
    {
        /* Store the Index as this is going in NV */
        *p++ = p_sld->index;
    }

    *p++ = p_sld->frame_type;
    *p++ = p_sld->command_frame_identifier;
    *p++ = p_sld->security_minimum;
    *p++ = p_sld->device_overide_security_minimum;

    return(p - buf);
}

/******************************************************************************/

uchar store_key_table_entry_in_pib( uchar *data, uchar table_entry )
{
#if 0
    key_descriptor_t *p_kd = NULL;
    key_device_descriptor_t *p_dl = NULL;
    key_id_lookup_descriptor_t *p_lu = NULL;
    key_usage_descriptor_t *p_ud = NULL;
    uchar i = 0;
    uchar *p = NULL;

    p = data;

    /* allocate some memory for it */
    p_kd = (key_descriptor_t *) app_bm_alloc(sizeof(key_descriptor_t));
    
    if ( p_kd != NULL_POINTER)
    {
        initialise_key_descriptor(p_kd);

        /* Are we restoring this in NV */
        /*if (restore == TRUE) // Sagar: not used
        {
            If so restore the table index from NV 
            p_kd->index = *p++;    
        }
        else*/
        {
            p_kd->index = table_entry;    
        }

                /* Delete the entry if it already exists */
                delete_key_table_entry( p_kd->index );

		/* Get the number of entries in the lookup Table */
		p_kd->key_id_lookup_list_entries = *p++;

                /* Add each entry to the look up table */
		for (i = 0; i < p_kd->key_id_lookup_list_entries; i++)
		{
			p_lu = (key_id_lookup_descriptor_t *) app_bm_alloc(sizeof(key_id_lookup_descriptor_t));
			if (p_lu != NULL_POINTER)
			{
                                p_lu->next = NULL_POINTER;			    
				memcpy(&p_lu->lookup_data[0], p, MAX_LOOKUP_LENGTH);
				p += MAX_LOOKUP_LENGTH;
				p_lu->lookup_data_size = *p++;
                                queue_item_put( (queue_t *) &p_kd->key_id_lookup_list, (queue_item_t*) p_lu );
			}
			else
			{
				// Return error that says we have ran out of memory
			    //debug(("MM Failed - Key Lookup \n\r"));
			    /* Free up all other memory that was Allocated by this function */
			    queue_delete((queue_t*)&p_kd->key_id_lookup_list);
			    app_bm_free((uint8_t*)p_kd);
                            return 0;
			}
		}

		/* Get the number of entries in the device list */
		p_kd->key_device_list_entries = *p++ ;

                /* Add each entry to the device List */
		for (i = 0; i < p_kd->key_device_list_entries; i++)
		{
                        p_dl = (key_device_descriptor_t *) app_bm_alloc(sizeof(key_device_descriptor_t));
			if (p_dl != NULL_POINTER)
			{
                                p_dl->next = NULL_POINTER;
				p_dl->device_table_entry_handle = *p++;
				p_dl->unique_device = *p++;
				p_dl->black_listed = *p++;

				queue_item_put( (queue_t *) &p_kd->key_device_list, (queue_item_t*) p_dl );
			}
			else
			{
			    // Return error that says we have ran out of memory
			    //debug(("MM Failed - Key device List \n\r"));
			    /* Free up all other memory that was Allocated by this function */
                            queue_delete((queue_t*)&p_kd->key_id_lookup_list);
                            queue_delete((queue_t*)&p_kd->key_device_list);
                            app_bm_free((uint8_t*)p_kd);
                            return 0;
			}
		}

		/* Get the number of entries in the usage list */
		p_kd->key_usage_list_entries = *p++ ;

		/* Add each entry to the device List */
		for (i = 0; i < p_kd->key_usage_list_entries; i++)
		{
			p_ud = (key_usage_descriptor_t *) app_bm_alloc(sizeof(key_usage_descriptor_t));
			if (p_ud != NULL_POINTER)
			{
                            p_ud->next = NULL_POINTER;
                            p_ud->frame_type = *p++;
                            p_ud->command_frame_identifier = *p++;
                            queue_item_put( (queue_t *) &p_kd->key_usage_list, (queue_item_t*) p_ud );
			}
			else
			{
				// Return error that says we have ran out of memory
			    //debug(("MM Failed - Key Usage List \n\r"));
			    /* Free up all other memory that was Allocated by this function */
                            queue_delete((queue_t*)&p_kd->key_id_lookup_list);
                            queue_delete((queue_t*)&p_kd->key_device_list);
                            queue_delete((queue_t*)&p_kd->key_usage_list);
                            app_bm_free((uint8_t*)p_kd);
                            return 0;
			}
		}

		/* Copy over the Key */
              memcpy(&p_kd->key[0],p,KEY_LENGTH);         
              p += KEY_LENGTH;
              queue_item_put((queue_t *) &mac_security_data.pib.mac_key_table, (queue_item_t*) p_kd );
              //if (restore != TRUE)
              //{
                  /* If we are not Restoring then inc count*/
                  /* Otherwise Count was read from NV */
                  mac_security_data.pib.mac_key_table_entries++;
              //}
#endif                  
                  key_descriptor_t *mac_keyd = NULL;
                  key_id_lookup_descriptor_t *p_lu = NULL;
                  mac_key_descriptor_t  *mk_d = NULL;
                  uchar i = 0;
                  uchar *p = NULL;
                  p = data;
                   /* allocate some memory for it */
                  mac_keyd = (key_descriptor_t *) app_bm_alloc(sizeof(key_descriptor_t));
                  
                  if ( mac_keyd != NULL_POINTER)
                  {
                    initialise_key_descriptor(mac_keyd);
                    
                     mac_keyd->index = table_entry;
                     /* Delete the entry if it already exists */
                     delete_key_table_entry( mac_keyd->index );
                     
                     mac_keyd->key_id_lookup_list_entries = *p++;
                   for (i = 0; i < mac_keyd->key_id_lookup_list_entries; i++)
                    {
                            p_lu = (key_id_lookup_descriptor_t *) app_bm_alloc(sizeof(key_id_lookup_descriptor_t));
                            if (p_lu != NULL_POINTER)
                            {
                                    p_lu->next = NULL_POINTER;			    
                                    memcpy(&p_lu->lookup_data[0], p, MAX_LOOKUP_LENGTH);
                                    p += MAX_LOOKUP_LENGTH;
                                    p_lu->lookup_data_size = 0x01;//*p++;
                                    queue_item_put( (queue_t *) &mac_keyd->key_id_lookup_list, (queue_item_t*) p_lu );
                            }
                            else
                            {
                                    // Return error that says we have ran out of memory
                                //debug(("MM Failed - Key Lookup \n\r"));
                                /* Free up all other memory that was Allocated by this function */
                                queue_delete((queue_t*)&mac_keyd->key_id_lookup_list);
                                app_bm_free((uint8_t*)mac_keyd);
                                return 0;
                            }
                    }
                   mac_keyd->mac_key_list_entries = *p++ ;
                   
                    for (i = 0; i < mac_keyd->mac_key_list_entries; i++)
                    {
                          mk_d = (mac_key_descriptor_t *) app_bm_alloc(sizeof(mac_key_descriptor_t));
                          if(mk_d != NULL)
                          {
                            mk_d->next = NULL;
                            mk_d->index = *p++;
                            memcpy(mk_d->mac_key,p,16);
                            p += 16;
                            queue_item_put( (queue_t *) &mac_keyd->mac_key_list, (queue_item_t*) mk_d );
                          }
                          else
                          {
                              queue_delete((queue_t*)&mac_keyd->key_id_lookup_list);
                              queue_delete((queue_t*)&mac_keyd->mac_key_list);
                              app_bm_free((uint8_t*)mac_keyd);
                              return 0;
                          }      
                    }
                      queue_item_put((queue_t *) &mac_security_data.pib.mac_key_table, (queue_item_t*) mac_keyd );
                      p += 68;
                      mac_security_data.pib.mac_key_table_entries++;
                      return( p - data );
                  }             
                  
                  return 0;             
}

/******************************************************************************/

uchar delete_key_table_entry( uchar table_entry )
{
	key_descriptor_t *p = NULL;
        uchar i = 0;
#if 1
    /* Check if this index already exists */
    while ( (p = (key_descriptor_t*) queue_item_read_from( (queue_t *)&mac_security_data.pib.mac_key_table, i )) != NULL_POINTER )
    {
        if (p->index == table_entry)
        {
            /* Index exists, So delete it*/
//            p->key_id_lookup_list_entries = 0;
//            p->mac_key_list_entries = 0;
            //p->key_usage_list_entries = 0;

            /* Delete and Free up memory on each of the lists contained in this entry */
            if ( p->key_id_lookup_list_entries != NULL_POINTER )
            {
                queue_delete((queue_t*) &p->key_id_lookup_list );
            }
            if ( p->mac_key_list_entries != NULL_POINTER )
            {
                queue_delete((queue_t*) &p->mac_key_list );
            }
//            if ( p->key_usage_list != NULL_POINTER )
//            {
//                queue_delete((queue_t*) &p->key_usage_list );
//            }

           /* Index already exists - So delete it*/
           queue_item_delete( (queue_t *) &mac_security_data.pib.mac_key_table, i);
           mac_security_data.pib.mac_key_table_entries--;
           return TRUE;
        }
        i++;
    }
#else
        /* Check if this index already exists */
    while ( (p = (key_descriptor_t*) queue_item_read_from( (queue_t *)&mac_security_data.pib.mac_key_table, i )) != NULL_POINTER )
    {
        if (p->index == table_entry)
        {
            /* Index exists, So delete it*/
            p->key_device_list_entries = 0;
            p->key_id_lookup_list_entries = 0;
            p->key_usage_list_entries = 0;
            /* Delete and Free up memory on each of the lists contained in this entry */
            if ( p->key_id_lookup_list != NULL_POINTER )
            {
                queue_delete((queue_t*) &p->key_id_lookup_list );
            }
           /* Index already exists - So delete it*/
           queue_item_delete( (queue_t *) &mac_security_data.pib.mac_key_table, i);
           mac_security_data.pib.mac_key_table_entries--;
           return TRUE;
        }
        i++;
    }
#endif    
    return FALSE;
}

/******************************************************************************/

key_descriptor_t* find_key_table_entry( uchar table_entry )
{
	key_descriptor_t *p = NULL;
      uchar i = 0;

    /* Check if this index already exists */
    while ( (p = (key_descriptor_t*) queue_item_read_from( (queue_t *)&mac_security_data.pib.mac_key_table, i )) != NULL_POINTER )
    {
        if (p->index == table_entry)
        {
           return p;
        }
        i++;
    }
    return NULL_POINTER;
}

/******************************************************************************/
#ifdef WISUN_FAN_MAC
//Suneet 14/09/2018 :: update mac_key in mac_pib according to index
void update_key_table_entry( uchar table_entry)
{
  key_descriptor_t *p = NULL;
  uchar i = 0;
  mac_key_descriptor_t *mkey = NULL;
  /* Check if this index already exists */
  while ( (p = (key_descriptor_t*) queue_item_read_from( (queue_t *)&mac_security_data.pib.mac_key_table, i )) != NULL_POINTER )
  {
      for(i=0;i < p->mac_key_list.count; i++)
      {
        /* For each Entry in the Key Lookup List */
        
        mkey = (mac_key_descriptor_t*) queue_item_read_from( (queue_t *) &(*p).mac_key_list, i );
        if (mkey != NULL_POINTER)
        {
          if (mac_key_list.active_key_index == mkey->index)
          {
            copy_key(&mkey->mac_key[0],&mac_key_list.MAC_SECURITY_KEY_LIST[table_entry].MAC_SECURITY_KEY[0]);
            return;
          }
        }                
      } 
  }
}
#endif
/******************************************************************************/

uchar store_device_table_entry_in_pib( uchar *data, uchar table_entry )
{
  device_descriptor_t *p = NULL;
  uchar *pd = NULL;
  
  pd = data;
  
  /* allocate some memory for it */
  p = (device_descriptor_t *) app_bm_alloc(sizeof(device_descriptor_t));
  
  if ( p != NULL_POINTER)
  {
    p->next = NULL_POINTER;
    
    memcpy(&p->ieee_addr[0],pd,IEEE_ADDRESS_LENGTH);
    pd+=8;
    
    if ( mac_security_data.pib.mac_sec_frame_counter_perkey == 0 ) // False Raka 
    {
      p->frame_count[0] = ((((ulong)pd[3]) << 24) | (((ulong)pd[2]) << 16)
                           | (((ulong)pd[1]) << 8) | (ulong)pd[0]) ;
    }
    else
    {
      p->frame_count[0] = ((((ulong)pd[3]) << 24) | (((ulong)pd[2]) << 16)
                           | (((ulong)pd[1]) << 8) | (ulong)pd[0]) ;
      p->frame_count[1] = ((((ulong)pd[3]) << 24) | (((ulong)pd[2]) << 16)
                           | (((ulong)pd[1]) << 8) | (ulong)pd[0]);
      p->frame_count[2] = ((((ulong)pd[3]) << 24) | (((ulong)pd[2]) << 16)
                           | (((ulong)pd[1]) << 8) | (ulong)pd[0]);
      p->frame_count[3] = ((((ulong)pd[3]) << 24) | (((ulong)pd[2]) << 16)
                           | (((ulong)pd[1]) << 8) | (ulong)pd[0]);
    }
    pd +=4;                             
    queue_item_put( (queue_t *) &mac_security_data.pib.mac_device_table, (queue_item_t*) p );
    mac_security_data.pib.mac_device_table_entries++;
#if (PRINT_DEBUG_LEVEL == CONSOLE_ERROR_DEBUG)
    stack_print_debug ("Store dev desc with frame count [%d] for ", p->frame_count[0]);
    print_mac_address (p->ieee_addr);
#endif
    return (pd - data);
  }
  else
  {
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
    stack_print_debug ("### device_descriptor_t malloc fail\n");
#endif
    // Return error that says we have ran out of memory
    //debug(("MM Failed - Device Table \n\r"));    
  }
  return(0);
}

/******************************************************************************/
/* Debdeep :: 17-jan-2019 :: modified this function */
uchar delete_device_table_entry( uchar table_entry )
{
  if (mac_security_data.pib.mac_device_table_entries > table_entry)
  {
    queue_item_delete ((queue_t *)&mac_security_data.pib.mac_device_table, table_entry);
    mac_security_data.pib.mac_device_table_entries--;
    return TRUE;
  }
  return FALSE;
  
//	device_descriptor_t *p = NULL;
//        uchar i = 0;
//
//    /* Check if this index already exists */
//    while ( (p = (device_descriptor_t*) queue_item_read_from( (queue_t *)&mac_security_data.pib.mac_device_table, i )) != NULL_POINTER )
//    {
//        if (p->index == table_entry)
//        {
//           /* Index already exists - So delete it*/
//           queue_item_delete( (queue_t *) &mac_security_data.pib.mac_device_table, i);
//           mac_security_data.pib.mac_device_table_entries--;
//           return TRUE;
//        }
//        i++;
//    }
//    return FALSE;
}

void delete_all_device_from_mac_pib (void)
{
  uint8_t ii = 0;
  
  while ((ii = queue_count_get ((queue_t *)&mac_security_data.pib.mac_device_table)) > 0)
    queue_item_delete (&mac_security_data.pib.mac_device_table, ii-1);
  
  mac_security_data.pib.mac_device_table_entries = 0;
}

/******************************************************************************/
/* Debdeep :: 17-jan-2019 :: modified this function */
device_descriptor_t* find_device_table_entry( uchar table_entry )
{
  device_descriptor_t *p = NULL;
//  uchar i = 0;
  
  if (mac_security_data.pib.mac_device_table_entries > table_entry)
    p = (device_descriptor_t*) queue_item_read_from ((queue_t *)&mac_security_data.pib.mac_device_table, table_entry);
  
  return p;
  
//  /* Check if this index already exists */
//  while ( (p = (device_descriptor_t*) queue_item_read_from( (queue_t *)&mac_security_data.pib.mac_device_table, i )) != NULL_POINTER )
//  {
//    if (p->index == table_entry)
//    {
//      return p;
//    }
//    i++;
//  }
//  return NULL_POINTER;
}

/******************************************************************************/

uchar store_security_level_table_entry_in_pib( uchar *data, uchar table_entry )
{
	security_level_descriptor_t *p = NULL;
	uchar *pd = NULL;
	
	pd = data;

    /* allocate some memory for it */
    p = (security_level_descriptor_t *) app_bm_alloc(sizeof(security_level_descriptor_t));

    if ( p != NULL_POINTER)
    {
        p->next = NULL_POINTER;

        /* Are we restoring this in NV */
        /*if (restore == TRUE) //Sagar: Not used
        {
             If so restore the table index from NV
            p->index = *pd++;    
        }
        else*/
        {
            p->index = table_entry;    
        }

        /* Delete the Entry if it already exists */
        delete_security_level_table_entry( p->index );

        p->frame_type = *pd++;
        p->command_frame_identifier = *pd++;
        p->security_minimum = *pd++;
        p->device_overide_security_minimum = *pd++;

        queue_item_put( (queue_t *) &mac_security_data.pib.mac_security_level_table, (queue_item_t*) p );
//        if (restore != TRUE)
       // {
            /* If we are not Restoring then inc count*/
            /* Otherwise Count was read from NV */
            mac_security_data.pib.mac_security_level_table_entries++;
       // }
        return (pd - data);
    }
    else
    {
		// Return error that says we have ran out of memory
	    //debug(("MM Failed - Sec Level Table \n\r"));    
    }
    return (0);
}

/******************************************************************************/

uchar delete_security_level_table_entry( uchar table_entry )
{
	security_level_descriptor_t *p = NULL;
        uchar i = 0;

    /* Check if this index already exists */
    while ( (p = (security_level_descriptor_t*) queue_item_read_from( (queue_t *)&mac_security_data.pib.mac_security_level_table, i )) != NULL_POINTER )
    {
        if (p->index == table_entry)
        {
           /* Index already exists - So delete it*/
           queue_item_delete( (queue_t *) &mac_security_data.pib.mac_security_level_table, i);
           mac_security_data.pib.mac_security_level_table_entries--;
           return TRUE;
        }
        i++;
    }
    return FALSE;
}

/******************************************************************************/

security_level_descriptor_t* find_security_level_table_entry( uchar table_entry )
{
	security_level_descriptor_t *p = NULL;
        uchar i = 0;

    /* Check if this index already exists */
    while ( (p = (security_level_descriptor_t*) queue_item_read_from( (queue_t *)&mac_security_data.pib.mac_security_level_table, i )) != NULL_POINTER )
    {
        if (p->index == table_entry)
        {
           return p;
        }
        i++;
    }
    return NULL_POINTER;
}

/******************************************************************************/

//uchar save_security_data( uchar *dst )
//{ 
//    key_descriptor_t *p_kd = NULL;
//    device_descriptor_t *p_dd = NULL;
//    security_level_descriptor_t *p_sld = NULL;
//    uchar i = 0;
//    uchar *p = NULL;
//    
//    p = dst;
//
//    /* store number of entries in Key Table */
//    *p++ = mac_security_data.pib.mac_key_table_entries;
//
//    /* KEY TABLE */
//    for (i=0; i<mac_security_data.pib.mac_key_table_entries; i++)
//    {
//        /* Get first entry of Key Table */
//        p_kd = (key_descriptor_t * ) queue_item_read_from((queue_t *)&mac_security_data.pib.mac_key_table, i);
//        if (p_kd != NULL_POINTER)
//        {
//            p += write_key_table_entry_to_buffer( p, p_kd, TRUE );
//        }
//    }
//
//    /* store number of entries in Device Table */
//    *p++ = mac_security_data.pib.mac_device_table_entries;
//
//    /* DEVICE TABLE */ 
//    for (i=0; i<mac_security_data.pib.mac_device_table_entries; i++)
//    {
//        p_dd = (device_descriptor_t * ) queue_item_read_from( (queue_t *)&mac_security_data.pib.mac_device_table, i);
//    	if (p_dd != NULL_POINTER)
//        {
//            p += write_device_table_entry_to_buffer( p, p_dd, TRUE );
//        } 
//    }
//
//    /* store number of entries in Security Level Table */
//    *p++ = mac_security_data.pib.mac_security_level_table_entries;
//
//    /* SECURITY LEVEL TABLE */
//    for (i=0; i<mac_security_data.pib.mac_security_level_table_entries; i++)
//    {
//    	p_sld = (security_level_descriptor_t * ) queue_item_read_from( (queue_t *) &mac_security_data.pib.mac_security_level_table, i );
//    	if (p_sld != NULL_POINTER)
//        {
//            p += write_security_level_table_entry_to_buffer( p, p_sld, TRUE );
//        }
//    }
//
//    /* Copy over the remaining Security PIB Data */
//    *p++ = (mac_security_data.pib.mac_frame_counter & 0xff);
//    *p++ = ((mac_security_data.pib.mac_frame_counter >> 8) & 0xFF);
//    *p++ = ((mac_security_data.pib.mac_frame_counter >> 16) & 0xFF);
//    *p++ = ((mac_security_data.pib.mac_frame_counter >> 24) & 0xFF);          
//
//    *p++ = mac_security_data.pib.auto_request_security_level;
//
//    *p++ = mac_security_data.pib.auto_request_keyid_mode;
//
//    memcpy(p, mac_security_data.pib.auto_request_key_source, KEY_SOURCE_LENGTH);
//    p += KEY_SOURCE_LENGTH;
//
//    *p++ = mac_security_data.pib.auto_request_key_index;
//
//    memcpy(p, mac_security_data.pib.mac_default_key_source, KEY_SOURCE_LENGTH);
//    p += KEY_SOURCE_LENGTH;
//
//    memcpy(p, mac_security_data.pib.mac_PAN_coord_extended_address, IEEE_ADDRESS_LENGTH);
//    p += IEEE_ADDRESS_LENGTH;
//
//    *p++ = (mac_security_data.pib.mac_PAN_coord_short_address & 0xff);
//    *p++ = ((mac_security_data.pib.mac_PAN_coord_short_address >> 8) & 0xff);
//
//    /* Copy over the Beacon Security Data */
//    *p++ = mac_security_data.beacon_sec_param.security_level;
//    *p++ = mac_security_data.beacon_sec_param.key_id_mode;
//    memcpy(p,mac_security_data.beacon_sec_param.key_identifier,KEY_IDENTIFIER_MAX_LENGTH);
//    p += KEY_IDENTIFIER_MAX_LENGTH;
//
//    /* Copy over the Security Flag Settings */
//    *p++ = mac_security_data.security_flags;
//
//    return( p - dst );
//}

/******************************************************************************/

uchar restore_security_data( uchar *src )
{ 
    uchar i = 0, offset = 0;
    uchar *p = NULL;

    p = src;

    /* restore number of entries in Key Table */
    mac_security_data.pib.mac_key_table_entries = *p++;

    /* KEY TABLE */
    for (i=0; i<mac_security_data.pib.mac_key_table_entries; i++)
    {
        offset = store_key_table_entry_in_pib( p, i);
        if (offset != 0)
        {
            p += offset;
        }
        else
        {
            /* Restore Failed - Abort */
            mac_security_data.pib.mac_key_table_entries = 0;
            return(0);
        }
    }

    /* restore number of entries in Device Table */
    mac_security_data.pib.mac_device_table_entries = *p++;

    /* DEVICE TABLE */ 
    for (i=0; i<mac_security_data.pib.mac_device_table_entries; i++)
    {
        offset = store_device_table_entry_in_pib( p, i );
        if (offset != 0)
        {
            p += offset;
        }
        else
        {
            /* Restore Failed - Abort */
            mac_security_data.pib.mac_device_table_entries = 0;
            return(0);
        }
    }

    /* restore number of entries in Security Level Table */
    mac_security_data.pib.mac_security_level_table_entries = *p++;

    /* SECURITY LEVEL TABLE */
    for (i=0; i<mac_security_data.pib.mac_security_level_table_entries; i++)
    {
        offset = store_security_level_table_entry_in_pib( p, i );
        if (offset != 0)
        {
            p += offset;
        }
        else
        {
            /* Restore Failed - Abort */
            mac_security_data.pib.mac_security_level_table_entries = 0;
            return(0);
        }
    }

    /* Copy over the remaining Security PIB Data */
    mac_security_data.pib.mac_frame_counter = ((((ulong)p[3]) << 24) | (((ulong)p[2]) << 16)
                                                | (((ulong)p[1]) << 8) | (ulong)p[0]) ;
    p += 4;

    mac_security_data.pib.auto_request_security_level = *p++;
    mac_security_data.pib.auto_request_keyid_mode = *p++;
    memcpy(mac_security_data.pib.auto_request_key_source, p, KEY_SOURCE_LENGTH);
    p += KEY_SOURCE_LENGTH;
    mac_security_data.pib.auto_request_key_index = *p++;
    memcpy(mac_security_data.pib.mac_default_key_source, p, KEY_SOURCE_LENGTH);
    p += KEY_SOURCE_LENGTH;
    memcpy(mac_security_data.pib.mac_PAN_coord_extended_address, p, IEEE_ADDRESS_LENGTH);
    p += IEEE_ADDRESS_LENGTH;
    //mac_security_data.pib.mac_PAN_coord_short_address = (p[1] << 0x08) | p[0]; 
	mac_security_data.pib.mac_PAN_coord_short_address = p[0] | (((uint16_t)(p[1])) << 8);
    p += 2;

    /* Copy over the Beacon Security Data */
    mac_security_data.beacon_sec_param.security_level = *p++;
    mac_security_data.beacon_sec_param.key_id_mode = *p++;
    memcpy(mac_security_data.beacon_sec_param.key_identifier,p,KEY_IDENTIFIER_MAX_LENGTH);
    p += KEY_IDENTIFIER_MAX_LENGTH;

    /* Copy over the Security Flag Settings */
    mac_security_data.security_flags = *p++;

    return( p - src );
}

/******************************************************************************/

void initialise_key_descriptor( key_descriptor_t *p )
{
#if 0
    p->next = NULL_POINTER;
    queue_initialise( (queue_t*) &p->key_device_list );
    queue_initialise( (queue_t*) &p->key_id_lookup_list );
    queue_initialise( (queue_t*) &p->key_usage_list );
    p->key_device_list_entries = p->key_id_lookup_list_entries = p->key_usage_list_entries = 0;
#endif   
    p->next = NULL_POINTER;
    queue_initialise( (queue_t*) &p->mac_key_list );
    queue_initialise( (queue_t*) &p->key_id_lookup_list );
    p->mac_key_list_entries = p->key_id_lookup_list_entries = 0;
}

/******************************************************************************/

void check_authentication_status( mac_rx_t *rxmsg )
{
    /* Check the results of the Authentication Procedure */
    if ((mac_pib.mac_security_enabled == TRUE)
        && (rxmsg->security_data != NULL_POINTER))
    {
        device_descriptor_t *temp;
       // device_descriptor_t **pDD;
         uchar i = mac_security_data.pib.mac_device_table_entries;
        if (rxmsg->security_data->state == MAC_RX_STATE_FAILED_AUTHENTICATION)
        {
          /*Suneet :: 23/03/2018 Delete device Desc from table if Secure authantication is failed */
          for(uchar j = 0;j<=i;j++)
          {
                 temp =  (device_descriptor_t*) queue_item_read_from( (queue_t *)&mac_security_data.pib.mac_device_table, 
                                                                   j);
                 if (temp != NULL_POINTER)
                 {
                   if(!memcmp(rxmsg->src.address.ieee_address,
                               temp->ieee_addr,
                               8))
                    {
                      /* Debdeep :: 18-jan-2019 :: need not to delete device descriptor if security fails */
//                      delete_device_table_entry(j);
                      break;
                    }
                  }
          }      
            rxmsg->security_status = MAC_SECURITY_ERROR;
            
        }
        else
        {
            rxmsg->security_status = MAC_SUCCESS;
        }             
        /* Release the Memory Associated with Security */
        /* As All required data is now in rxmsg */
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//        stack_print_debug ("3F %p\n", rxmsg->security_data);
//#endif         
        app_bm_free((uint8_t*)rxmsg->security_data);
        rxmsg->security_data = NULL_POINTER;
    }
}

/******************************************************************************/

uint8_t mac_incoming_security_processor( void )
{
  security_struct_t* p_security_data = (security_struct_t *) queue_item_get(&(mac_data.security->hallin_rx_queue));
  if (p_security_data == NULL_POINTER)
  {
    event_clear( UNSECURE_EVENT );
    /* nothing on the decrypt queue to process so exit */
    return 0;
  }
  else
  {
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("frame fetched from queue for decryption: ");
//    print_mac_address (((mac_rx_t*)p_security_data->private_msg_data)->src.address.ieee_address);
//#endif      
    count++;
    if(p_security_data->payload_length > 0x17)
    {
      count = 15;
    }
    Apply_CCM_Star_Decrypt( p_security_data );
    
    Enqueue_Secure_Item( p_security_data );
    return 1;
  }
}

/******************************************************************************/

uint8_t mac_outgoing_security_processor( void )
{
	/*process outgoing packets*/
	security_struct_t* p_security_data = (security_struct_t *) queue_item_get(&(mac_data.security->hallin_tx_queue));
	
	if (p_security_data == NULL_POINTER)
    {
    	event_clear( SECURE_EVENT );
    	/* nothing on the tx encrypt queue to process so exit */
        return 0;
    }
    else
    {
    	Apply_CCM_Star_Encrypt( p_security_data );
    	
    	Enqueue_Secure_Item( p_security_data );
    	
		return 1;
    }
}

/* Debdeep :: API for direct security encryption for ACK packet */
void encrypt_ack_frame (security_struct_t* p_security_data)
{
  if (p_security_data == NULL_POINTER)
    return;
  
  Apply_CCM_Star_Encrypt( p_security_data );
}

/* Debdeep :: API for direct security encryption for secure data packet */
void encrypt_data_packet (security_struct_t* p_security_data)
{
  if (p_security_data == NULL_POINTER)
    return;
  
  Apply_CCM_Star_Encrypt( p_security_data );
}

/******************************************************************************/

/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/

static uint8_t Apply_CCM_Star_Encrypt( security_struct_t* p_security_data )
{
        uint8_t mic[16] = {0};
        p_security_data->raw_payload_data = NULL;
	//uint8_t aux_hdr_len;
	
#ifdef WISUN_ENET_PROFILE
        uint8_t wrong_mic[4] = {0x16, 0x23, 0x48, 0xFF};
#endif
	
#ifdef UTEST_SEC
        utu_timestamp( CCM_STAR_ENCRYPT, ((p_security_data->security_level) | 0x80) );
#endif
        p_security_data->raw_payload_data = (uchar*)app_bm_alloc(p_security_data->payload_length );
        if((p_security_data->raw_payload_data != NULL) && (p_security_data->payload_length != 0)
           &&p_security_data->payload_length != 0xFFFF)
          memcpy(p_security_data->raw_payload_data,p_security_data->payload,p_security_data->payload_length); //Suneet :: copy original payload to use in retranmit packet with update ufsi and bfsi
        
        
	if (m_lut[p_security_data->security_level&3] > 0)
    {
    	/*Perform authentication*/
        ccm_star_authentication 
        (
        	0, 
			p_security_data->security_level,// (Security) level
			p_security_data->key,// Key
			p_security_data->nonce,                                     // nonce 
			p_security_data->header,//adata(includes AUX also) 
			p_security_data->header_length,//p_security_data->header_length, // length which includes AUX len also
			p_security_data->payload,// mdata
			p_security_data->payload_length,   //length 
			mic	//output param where MIC is stored							
		);
  
    }
#ifdef WISUN_ENET_PROFILE
    if(send_wrong_mic)
    {
      memcpy(mic,wrong_mic,4);
      send_wrong_mic = 0x00;
    }
#endif
	/* Perform encryption */
	ccm_star_encryption 
	(
                p_security_data->security_level,// (Security) level
                p_security_data->key,// Key
                p_security_data->nonce,// nonce 
                p_security_data->payload,// mdata 
                p_security_data->payload_length , // length 
                mic
	);

          p_security_data->state = MAC_DTX_STATE_ENCRYPTION_COMPLETE; 

          memcpy( &p_security_data->payload[p_security_data->payload_length], mic, m_lut[p_security_data->security_level & 0x03]); 

          //aux_hdr_len = sec_stuff_length(&(((mac_tx_t*)(p_security_data->private_msg_data))->sec_param));

          //aux_hdr_len -= m_lut[p_security_data->security_level&3];

          // Clearout security level in aux header
          //p_security_data->header[p_security_data->header_length-aux_hdr_len] &= 0xF8;
    
#ifdef UTEST_SEC
        utu_timestamp( CCM_STAR_ENCRYPT, ((p_security_data->security_level) | 0x40) );
#endif
     
    return (p_security_data->payload_length + p_security_data->header_length + m_lut[p_security_data->security_level & 0x03]);	
}

/******************************************************************************/
volatile int ccm_decrypt_fail = 0;
static uint8_t Apply_CCM_Star_Decrypt( security_struct_t* p_security_data )
{
        uint8_t mic[16] = {0};	
#ifdef UTEST_SEC
        utu_timestamp( CCM_STAR_DECRYPT, ((p_security_data->security_level) | 0x20) );
#endif
        memset(mic,0,16);
	memcpy( mic, &p_security_data->payload[p_security_data->payload_length], m_lut[p_security_data->security_level & 0x03]); 

	/* Perform decryption */
	ccm_star_encryption 
	(
            p_security_data->security_level, 
            p_security_data->key, 
            p_security_data->nonce, 
            p_security_data->payload, 
            p_security_data->payload_length,
            mic
    );
    
    if (m_lut[p_security_data->security_level&3] > 0)
    {
        /* Perform authentication */  
        if (ccm_star_authentication 
        	(
                    1, 
                    p_security_data->security_level, 
                    p_security_data->key, 
                    p_security_data->nonce,
                    p_security_data->header, 
                    p_security_data->header_length,
                    p_security_data->payload, 
                    p_security_data->payload_length,      // data// excluding the MIC
                    mic // mic
                ) == 0)                  
        {
            p_security_data->state = MAC_RX_STATE_FAILED_AUTHENTICATION;
            ccm_decrypt_fail++;
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
            stack_print_debug ("### CCM_Decryption Failed\n");
            stack_print_debug ("sec level = %d\n", p_security_data->security_level);
            stack_print_debug ("Key = ");
            for (int ii = 0; ii < 16; ii++)
              printf ("%02X ", p_security_data->key[ii]);
            printf ("\n");
            stack_print_debug ("Nonce = ");
            for (int ii = 0; ii < 13; ii++)
              printf ("%02X ", p_security_data->nonce[ii]);
            printf ("\n");
            stack_print_debug ("Header length = %d\n", p_security_data->header_length);
            stack_print_debug ("Payload length = %d\n", p_security_data->payload_length);
#endif
            return(0); // Failed
        }
    }
 
    p_security_data->state = MAC_RX_STATE_DECRYPTION_COMPLETE;

    //memcpy( &pre_msdu[nwkhdrlen], &pre_msdu[nwkhdrlen + SEC_AUXHDR_LENGTH], mdata_len);
    /*modify the rxmsg's payload length member to indicate the length excluding the MIC*/
    ((mac_rx_t*)(p_security_data->private_msg_data))->payload_length -= m_lut[p_security_data->security_level&3];
    
#ifdef UTEST_SEC
        utu_timestamp( CCM_STAR_DECRYPT, ((p_security_data->security_level) | 0x10 ) );
#endif
	
	return 0;
}
#ifndef WISUN_FAN_MAC
uchar get_key_identifier_length( uchar key_id_mode )
{
    return 0;
}
#endif



/******************************************************************************/
/*Suneet :: find device already is add or not if added then reset the frame counter of that device */
uint8_t find_device_already_added_or_not(uint8_t *device_addr, uint8_t *dev_index)
{
  device_descriptor_t *temp;
  uint8_t node_addr[8] = {0x00};
  mem_rev_cpy (node_addr, device_addr, 8);
  uchar i = mac_security_data.pib.mac_device_table_entries;
  
  for (uchar j = 0; j < i; j++)
  {
    temp = (device_descriptor_t *)queue_item_read_from ((queue_t *)&mac_security_data.pib.mac_device_table, j);
    if (temp != NULL_POINTER)
    {
      if (!memcmp(node_addr, temp->ieee_addr, 8))
      {
        if (mac_security_data.pib.mac_sec_frame_counter_perkey == 0)
        {
          temp->frame_count[0] = 0x00000000;/*Suneet :: if device is already added then reset the frame counter to zero */
        }
        else
        {
          temp->frame_count[0] = 0x00000000;/*Suneet :: if device is already added then reset the frame counter to zero */
          temp->frame_count[1] = 0x00000000;/*Suneet :: if device is already added then reset the frame counter to zero */
          temp->frame_count[2] = 0x00000000;/*Suneet :: if device is already added then reset the frame counter to zero */
          temp->frame_count[3] = 0x00000000;/*Suneet :: if device is already added then reset the frame counter to zero */
        }
        return 1;
      }
    }
  }
  *dev_index = mac_security_data.pib.mac_device_table_entries;
  return 0;
}

void del_key_and_device_from_mac_pib(uint8_t *device_addr)
{
      device_descriptor_t *temp;
      uchar i = mac_security_data.pib.mac_device_table_entries;
      for(uchar j = 0;j<=i;j++)
      {
        temp =  (device_descriptor_t*) queue_item_read_from( (queue_t *)&mac_security_data.pib.mac_device_table, 
                                                            j);
        if (temp != NULL_POINTER)
        {
          if(!memcmp(device_addr,
                     temp->ieee_addr,
                     8))
          {
            delete_device_table_entry(j);
            delete_key_table_entry(0);
            break;
          }
        }
      }      
}

void reset_incoming_frame_counter_for_stale_key (uint8_t stale_key_index)
{
  device_descriptor_t *temp;
  uchar i = mac_security_data.pib.mac_device_table_entries;
  
  for(uchar j = 0;j<=i;j++)
  {
    temp =  (device_descriptor_t*) queue_item_read_from( (queue_t *)&mac_security_data.pib.mac_device_table, 
                                                        j);
    if (temp != NULL_POINTER)
    {
      if(mac_security_data.pib.mac_sec_frame_counter_perkey == 0)
        temp->frame_count[0] = 0x00000000;/*Suneet :: if device is already added then reset the frame counter to zero */
      else
        temp->frame_count[stale_key_index] = 0x00000000;/*Suneet :: if device is already added then reset the frame counter to zero */
    }
  }
}

extern  void add_security_key_descriptor_on_MAC();
extern uint8_t key_id_index;

#if(APP_NVM_FEATURE_ENABLED == 1)

void store_device_desc_from_nvm(device_descriptor_t *dev_desc)
{
  queue_item_put( (queue_t *) &mac_security_data.pib.mac_device_table, (queue_item_t*) dev_desc );
  mac_security_data.pib.mac_device_table_entries++;
}

void store_mac_key_table_from_nvm(key_descriptor_t *p_kd)
{
  key_id_index = mac_key_list.active_key_index-1;
  mac_pib.mac_security_enabled = true;
  add_security_key_descriptor_on_MAC();
}

#endif


/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/

/*need to check it is static or not @ Umesh */
//static uint8_t MACSecKey1[16] = {0};
#if WITH_SECURITY //Umesh
uint8_t MACSecKey1[16] = {0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01};
#endif

fan_mac_security mac_key_list;
/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/

/* None */

/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/

/* None */

/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/

#ifdef WISUN_FAN_MAC
extern self_info_fan_mac_t mac_self_fan_info;/****RAka**Temperory Since EAPOL is not present [ 14-June-2017]*/


/*
** ============================================================================
** External Function Declarations
** ============================================================================
*/
#if ((FAN_EAPOL_FEATURE_ENABLED == 1) || (APP_GTK_HARD_CODE_KEY == 1))
extern void MAC_sha256_init(MAC_SHA256_CTX *);
#endif

#endif

/*
** =============================================================================
** Public Variables Definitions
** =============================================================================
**/

/* None */

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

#ifdef WISUN_FAN_MAC
uchar get_key_identifier_length( uchar key_id_mode )
{
    /* Calculate the Key_identifier Length */
    switch(key_id_mode)
    {
    case 0:
        return(0);
    case 1:
        return(1);
    case 2:
        return(5);
    case 3:
        return(9);

    default:
        return(0);
    }
}
#endif

/******************************************************************************/

/****RAka**Temperory Since EAPOL is not present [ 14-June-2017]*/

#ifdef WISUN_FAN_MAC
uint8_t  generate_MAC_Security_Key (uint8_t live_gtk_key_index,uint16_t len)
{
  
#if((FAN_EAPOL_FEATURE_ENABLED == 1) || (APP_GTK_HARD_CODE_KEY == 1))
  
  uint8_t gtk_key_index = 0;
  uint8_t hash[32] = {0};
  uint8_t MACSecKey1[16] = {0x00};
  MAC_SHA256_CTX ctx;
  uint8_t sha256inputbuff[50] = {0};
  mac_key_list.active_key_index = live_gtk_key_index + 1;
  memcpy(&sha256inputbuff[0],mac_self_fan_info.net_name, strlen((const char *)mac_self_fan_info.net_name));
  for(int ii = 0; ii< MAX_MAC_SECURITY_KEY; ii++)
  {
    memset (MACSecKey1, 0, 16);
    memcpy(&sha256inputbuff[strlen((const char *)mac_self_fan_info.net_name)],mac_self_fan_info.mac_gtk_hash_ele.MAC_GTK0_Key+(gtk_key_index*16), 16);
    if (mac_self_fan_info.mac_gtk_hash_ele.gtkl & (0x01 << ii))
    {
      // Hash one
      MAC_sha256_init(&ctx);
      MAC_sha256_update(&ctx,sha256inputbuff,(strlen((const char *)mac_self_fan_info.net_name)+16) );
      MAC_sha256_final(&ctx,hash);
      Truncate_128 ( hash , MACSecKey1);
    }
    mac_key_list.MAC_SECURITY_KEY_LIST[gtk_key_index].MAC_KEY_INDEX =  gtk_key_index + 1 ;
    memcpy(mac_key_list.MAC_SECURITY_KEY_LIST[gtk_key_index].MAC_SECURITY_KEY,MACSecKey1,16);
    gtk_key_index++;
  }
#endif
  
  return 0;
}
  
uint8_t get_mac_active_key_index (void)
{
  return mac_key_list.active_key_index;
}
#else  /*//#ifdef MAC_CFG_SECURITY_ENABLED*/

/******************************************************************************/
/*Umesh : 11-01-2018 need to check why this function in else macro*/
        /*As per rakesh sir this is for compiler*/
uchar get_key_identifier_length( uchar key_id_mode )
{
    return 0;
}

/******************************************************************************/
#endif

#endif


