/** \file l3_memb_utility.h
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


#ifndef _L3_MEMB_UTILITY_H_
#define _L3_MEMB_UTILITY_H_

#include "l3_configuration.h"
#if (L3_SUPPORTING_OS == OS_CONTIKI)
#include "lib/memb.h"
#endif


#if (L3_SUPPORTING_OS == OS_NONE)
struct l3_memb {
  unsigned short size;
  unsigned short num;
  char *count;
  void *mem;
};  
#endif

#if (L3_SUPPORTING_OS == OS_NONE)
#define L3_MEMB(name, structure, num)      static char name##_memb_count[num]; \
                                            static structure name##_memb_mem[num]; \
                                              static struct l3_memb name = {sizeof(structure), num, \
                                                name##_memb_count, \
                                                  (void *)name##_memb_mem}
#elif (L3_SUPPORTING_OS == OS_CONTIKI)
#define L3_MEMB(name, structure, num)      MEMB(name, structure, num)
#endif
                 
                                                  
void l3_memb_init (void *m);
void *l3_memb_alloc (void *m);
char l3_memb_free (void *m, void *ptr);
int l3_memb_inmemb (void *m, void *ptr);
int l3_memb_numfree (void *m);

#endif //_L3_MEMB_UTILITY_H_