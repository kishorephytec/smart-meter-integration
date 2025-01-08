/*****************************************************************************
* hif_process.c
*****************************************************************************/

/** \file hif_process.c
*******************************************************************************
** \brief This application provides embedded demo for RIT data transmission
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

/**
*****************************************************************************
* @ingroup sysdoc
*
* @{
*****************************************************************************/

/*
********************************************************************************
* File inclusion
********************************************************************************
*/

#if 0 
#include "StackAppConf.h"
#include "common.h"

#if(APP_HIF_PROCESS_FEATURE_ENABLED == 1)
#include "list_latest.h"
#include "queue_latest.h"
#include "buff_mgmt.h"
#include "uart_hal.h"
#include "hif_utility.h"
#include "l3_configuration.h"
#include "l3_process_interface.h"
#include "hif_process.h"


/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/

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
extern hif_t hif;
/*
** =============================================================================
** Private Function Prototypes
** =============================================================================
*/

static uint8_t hif_process_event_handler(l3_process_event_t ev, l3_process_data_t data);
L3_PROCESS(hif_process, "HIF Process");

/*
** =============================================================================
** Private Function Definitions
** =============================================================================
*/


/*
** =============================================================================
** External Variable Declarations
** =============================================================================
*/

/*
** =============================================================================
** External Function Prototypes
** =============================================================================
*/

void ReceiveProcess(void);
void TransmitProcess(void);
void APPhifForToolTest_Init(void);
void hif_service_init(hif_t* p_hif_data);

/*
** =============================================================================
** Public Variables Definitions
** =============================================================================
**/

/*
** =============================================================================
** Public Function Prototypes
** =============================================================================
*/

/*
** =============================================================================
** Public Function Definitions
** =============================================================================
*/

L3_PROCESS_THREAD(hif_process, ev, data)
{
  L3_PROCESS_BEGIN();
  
  while(1) 
  {
    L3_PROCESS_YIELD();
    hif_process_event_handler(ev, data);
  }
  L3_PROCESS_END();
}

/*----------------------------------------------------------------------------*/
static uint8_t hif_process_event_handler(l3_process_event_t ev, l3_process_data_t data)
{
  if (ev == HIF_PROCESS_INIT )
  {
    hif_service_init(&hif);    
    APPhifForToolTest_Init();
  }
  else if (ev == HIF_PROCESS_TX )
  {
    TransmitProcess();  
  }
  else if (ev == HIF_PROCESS_RX )
  {
    ReceiveProcess();
  }
  return 0;
}
/*----------------------------------------------------------------------------*/
static void hif_post_event( l3_process_event_t ev, uint8_t* data)
{
  l3_process_post(&hif_process, ev, data);
}

void hif_process_init (void *data)
{
  hif_post_event (HIF_PROCESS_INIT, (uint8_t *)data);
} 

void hif_process_tx (void *data)
{
  hif_post_event (HIF_PROCESS_TX, (uint8_t *)data);    
} 

void hif_process_rx (void *data)
{
  hif_post_event (HIF_PROCESS_RX, (uint8_t *)data);    
} 
/******************************************************************************/


#endif


#endif // #if 0 

