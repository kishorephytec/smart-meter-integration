/** \file ext_FLASH_app.c
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

/*
********************************************************************************
* File inclusion
********************************************************************************
*/

#include "StackAppConf.h"
#include "common.h"

#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 1)

#include "ext_FLASH_app.h"

/*
********************************************************************************
* Functions
********************************************************************************
*/

#define  TRANS_LENGTH  16
#define  RANDOM_SEED   106
#define  FLASH_TARGET_ADDR  0x00000000 
#define  Error_inc(x)  x = x + 1;
extern void MX25_init( void );

/*
 * Simple flash read/write test
 */

uint8_t   memory_addr[TRANS_LENGTH] = "hello world\r\n";
uint8_t   memory_addr_cmp[TRANS_LENGTH] = {0};
uint32_t  flash_id = 0;
uint16_t  error_cnt = 0;
uint16_t  rems_id;
uint8_t   electric_id = 0;

/*
********************************************************************************
********************************************************************************
*/
// checking the id of flash
uint8_t FlashID_Test( void )
{
    
    FlashStatus  flash_state = {0};
    ReturnMsg  msg;

    /* Read flash device id */
    msg =  MX25_RDID( &flash_id );
    if( msg != (ReturnMsg)FlashOperationSuccess )
        return FALSE;

    msg = MX25_RES( &electric_id );
    if( msg != (ReturnMsg)FlashOperationSuccess )
        return FALSE;

    /* Decide rems_id order. 0: { manufacturer id, device id }
                             1: { device id,  manufacturer id } */
    flash_state.ArrangeOpt = 0;

    msg = MX25_REMS( &rems_id, &flash_state );
    if( msg != (ReturnMsg)FlashOperationSuccess )
        return FALSE;

    /* Compare to expected value */
    if( flash_id != FlashID )
        Error_inc( error_cnt );

    if( electric_id != ElectronicID )
        Error_inc( error_cnt );

    if( flash_state.ArrangeOpt )
    {
        if( rems_id != RESID1 )
           Error_inc( error_cnt );
    }else
    {
        if( rems_id != RESID0 )
           Error_inc( error_cnt );
    }

    if( error_cnt != 0 )
        return FALSE;
    else
        return TRUE;
}

/*
********************************************************************************
********************************************************************************
*/
    
uint8_t FlashReadWrite_Test( void )
{
    //ReturnMsg  message= 0;
    //FlashStatus  flash_state = {0};

    uint32_t  flash_addr;
    uint32_t  trans_len = 0;
    uint16_t  i=0, error_cnt = 0;
    uint16_t  seed = 0;
   // uint8_t   st_reg = 0;


    /* Assign initial condition */
    flash_addr = FLASH_TARGET_ADDR;
    trans_len = TRANS_LENGTH;
    seed = RANDOM_SEED;

    /* Prepare data to transfer */
    srand( seed );
//    for( i=0; i< trans_len; i=i+1 ){
//        memory_addr[i] = rand()%256;   // generate random byte data
//    }

    /* Erase 4K sector of flash memory
       Note: It needs to erase dirty sector before program */
    MX25_SE( flash_addr );

    /* Program data to flash memory */
    MX25_PP( flash_addr, memory_addr, trans_len );

    /* Read flash memory data to memory buffer */
    MX25_READ( flash_addr, memory_addr_cmp, trans_len );

    /* Compare original data and flash data */
    for( i=0; i < (trans_len); i=i+1 )
    {
        if( memory_addr[i] != memory_addr_cmp[i] )
            Error_inc( error_cnt );
    }

    /* Erase 4K sector of flash memory */
    MX25_SE( flash_addr );

    if( error_cnt != 0 )
        return FALSE;
    else
        return TRUE;

}




/*
********************************************************************************
* End
********************************************************************************
*/


#endif //#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 1)

