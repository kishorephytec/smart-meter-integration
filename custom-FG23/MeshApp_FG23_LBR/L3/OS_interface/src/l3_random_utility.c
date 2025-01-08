#include "l3_random_utility.h"

void l3_random_init (unsigned short seed)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)
  random_init (seed);
#endif
}

unsigned short l3_random_rand (void)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return 0;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)
  return random_rand ();
#endif
}

