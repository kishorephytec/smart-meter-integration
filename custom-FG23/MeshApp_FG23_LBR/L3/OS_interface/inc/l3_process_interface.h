/** \file l3_process_interface.h
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

#ifndef _L3_PROCESS_INTERFACE_H_
#define _L3_PROCESS_INTERFACE_H_

#include "l3_configuration.h"

#if (L3_SUPPORTING_OS == OS_CONTIKI)
#include "process.h"
#endif

#if (L3_SUPPORTING_OS == OS_NONE)
#define L3_PROCESS_NONE                                 NULL
#define L3_PROCESS_EVENT_NONE                           1
#define L3_PROCESS_EVENT_INIT                           2
#define L3_PROCESS_EVENT_POLL                           3
#define L3_PROCESS_EVENT_EXIT                           4
#define L3_PROCESS_EVENT_SERVICE_REMOVED                5
#define L3_PROCESS_EVENT_CONTINUE                       6
#define L3_PROCESS_EVENT_MSG                            7
#define L3_PROCESS_EVENT_EXITED                         8
#define L3_PROCESS_EVENT_TIMER                          9
#define L3_PROCESS_EVENT_COM                            10
#define L3_PROCESS_EVENT_MAX                            11

#elif (L3_SUPPORTING_OS == OS_CONTIKI)
                                                        
#define L3_PROCESS_NONE                                 PROCESS_NONE
#define L3_PROCESS_EVENT_NONE                           PROCESS_EVENT_NONE
#define L3_PROCESS_EVENT_INIT                           PROCESS_EVENT_INIT
#define L3_PROCESS_EVENT_POLL                           PROCESS_EVENT_POLL
#define L3_PROCESS_EVENT_EXIT                           PROCESS_EVENT_EXIT
#define L3_PROCESS_EVENT_SERVICE_REMOVED                PROCESS_EVENT_SERVICE_REMOVED
#define L3_PROCESS_EVENT_CONTINUE                       PROCESS_EVENT_CONTINUE
#define L3_PROCESS_EVENT_MSG                            PROCESS_EVENT_MSG
#define L3_PROCESS_EVENT_EXITED                         PROCESS_EVENT_EXITED
#define L3_PROCESS_EVENT_TIMER                          PROCESS_EVENT_TIMER
#define L3_PROCESS_EVENT_COM                            PROCESS_EVENT_COM
#define L3_PROCESS_EVENT_MAX                            PROCESS_EVENT_MAX
#endif


#if (L3_SUPPORTING_OS == OS_NONE)
typedef unsigned char 	l3_process_event_t;
typedef void *        	l3_process_data_t;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)
typedef process_event_t l3_process_event_t;
typedef process_data_t  l3_process_data_t;
#endif

#if (L3_SUPPORTING_OS == OS_NONE)
struct l3_process {
  struct l3_process *next;
  void ((* thread)(void *, l3_process_event_t, l3_process_data_t));
//  struct pt pt;
//  unsigned char state, needspoll;
};
#endif

#if (L3_SUPPORTING_OS == OS_NONE)
#define L3_PROCESS_BEGIN()				
#define L3_PROCESS_END()				
#define L3_PROCESS_WAIT_EVENT()				
#define L3_PROCESS_WAIT_EVENT_UNTIL(c)			
#define L3_PROCESS_YIELD()				
#define L3_PROCESS_YIELD_UNTIL(c)			
#define L3_PROCESS_WAIT_UNTIL(c)			
#define L3_PROCESS_WAIT_WHILE(c)			
#define L3_PROCESS_EXIT()				
#define L3_PROCESS_PT_SPAWN(pt, thread)			
#define L3_PROCESS_PAUSE()				
#define L3_PROCESS_POLLHANDLER(handler)			
#define L3_PROCESS_EXITHANDLER(handler)			
#define L3_PROCESS_THREAD(name, ev, data)		static void process_thread_##name (void *process_pt, l3_process_event_t ev, l3_process_data_t data)
#define L3_PROCESS_NAME(name)				
#define L3_PROCESS(name, strname)                       L3_PROCESS_THREAD(name, ev, data); struct l3_process name = {NULL, process_thread_##name}
#define L3_PROCESS_NAME_STRING(process)			
#define L3_PROCESS_CURRENT()				NULL
#define L3_PROCESS_CONTEXT_BEGIN(p)			
#define L3_PROCESS_CONTEXT_END(p)			

#elif (L3_SUPPORTING_OS == OS_CONTIKI)
	
#define L3_PROCESS_BEGIN()				PROCESS_BEGIN()
#define L3_PROCESS_END()				PROCESS_END()
#define L3_PROCESS_WAIT_EVENT()				PROCESS_WAIT_EVENT()
#define L3_PROCESS_WAIT_EVENT_UNTIL(c)			PROCESS_WAIT_EVENT_UNTIL(c)
#define L3_PROCESS_YIELD()				PROCESS_YIELD()
#define L3_PROCESS_YIELD_UNTIL(c)			PROCESS_YIELD_UNTIL(c)
#define L3_PROCESS_WAIT_UNTIL(c)			PROCESS_WAIT_UNTIL(c)
#define L3_PROCESS_WAIT_WHILE(c)			PROCESS_WAIT_WHILE(c)
#define L3_PROCESS_EXIT()				PROCESS_EXIT()
#define L3_PROCESS_PT_SPAWN(pt, thread)			PROCESS_PT_SPAWN(pt, thread)
#define L3_PROCESS_PAUSE()				PROCESS_PAUSE()
#define L3_PROCESS_POLLHANDLER(handler)			PROCESS_POLLHANDLER(handler)
#define L3_PROCESS_EXITHANDLER(handler)			PROCESS_EXITHANDLER(handler)
#define L3_PROCESS_THREAD(name, ev, data)		PROCESS_THREAD(name, ev, data)
#define L3_PROCESS_NAME(name)				PROCESS_NAME(name)
#define L3_PROCESS(name, strname)			PROCESS(name, strname)
#define L3_PROCESS_NAME_STRING(process)			PROCESS_NAME_STRING(process)
#define L3_PROCESS_CURRENT()				PROCESS_CURRENT()
#define L3_PROCESS_CONTEXT_BEGIN(p)			PROCESS_CONTEXT_BEGIN(p)
#define L3_PROCESS_CONTEXT_END(p)			PROCESS_CONTEXT_END(p)
#endif

void l3_process_start (void *p, l3_process_data_t data);
int l3_process_post (void *p, l3_process_event_t ev, l3_process_data_t data);
void l3_process_post_synch (void *p, l3_process_event_t ev, l3_process_data_t data);
l3_process_event_t l3_process_alloc_event (void);
#endif //_L3_PROCESS_INTERFACE_H_