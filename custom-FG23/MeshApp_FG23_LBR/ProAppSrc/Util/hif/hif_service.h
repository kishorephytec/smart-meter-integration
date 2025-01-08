/** \file hif_service.h
 *******************************************************************************
 ** \brief provides APIs for using HIF service.
 **
 ** This file contains the macros used to work with the LED's and the API's to 
 ** work with LED operations and PUSH buttons. 
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

#ifndef _HIF_SERVICE_H_
#define _HIF_SERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
** ============================================================================
** Public Macro definitions
** ============================================================================
*/

/**
 ** \defgroup hif_service  HIF Service Module
 */

/*@{*/

/**
 ** \defgroup hif_service_defs  HIF Service Definitions
 ** \ingroup hif_service
 */

/*@{*/

/*@}*/

/* None */

/*
** ============================================================================
** Public Structures, Unions & enums Type Definitions
** ============================================================================
*/

/* None */

/*
** ============================================================================
** Public Variable Declarations
** ============================================================================
*/

/* None */

/*
** ============================================================================
** Public Function Prototypes
** ============================================================================
*/

/**
 ** \defgroup hif_service_func  HIF Service APIs
 ** \ingroup hif_service
 */

/*@{*/

/**
 *******************************************************************************
 ** \brief Initializes the host interface service 
 ** \param *p_hif_data[in] - context holding all the information about the 
 **								host interface
 ** \retval - None
 ** \note None
 ******************************************************************************/
void hif_service_init(hif_t* p_hif_data);

/**
 *******************************************************************************
 ** \brief Initializes the task which takes care of sending/receiving packets 
 ** to/from PC host application 
 ** \param - None 
 ** \retval - None
 ** \note None
 ******************************************************************************/
void hif_task_init(void);

/**
 *******************************************************************************
 ** \brief Implements the task which takes care of sending/receiving packets 
 ** to/from PC host application
 ** \param *p_hif[in] - 
 ** \retval - None
 ** \note - None
 ******************************************************************************/
void hif_task(hif_t* p_hif);

/*@}*/

/*@}*/

void ReceiveProcess(void);
void TransmitProcess(void);
 
#ifdef __cplusplus
  }
#endif

#endif /*_HIF_SERVICE_H_*/

