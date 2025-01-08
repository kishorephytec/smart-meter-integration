#include "l3_process_interface.h"

void l3_process_start (void *p, l3_process_data_t data)
{
#if (L3_SUPPORTING_OS == OS_NONE)
  return;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)
  process_start ((struct process *)p, data);
#endif
}

int l3_process_post (void *p, l3_process_event_t ev, l3_process_data_t data)
{
#if (L3_SUPPORTING_OS == OS_NONE)
  return 0;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)
  return process_post ((struct process *)p, ev, data);
#endif
}

void l3_process_post_synch (void *p, l3_process_event_t ev, l3_process_data_t data)
{
#if (L3_SUPPORTING_OS == OS_NONE)
  return;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  process_post_synch ((struct process *)p, ev, data);
#endif
}

l3_process_event_t l3_process_alloc_event (void)
{
#if (L3_SUPPORTING_OS == OS_NONE)
  return 0;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  return process_alloc_event ();
#endif
}