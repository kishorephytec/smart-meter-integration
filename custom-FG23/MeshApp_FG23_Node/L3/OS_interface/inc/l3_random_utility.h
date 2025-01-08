/** \file l3_random_utility.h
 *******************************************************************************
 ** \brief  Contains all the Processor architecture selection macros for selection 
 ** of processor Architecture.
 **
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

#ifndef _L3_RANDOM_UTILITY_H_
#define _L3_RANDOM_UTILITY_H_



#include "l3_configuration.h"
#if (L3_SUPPORTING_OS == OS_CONTIKI) 
#include "lib/random.h"
#endif

#if (L3_SUPPORTING_OS == OS_NONE) 
#define L3_RANDOM_RAND_MAX      65535U
#elif (L3_SUPPORTING_OS == OS_CONTIKI)
#define L3_RANDOM_RAND_MAX      RANDOM_RAND_MAX
#endif

void l3_random_init (unsigned short seed);
unsigned short l3_random_rand (void);

#endif //_L3_RANDOM_UTILITY_H_