/** \file p3_ccmstar.h
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

#ifndef P3_CCMSTAR_H
#define P3_CCMSTAR_H

/* Security levels */
#define CCMSTAR_LEVEL_NONE          0
#define CCMSTAR_LEVEL_MIC_32        1
#define CCMSTAR_LEVEL_MIC_64        2
#define CCMSTAR_LEVEL_MIC_128       3 
#define CCMSTAR_LEVEL_ENC           4
#define CCMSTAR_LEVEL_ENC_MIC_32    5
#define CCMSTAR_LEVEL_ENC_MIC_64    6
#define CCMSTAR_LEVEL_ENC_MIC_128   7

uint8_t ccm_star_authentication (uint8_t validate, uint8_t level, uint8_t *key, uint8_t *nonce, uint8_t *header, uint16_t header_length, uint8_t *body, uint16_t body_length, uint8_t *mic);
void ccm_star_encryption (uint8_t level, uint8_t *key, uint8_t *nonce, uint8_t *body, uint16_t body_length, uint8_t *mic);
void ccm_star_encryption_for_pana (uint8_t level, uint8_t *key, uint8_t *nonce, uint8_t *body, uint16_t body_length, uint8_t *mic);

#endif /* P3_CCMSTAR_H */

