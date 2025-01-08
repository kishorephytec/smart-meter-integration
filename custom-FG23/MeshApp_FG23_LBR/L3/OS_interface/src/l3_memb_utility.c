#include <stdio.h>
#include "l3_memb_utility.h"

void l3_memb_init (void *m)
{
#if (L3_SUPPORTING_OS == OS_NONE)
  return;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  memb_init ((struct memb *)m);
#endif
}

void *l3_memb_alloc (void *m)
{
#if (L3_SUPPORTING_OS == OS_NONE)
  return NULL;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  return memb_alloc ((struct memb *)m);
#endif
}

char l3_memb_free (void *m, void *ptr)
{
#if (L3_SUPPORTING_OS == OS_NONE)
  return 0;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  return memb_free ((struct memb *)m, ptr);
#endif
}

int l3_memb_inmemb (void *m, void *ptr)
{
#if (L3_SUPPORTING_OS == OS_NONE)
  return 0;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  return memb_inmemb ((struct memb *)m, ptr);
#endif
}

int l3_memb_numfree (void *m)
{
#if (L3_SUPPORTING_OS == OS_NONE)
  return 0;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  return memb_numfree ((struct memb *)m);
#endif
}