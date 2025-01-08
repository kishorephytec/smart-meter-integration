/** \file p3_aes.c
 *******************************************************************************
 ** \brief 
 ** Implements the AES Cipher Block
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

#include "common.h"
#include "p3_aes.h"
#include "p3_ccmstar.h"

/* Default to 8-bit implementation */
#ifndef __bits__
#define __bits__ 8
#endif

/* Local function prototypes */
static void sub_bytes_and_shift_rows(uint8_t *datap);
static uint8_t gfmul2(uint8_t b);
static void mix_columns(uint8_t *datap);
static void add_round_key(uint8_t *key, uint8_t *datap);
static void expand_key(uint8_t *key, int round);

/* AES s-box lookup table */
static const uint8_t sbox[256] = 
{
  0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
  0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
  0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
  0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
  0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
  0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
  0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
  0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2, 
  0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
  0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
  0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
  0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
  0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a, 
  0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
  0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
  0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16, 
};

/* AES round constant used in key expansion */
static const uint8_t rc[11] = 
{
  0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36
};

/* AES SubBytes and ShiftRows transforms */
static void sub_bytes_and_shift_rows(uint8_t *datap)
{
  uint8_t temp1, temp2;

  datap[0] = sbox[datap[0]];
  datap[4] = sbox[datap[4]];
  datap[8] = sbox[datap[8]];
  datap[12] = sbox[datap[12]]; 
  temp1 = sbox[datap[1]]; 
  datap[1] = sbox[datap[5]];
  datap[5] = sbox[datap[9]];
  datap[9] = sbox[datap[13]];
  datap[13] = temp1;
  temp1 = sbox[datap[2]];
  datap[2] = sbox[datap[10]];
  datap[10] = temp1;
  temp1 = sbox[datap[6]];
  datap[6] = sbox[datap[14]];
  datap[14] = temp1;
  temp1 = sbox[datap[3]];
  datap[3] = sbox[datap[15]];
  temp2 = sbox[datap[7]];
  datap[7] = temp1;
  temp1 = sbox[datap[11]];
  datap[11] = temp2;
  datap[15] = temp1;
}

/* Multiply b by 02 in GF(2^8) */
static uint8_t gfmul2(uint8_t b)
{
#ifdef __barrel_shift_enabled__
  static const uint8_t poly[2] = {0x00, 0x1b};
  
  return (b << 1) ^ (poly[b >> 7]);
#else  
  return (b << 1) ^ (((b & 0x80) != 0) ? 0x1b : 0x00);
#endif
}

/* AES MixColumns transform */
static void mix_columns(uint8_t *datap)
{  
#if defined(__excess__) && defined(__aes_enabled__) && (__bits__ == 32) && defined(__GNUC__)
  unsigned long *d = (unsigned long *)datap;
  
  __asm__ ("aesmc\t%0, %1" : "=d" (d[0]) : "d" (d[0]));
  __asm__ ("aesmc\t%0, %1" : "=d" (d[1]) : "d" (d[1]));
  __asm__ ("aesmc\t%0, %1" : "=d" (d[2]) : "d" (d[2]));
  __asm__ ("aesmc\t%0, %1" : "=d" (d[3]) : "d" (d[3]));
#else
  uint8_t t, u;
  uint8_t *a;
#if __bits__ == 8
  uint8_t i;
#else
  int i;
#endif  

  for (i = 0, a = datap; i < 4; i++, a += 4) 
    {
      t = a[0] ^ a[1] ^ a[2] ^ a[3];
      u = a[0];
      a[0] ^= gfmul2 (a[0] ^ a[1]) ^ t;
      a[1] ^= gfmul2 (a[1] ^ a[2]) ^ t;
      a[2] ^= gfmul2 (a[2] ^ a[3]) ^ t;
      a[3] ^= gfmul2 (a[3] ^ u) ^ t;
    }
#endif  
}

/* AES AddRoundKey */
static void add_round_key(uint8_t *key, uint8_t *datap)
{ 
#if __bits__ == 32
  unsigned long *k = (unsigned long *)key;
  unsigned long *d = (unsigned long *)datap;
  
  d[0] ^= k[0];
  d[1] ^= k[1];
  d[2] ^= k[2];
  d[3] ^= k[3];
#else
  uint8_t i;
  
  for (i = 0; i < 16; i++)
      datap[i] ^= key[i];
#endif      
}

/* AES key expansion */
static void expand_key(uint8_t *key, int round)
{
#if __bits__ == 32
  unsigned long *k;
  
  key[0] ^= sbox[key[13]] ^ rc[round];
  key[1] ^= sbox[key[14]];
  key[2] ^= sbox[key[15]];
  key[3] ^= sbox[key[12]];          
  k = (unsigned long *)key;
  k[1] ^= k[0];
  k[2] ^= k[1];
  k[3] ^= k[2];  
#else
  uint8_t *k;
  uint8_t i;
  
  key[0] ^= sbox[key[13]] ^ rc[round];
  key[1] ^= sbox[key[14]];
  key[2] ^= sbox[key[15]];
  key[3] ^= sbox[key[12]];      
  for (k = key, i = 1; i < 4; k += 4, i++) 
    {
      k[4] ^= k[0];
      k[5] ^= k[1];
      k[6] ^= k[2];
      k[7] ^= k[3];      
    }         
#endif  
}

/* AES block cipher. Fixed to 128-bit block size.
 *  
 * key - Pointer to 128-bit key (not modified)
 * datap - Pointer to 128-bit data (modified)
 */
void aes(const uint8_t *key, uint8_t *datap)
{
  uint8_t ekey[16];
#if __bits__ == 32
  int i;
#else
  uint8_t i;
#endif 
  memcpy (ekey, key, sizeof(ekey));
  add_round_key (ekey, datap);
  for (i = 1; i < 10; i++)
    {
      expand_key (ekey, i);
      sub_bytes_and_shift_rows (datap);
      mix_columns (datap);
      add_round_key (ekey, datap);   
    }
  expand_key (ekey, i);
  sub_bytes_and_shift_rows (datap);
  add_round_key (ekey, datap);  
}
