/** \file common.h
 *******************************************************************************
 ** \brief  Contains all the basics typedefs that will be commonly will be used 
 ** by most of the modules.
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
 
#ifndef _COMMON_H_
#define _COMMON_H_

//#ifdef EFM32_TARGET_IAR //EFM32_TARGET_IAR

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> 
#include <stdbool.h>
#include <intrinsics.h>
#include <math.h>


/***************************************************************************//**
 ******************************************************************************/


typedef uint8_t 		bitfield;
typedef unsigned char  		boolean;
typedef bool gboolean;

#define true 1
#define false 0

typedef uint32_t 			base_t;
typedef int32_t 			sbase_t;
typedef base_t 				stime_t;
typedef base_t 				p3time_t;

typedef  uint8_t  			ubit;
typedef  ubit				uchar;
typedef  uint16_t			ushort;
typedef  uint32_t			ulong;

typedef uint8_t 			uint8;
typedef ushort 				uint16;


typedef struct uint128_tag {
   uint8_t a[16];		
} uint128_t;

#define LO_BYTE(x) ((uchar)(x&0xFF))
#define HI_BYTE(x) ((uchar)(x>>8))

#define HIGH_BYTE(x) ((x) >> 8)
#define LOW_BYTE(x) ((x) & 0xff)

#define HILO_BYTE(x) ((uchar)(x>>16))
#define HIHI_BYTE(x) ((uchar)(x>>24))

/* macro for getting and putting a ushort embedded in a byte array in LE order */
#define get_ushort(bp) (((ushort)*(bp))+((ushort)(*((bp)+1))<<8))
#define put_ushort(bp,value) (bp)[0]=(uchar)(value)&0xFF;(bp)[1]=(uchar)((value)>>8)
#define get_ushort_BE(bp) (((ushort)*((bp)+1))+((ushort)(*(bp))<<8))
#define put_ushort_BE(bp,value) (bp)[1]=(uchar)(value)&0xFF;(bp)[0]=(uchar)((value)>>8)
#define get_ulong_BE(bp) (((ulong)*(bp+3))+((ulong)(*((bp)+2))<<8)+((ulong)(*((bp)+1))<<16)+((ulong)(*((bp)))<<24))
#define get_ulong(bp) (((ulong)*(bp))+((ulong)(*((bp)+1))<<8)+((ulong)(*((bp)+2))<<16)+((ulong)(*((bp)+3))<<24))
#define put_ulong(bp,value) (bp)[0]=(uchar)(value)&0xFF;(bp)[1]=(uchar)(((value)>>8)&0xFF);(bp)[2]=(uchar)(((value)>>16)&0xFF);(bp)[3]=(uchar)(((value)>>24)&0xFF)


#ifndef NULL
#define NULL                        ((void*)0)
#endif

#define NULL_POINTER                0x0
#define NULL_BYTE                   ((uchar)0x00)


#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((unsigned long) &((TYPE *)0)->MEMBER)
#endif

#define MAX_BASE_VALUE 		(~((base_t)0x0))



#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FAILURE
#define FAILURE 1
#endif



#endif  //_COMMON_H
