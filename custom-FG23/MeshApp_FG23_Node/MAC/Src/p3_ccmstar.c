/** \file p3_ccmstar.c
 *******************************************************************************
 ** \brief 
 ** Implements the CCM Star Algorithm 
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
#define AES_128_IN_HW   0

#if (AES_128_IN_HW)/*if H/W AES is enabled*/

/************************* With AES-128 in EFM32 HW ***********************/
#include "common.h"
#include "p3_ccmstar.h"
#include "hw_sec.h"

/* Static function prototypes */

/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/

/* None */
/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/

/* None */

/*
** =============================================================================
** Private Structures, Unions & enums Type Definitions
** =============================================================================
**/

/* None */

/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/

static const uint8_t auth_flags[8] = {0x00, 0x49, 0x59, 0x79, 0x00, 0x49, 0x59, 0x79};
static const uint8_t m_lut[4] = {0, 4, 8, 16};

/*
** =============================================================================
** Public Variable Definitions
** =============================================================================
**/

/* None */

/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/

/* None */

/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/

/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/

static void bytes_to_buffer (uint8_t **h, uint16_t *h_remaining, uint8_t **b, uint16_t *b_remaining, uint8_t *buf, uint16_t buf_length, uint8_t use_b);

/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/
/* Copy next buf_length bytes from header or body */
static void bytes_to_buffer (
    uint8_t **h, uint16_t *h_remaining,    
    uint8_t **b, uint16_t *b_remaining, 
    uint8_t *buf, uint16_t buf_length, 
    uint8_t use_b)
{
  uint8_t consumed = 0;
  
        if (*h_remaining > 0)
        {
              consumed = *h_remaining > buf_length ? buf_length : *h_remaining;

              // For Taiyo Yuden
              memcpy(buf,*h,consumed);

              *h = xor_func (buf, *h, consumed);      
        if (*h_remaining <= consumed) *h_remaining = 0;
        else *h_remaining -= consumed;
              buf += consumed;      
        if (buf_length <= consumed) buf_length = 0;
      else buf_length -= consumed;
    }
  if (use_b && (buf_length > 0) && (*b_remaining > 0))
    {
      consumed = *b_remaining > buf_length ? buf_length : *b_remaining;

	  // For Taiyo Yuden
	  memcpy(buf,*b,consumed);

      *b = xor_func (buf, *b, consumed);      
      if (*b_remaining <= consumed) *b_remaining = 0;
      else *b_remaining -= consumed;
    }
}

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

/* Exclusive Or two arrays */

// For Taiyo Yuden
uint8_t *xor_func (uint8_t *dst, uint8_t *src, uint8_t length)
{
  // For Taiyo Yuden
  return src + length;
}


// Authenticate a frame
uint8_t ccm_star_authentication (    
    uint8_t validate,
    uint8_t level,
    uint8_t *key, uint8_t *nonce,
    uint8_t *header, uint16_t header_length, 
    uint8_t *body, uint16_t body_length,
    uint8_t *mic)
{
  uint8_t m = 0;                                    /* Length of MIC */
  uint8_t encrypt = 0;                              /* Whether body should be encrypted */
  uint8_t datap[16] = {0};
  uint8_t *h = NULL, *b = NULL;
  uint16_t hdr_remaining = 0;                     /* Number of header bytes remaining to be authenticated */
  uint16_t body_remaining = 0;                       /* Number of body bytes remaining to be authenticated */
  uint16_t a_length = 0, m_length = 0;
  
  encrypt = (level & 0x4) != 0;  
  m = m_lut[level & 3]; 

  h = header;
  hdr_remaining = header_length;    
  b = body;
  body_remaining = body_length;    
  if (encrypt)
    {
      a_length = header_length;
      m_length = body_length;
    }
  else 
    {
      a_length = header_length + body_length;   
      m_length = 0;
    }


  // For Taiyo Yuden
  init_cbc_mode(key);

  /* Block 0 */
  datap[0] = auth_flags[level];
  memcpy (&datap[1], nonce, 13);
  datap[14] = ((m_length &0xFF00) >> 8);
  datap[15] = m_length;

  exec_cbc_mode(key, datap);
  

/* Block 1 */      

  /* L(a) is the 2-octets encoding of l(a). Since l(a) which is a_length in our case, does not exceed 255(ENET inetrop and conf tests)
  having the byte as 0x0 works. But if a_length goes beyond 255, then 0th and 1st byte should be 2 byte representation 
  of a_length which needs modification in the below line of code */
  datap[0] = ((a_length &0xFF00) >> 8);
  datap[1] = a_length;   
  
  bytes_to_buffer (&h, &hdr_remaining, &b, &body_remaining, &datap[2], 14, !encrypt);  
  if (a_length <= 14) a_length = 0;
  else a_length -= 14;
  exec_cbc_mode(key, datap);
  

  /* Block n */
  while (a_length > 0)
    {
      bytes_to_buffer (&h, &hdr_remaining, &b, &body_remaining, &datap[0], ((a_length <= 16)?a_length:16), !encrypt); 
     
      if (a_length <= 16)
      {

        // For Taiyo Yuden
        /* 
        The memset is to make sure that the Hardware AES does the proper Encryption 
        only for the required bytes by setting the unwanted bytes to 0, as the 
        Hardware AES always does the encryption of 16 Bytes each time it is called.
        */
        memset(&(datap[a_length]),0x00,(16-a_length));

        a_length = 0;
      }        
      else 
      {
        a_length -= 16; 
      }
      exec_cbc_mode(key, datap);
      
    }

  /* If encryption is being performed, authenticate m left aligned in a new block */
  while (m_length > 0)
    {
      bytes_to_buffer (&h, &hdr_remaining, &b, &body_remaining, &datap[0], ((m_length <= 16)?m_length:16), 1); 
     
      if (m_length <= 16)
      {
            // For Taiyo Yuden
            /* 
            The memset is to make sure that the Hardware AES does the proper Encryption 
            only for the required bytes by setting the unwanted bytes to 0, as the 
            Hardware AES always does the encryption of 16 Bytes each time it is called.
            */
            memset(&(datap[m_length]),0x00,(16-m_length));
            m_length = 0;
      }
      else
      {
        m_length -= 16;
      }
      exec_cbc_mode(key, datap);
     
    }
    
  if (validate) 
    {
      /* Compare computed MIC with MIC in frame */
      return !memcmp (mic, datap, m);
    }
  else
    {
      /* Last block of output datap is the MIC */
      memcpy (mic, datap, m); 
      return 0;
    }
}



/* Encrypt/decrypt a frame */
void ccm_star_encryption (uint8_t level, uint8_t *key, uint8_t *nonce, uint8_t *body, uint16_t body_length, uint8_t *mic)
{
  uint16_t m = 0;                                        /* Length of MIC */
  uint8_t datap[16] = {0};                               /* AES state buffer */
  uint8_t *b = NULL;                             
  uint16_t body_remaining = 0;                           /* Number of bytes remaining to be encrypted */
  uint8_t counter = 0;                                   /* Block counter */

  m = m_lut[level & 3];

  // For Taiyo Yuden
  init_ctr_mode(key);

  /* Encrypt MIC */
  if (m > 0)
    {
      datap[0] = 0x01;
      memcpy (&datap[1], nonce, 13);
      datap[14] = 0x00;
      datap[15] = 0x00;
      exec_ctr_mode(key, datap, &mic, m);
      
    }
 
  /* Encrypt message body */
  if ((level & 0x04) != 0)
    {
      b = body;
      body_remaining = body_length;
      counter = 1;
      while (body_remaining > 0)
        {
          datap[0] = 0x01;
          memcpy (&datap[1], nonce, 13);
          datap[14] = 0x00;
          datap[15] = counter++;
          
          exec_ctr_mode(key, datap, &b, body_remaining);  
          
          if (body_remaining <= 16)
            body_remaining = 0;
          else 
            body_remaining -= 16;
          b += 16;
        }    
    }
}


#else // for #if 0

/******************************* Software AES-128 Encryption *********************************/

                                   /*if S/W AES is enabled*/

/*******************************************************************************
* File inclusion
*******************************************************************************/

#include "common.h"
#include "p3_aes.h"
#include "p3_ccmstar.h"


/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/

/* None */
/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/

/* None */

/*
** =============================================================================
** Private Structures, Unions & enums Type Definitions
** =============================================================================
**/

/* None */

/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/

static const uint8_t auth_flags[8] = {0x00, 0x49, 0x59, 0x79, 0x00, 0x49, 0x59, 0x79};
static const uint8_t m_lut[4] = {0, 4, 8, 16};

/*
** =============================================================================
** Public Variable Definitions
** =============================================================================
**/

/* None */

/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/

/* None */

/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/

/* Static function prototypes */
static uint8_t *xor_func(uint8_t *dst, uint8_t *src, uint8_t length);
static void bytes_to_buffer (uint8_t **h, uint16_t *h_remaining, uint8_t **b, uint16_t *b_remaining, uint8_t *buf, uint16_t buf_length, uint8_t use_b);

/*
** ============================================================================
** External Function Prototypes
** ============================================================================
*/

extern void exec_cbc_mode(uint8_t *key, uint8_t *datap);
extern void exec_ctr_mode(uint8_t *key, uint8_t *datap, uint8_t **b, uint16_t body_remaining);

/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/

/* Exclusive Or two arrays */
static uint8_t *xor_func (uint8_t *dst, uint8_t *src, uint8_t length)
{
	uint8_t i = 0;
  
	for (i = 0; i < length; i++) 
	  dst[i] ^= src[i];
        
  return src + length;
}

/* Copy next buf_length bytes from header or body */
static void bytes_to_buffer (
    uint8_t **h, uint16_t *h_remaining,    
    uint8_t **b, uint16_t *b_remaining, 
    uint8_t *buf, uint16_t buf_length, 
    uint8_t use_b)
{
  uint8_t consumed = 0;
  
  if (*h_remaining > 0)
    {
      consumed = *h_remaining > buf_length ? buf_length : *h_remaining;
      
      *h = xor_func (buf, *h, consumed);      
      if (*h_remaining <= consumed) 
        *h_remaining = 0;
      else 
        *h_remaining -= consumed;
      
      buf += consumed;      
      if (buf_length <= consumed) 
        buf_length = 0;
      else 
        buf_length -= consumed;
    }
  if (use_b && (buf_length > 0) && (*b_remaining > 0))
    {
      consumed = *b_remaining > buf_length ? buf_length : *b_remaining;
      *b = xor_func (buf, *b, consumed);      
      if (*b_remaining <= consumed) 
        *b_remaining = 0;
      else
        *b_remaining -= consumed;
    }
}


/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

// Authenticate a frame
uint8_t ccm_star_authentication (    
    uint8_t validate,
    uint8_t level,
    uint8_t *key, uint8_t *nonce,
    uint8_t *header, uint16_t header_length, 
    uint8_t *body, uint16_t body_length,
    uint8_t *mic)
{
  uint8_t m = 0;                                    /* Length of MIC */
  uint8_t encrypt = 0;                              /* Whether body should be encrypted */
  uint8_t datap[16] = {0};
  uint8_t *h = NULL, *b = NULL;
  uint16_t hdr_remaining = 0;                     /* Number of header bytes remaining to be authenticated */
  uint16_t body_remaining = 0;                       /* Number of body bytes remaining to be authenticated */
  uint16_t a_length = 0, m_length = 0;
  
  encrypt = (level & 0x4) != 0;  
  m = m_lut[level & 3]; 

  h = header;
  hdr_remaining = header_length;    
  b = body;
  body_remaining = body_length;    
  if (encrypt)
    {
      a_length = header_length;
      m_length = body_length;
    }
  else 
    {
      a_length = header_length + body_length;   
      m_length = 0;
    }

  /* Block 0 */
  datap[0] = auth_flags[level];
  memcpy (&datap[1], nonce, 13);
  datap[14] = ((m_length &0xFF00) >> 8);
  datap[15] = m_length;

  exec_cbc_mode(key, datap);
  

/* Block 1 */      

  /* L(a) is the 2-octets encoding of l(a). Since l(a) which is a_length in our case, does not exceed 255(ENET inetrop and conf tests)
  having the byte as 0x0 works. But if a_length goes beyond 255, then 0th and 1st byte should be 2 byte representation 
  of a_length which needs modification in the below line of code */
  datap[0] ^= ((a_length &0xFF00) >> 8);
  datap[1] ^= a_length; 


  bytes_to_buffer (&h, &hdr_remaining, &b, &body_remaining, &datap[2], 14, !encrypt);  
  if (a_length <= 14) a_length = 0;
  else a_length -= 14;
  exec_cbc_mode(key, datap);
  
  /* Block n */
  while (a_length > 0)
    {
          bytes_to_buffer (&h, &hdr_remaining, &b, &body_remaining, &datap[0], ((a_length <= 16)?a_length:16), !encrypt); 
          if (a_length <= 16)
          {
            a_length = 0;
          }        
          else 
          {
            a_length -= 16; 
          }
          exec_cbc_mode(key, datap);
         
    }

  /* If encryption is being performed, authenticate m left aligned in a new block */
  while (m_length > 0)
    {
      bytes_to_buffer (&h, &hdr_remaining, &b, &body_remaining, &datap[0], ((m_length <= 16)?m_length:16), 1); 
     
      if (m_length <= 16)
      {
        m_length = 0;
      }
      else
      {
        m_length -= 16;
      }
      exec_cbc_mode(key, datap);
    }
    
  if (validate) 
    {
      /* Compare computed MIC with MIC in frame */
      return !memcmp (mic, datap, m);
    }
  else
    {
      /* Last block of output datap is the MIC */
      memcpy (mic, datap, m); 
      return 0;
    }
}


/* Encrypt/decrypt a frame */
void ccm_star_encryption (uint8_t level, uint8_t *key, uint8_t *nonce, uint8_t *body, uint16_t body_length, uint8_t *mic)
{
  uint16_t m = 0;                                   /* Length of MIC */
  uint8_t datap[16] = {0};                          /* AES state buffer */
  uint8_t *b = NULL;                                 
  uint16_t body_remaining = 0;                      /* Number of bytes remaining to be encrypted */
  uint8_t counter = 0;                              /* Block counter */

  m = m_lut[level & 3];
  
  /* Encrypt MIC */
  if (m > 0)
    {
      datap[0] = 0x01;
      memcpy (&datap[1], nonce, 13);
      datap[14] = 0x00;
      datap[15] = 0x00;
      exec_ctr_mode(key, datap, &mic, m);
      /* Reason for using 16 in the next statement to decrement the mic buffer pointer:
         To make the mic to point to the proper location where the actual encrypted 
         mic is present, the mic is needed to be decremented by 16 Bytes location 
         as the above function increments this mic inside it by 16 Bytes which is 
         not required when encrypting the MIC. 
      */
    }
 
  /* Encrypt message body */
  if ((level & 0x04) != 0)
    {
      b = body;
      body_remaining = body_length;
      counter = 1;
      while (body_remaining > 0)
        {
          datap[0] = 0x01;
          memcpy (&datap[1], nonce, 13);
          datap[14] = 0x00;
          datap[15] = counter++;
          
          exec_ctr_mode(key, datap, &b, body_remaining);  

          if (body_remaining <= 16) 
            body_remaining = 0;
          else 
            body_remaining -= 16;
          
          b += 16;
        }    
    }
}

//void aes(const uint8_t *key, uint8_t *datap)
//{
//  
//}

void exec_cbc_mode(uint8_t *key, uint8_t *datap)
{
  aes(key, datap);
}

void exec_ctr_mode(uint8_t *key, uint8_t *datap, uint8_t **b, uint16_t body_remaining)
{   
  uint8_t datalen = 0x00;//*((uint16_t*)&body_remaining);
  
//  uint16_t datalen = (uint16_t)*((uint16_t*)&body_remaining);
//  
//  memcpy((uint8_t*)&datalen,body_remaining,2);
  datalen = (uint8_t) (body_remaining > 16 ? 16 : body_remaining);
    
    aes (key, datap);   
    xor_func (*b, datap, datalen );
}



#endif

