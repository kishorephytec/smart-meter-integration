/** \file l3_list_utility.h
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


#ifndef _L3_LIST_UTILITY_H_
#define _L3_LIST_UTILITY_H_


#include "l3_configuration.h"
#if (L3_SUPPORTING_OS == OS_CONTIKI) 
#include "lib/list.h"
#endif


#if (L3_SUPPORTING_OS == OS_NONE) 
#define L3_LIST(name)                                   static void *name##_list = NULL; static l3_list_t name = (l3_list_t)&name##_list
#define L3_LIST_STRUCT(name)                            void *name##_list; l3_list_t name
#define L3_LIST_STRUCT_INIT(struct_ptr, name)           do {                                                    \
                                                           (struct_ptr)->name = &((struct_ptr)->name##_list);   \
                                                             (struct_ptr)->name##_list = NULL;                  \
                                                               l3_list_init((struct_ptr)->name);                \
                                                        } while(0) 
#elif (L3_SUPPORTING_OS == OS_CONTIKI)
#define L3_LIST(name)                                   LIST(name)
#define L3_LIST_STRUCT(name)                            LIST_STRUCT(name)
#define L3_LIST_STRUCT_INIT(struct_ptr, name)           LIST_STRUCT_INIT(struct_ptr, name)  
#endif

#if (L3_SUPPORTING_OS == OS_NONE) 
typedef void **         l3_list_t;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)
typedef list_t          l3_list_t;
#endif


void   l3_list_init             (l3_list_t list);
void * l3_list_head             (l3_list_t list);
void * l3_list_tail             (l3_list_t list, void *item);
void * l3_list_pop              (l3_list_t list);
void   l3_list_push             (l3_list_t list, void *item);
void * l3_list_chop             (l3_list_t list);
void   l3_list_add              (l3_list_t list, void *item);
void   l3_list_remove           (l3_list_t list, void *item);
int    l3_list_length           (l3_list_t list);
void   l3_list_copy             (l3_list_t dest, l3_list_t src);
void   l3_list_insert           (l3_list_t list, void *previtem, void *newitem);
void * l3_list_item_next        (void *item);

#endif //_L3_LIST_UTILITY_H_