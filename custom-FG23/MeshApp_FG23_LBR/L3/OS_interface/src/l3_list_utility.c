#include <stdio.h>
#include "l3_list_utility.h"

void l3_list_init (l3_list_t list)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)
  list_init (list);
#endif
}
void *l3_list_head (l3_list_t list)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return NULL;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)
  return list_head (list);
#endif
}
void *l3_list_tail (l3_list_t list , void *item)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return NULL;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)
  return list_tail (list);
#endif
}
void *l3_list_pop (l3_list_t list)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return NULL;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)
  return list_pop (list);
#endif
}
void l3_list_push (l3_list_t list, void *item)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)
  list_push (list, item);
#endif
}
void *l3_list_chop (l3_list_t list)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return NULL;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)
  return list_chop (list);
#endif
}
void l3_list_add (l3_list_t list, void *item)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)
  list_add (list, item);
#endif
}
void l3_list_remove (l3_list_t list, void *item)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)
  list_remove (list, item);
#endif
}
int l3_list_length (l3_list_t list)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return 0;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)
  return list_length (list);
#endif
}
void l3_list_copy (l3_list_t dest, l3_list_t src)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)
  list_copy (dest, src);
#endif
}
void l3_list_insert (l3_list_t list, void *previtem, void *newitem)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)
  list_insert (list, previtem, newitem);
#endif
}
void *l3_list_item_next (void *item)
{
#if (L3_SUPPORTING_OS == OS_NONE) 
  return NULL;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)
  return list_item_next (item);
#endif
}