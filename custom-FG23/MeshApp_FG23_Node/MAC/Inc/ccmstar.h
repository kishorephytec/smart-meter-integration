/** \file ccmstar.h
 *******************************************************************************
 ** \brief Provides APIs of the CCM Star Algorithm
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

#ifndef CCMSTAR_H
#define CCMSTAR_H

#ifdef __cplusplus
extern "C" {
#endif

/*
** ============================================================================
** Public Macro definitions
** ============================================================================
*/
/* Security levels */
/*! Defines CCMStar level none value*/
#define CCMSTAR_LEVEL_NONE          0
/*! Defines CCMSTAR_LEVEL_MIC_32 value*/
#define CCMSTAR_LEVEL_MIC_32        1
/*! Defines CCMSTAR_LEVEL_MIC_64 value*/
#define CCMSTAR_LEVEL_MIC_64        2
/*! Defines CCMSTAR_LEVEL_MIC_128 value*/
#define CCMSTAR_LEVEL_MIC_128       3 
/*! Defines CCMSTAR_LEVEL_ENC value*/
#define CCMSTAR_LEVEL_ENC           4
/*! Defines CCMSTAR_LEVEL_ENC_MIC_32 value*/
#define CCMSTAR_LEVEL_ENC_MIC_32    5
/*! Defines CCMSTAR_LEVEL_ENC_MIC_64 value*/
#define CCMSTAR_LEVEL_ENC_MIC_64    6
/*! Defines CCMSTAR_LEVEL_ENC_MIC_128 value*/
#define CCMSTAR_LEVEL_ENC_MIC_128   7

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
	
/* None*/

/*
**=========================================================================
**  Public Function Prototypes
**=========================================================================
*/

/**
 *******************************************************************************
 ** @brief Authenticate a frame
 ** @param validate - to check if the frame needs to be validated or not if true
 **                   Compare computed MIC with MIC in frame else last block of 
 **                   output data is the MIC 
 ** @param level - to check if the body should be encrypted or not
 ** @param *key - key to be used for encrption
 ** @param *nonce - pointer to the nonce value
 ** @param *header - pointer from where the header begins
 ** @param header_length - header length value
 ** @param *body - pointer to the location where the actual body starts
 ** @param body_length - length of the body
 ** @param *mic - MIC value
 ** @retval zero - if last block of output data is the MIC
 ** @retval nonzero - if computed MIC is different from the previous MIC
 ******************************************************************************/
uint8_t ccm_star_authentication ( uint8_t validate, 
								  uint8_t level, 
								  uint8_t *key, 
								  uint8_t *nonce, 
								  uint8_t *header, 
								  uint16_t header_length, 
								  uint8_t *body, 
								  uint16_t body_length, 
								  uint8_t *mic
								 );

/**
 *******************************************************************************
 ** @brief Encrypts a frame
 ** @param level - to check if the body should be encrypted or not
 ** @param *key - key to be used for encrption
 ** @param *nonce - pointer to the nonce value
 ** @param *body - pointer to the location where the actual body starts
 ** @param body_length - length of the body
 ** @param *mic - MIC value
 ** @retval zero - if last block of output data is the MIC
 ** @retval nonzero - if computed MIC is different from the previous MIC
 ******************************************************************************/
void ccm_star_encryption ( uint8_t level, 
						   uint8_t *key, 
						   uint8_t *nonce, 
						   uint8_t *body, 
						   uint16_t body_length, 
						   uint8_t *mic
						  );

#ifdef __cplusplus
}
#endif
#endif /* CCMSTAR_H */

