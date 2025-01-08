
#include "l3_timer_utility.h"

void l3_ctimer_reset (l3_ctimer_t *c)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return;
#elif (L3_SUPPORTING_OS == OS_CONTIKI) 
  ctimer_reset (&(c->ct));
#endif
}

void l3_ctimer_restart (l3_ctimer_t *c)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return;
#elif (L3_SUPPORTING_OS == OS_CONTIKI) 
  ctimer_restart (&(c->ct));
#endif
}

void l3_ctimer_set (l3_ctimer_t *c, l3_clock_time_t t, void (*f)(void *), void *ptr)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return;
#elif (L3_SUPPORTING_OS == OS_CONTIKI) 
  ctimer_set (&(c->ct), t, f, ptr);
#endif
}

void l3_ctimer_stop (l3_ctimer_t *c)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return;
#elif (L3_SUPPORTING_OS == OS_CONTIKI) 
  ctimer_stop (&(c->ct));
#endif
}

int l3_ctimer_expired (l3_ctimer_t *c)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return 0;
#elif (L3_SUPPORTING_OS == OS_CONTIKI) 
  return ctimer_expired (&(c->ct));
#endif
}

l3_clock_time_t l3_ctimer_expiration_time (l3_ctimer_t *c)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return 0;
#elif (L3_SUPPORTING_OS == OS_CONTIKI) 
  return etimer_expiration_time (&(c->ct.etimer));
#endif
}





void l3_stimer_set (l3_stimer_t *t, unsigned long interval)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  stimer_set (&(t->st), interval);
#endif
}

void l3_stimer_reset (l3_stimer_t *t)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  stimer_reset (&(t->st));
#endif
}

void l3_stimer_restart (l3_stimer_t *t)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  stimer_restart (&(t->st));
#endif
}

int l3_stimer_expired (l3_stimer_t *t)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return 0;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  return stimer_expired (&(t->st));
#endif
}

unsigned long l3_stimer_remaining (l3_stimer_t *t)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return 0;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  return stimer_remaining (&(t->st));
#endif
}

unsigned long l3_stimer_elapsed (l3_stimer_t *t)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return 0;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  return stimer_elapsed (&(t->st));
#endif
}





void l3_etimer_set (l3_etimer_t *et, l3_clock_time_t interval)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  etimer_set (&(et->et), interval);
#endif
}

void l3_etimer_reset (l3_etimer_t *et)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  etimer_reset (&(et->et));
#endif
}

void l3_etimer_reset_with_new_interval (l3_etimer_t *et, l3_clock_time_t interval)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  etimer_reset_with_new_interval (&(et->et), interval);
#endif
}

void l3_etimer_restart (l3_etimer_t *et)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  etimer_restart (&(et->et));
#endif
}

void l3_etimer_adjust (l3_etimer_t *et, int td)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  etimer_adjust (&(et->et), td);
#endif
}

l3_clock_time_t l3_etimer_expiration_time (l3_etimer_t *et)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return 0;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  return etimer_expiration_time (&(et->et));
#endif
}

l3_clock_time_t l3_etimer_start_time (l3_etimer_t *et)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return 0;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  return etimer_start_time (&(et->et));
#endif
}

int l3_etimer_expired (l3_etimer_t *et)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return 0;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  return etimer_expired (&(et->et));
#endif
}

void l3_etimer_stop (l3_etimer_t *et)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  etimer_stop (&(et->et));
#endif
}

void l3_etimer_request_poll (void)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  etimer_request_poll ();
#endif
}
  
int l3_etimer_pending (void)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return 0;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  return etimer_pending ();
#endif
}
  
l3_clock_time_t l3_etimer_next_expiration_time (void)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return 0;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  return etimer_next_expiration_time ();
#endif
}
  
  
  
  
l3_clock_time_t l3_clock_time (void)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return 0;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  return clock_time ();
#endif
}



void l3_timer_set (l3_timer_t *t, l3_clock_time_t interval)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)
  timer_set (&(t->t), interval);
#endif
}

void l3_timer_reset (l3_timer_t *t)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)
  timer_reset (&(t->t));
#endif
}

void l3_timer_restart (l3_timer_t *t)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)
  timer_restart (&(t->t));
#endif  
}

int l3_timer_expired (l3_timer_t *t)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return 0;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)
  return timer_expired (&(t->t));
#endif  
}

l3_clock_time_t l3_timer_remaining (l3_timer_t *t)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return 0;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)
  return timer_remaining (&(t->t));
#endif  
}
    