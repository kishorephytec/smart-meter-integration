/**
 * Copyright (c) 2008, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 **/
#include "Stack6LoWPANConf.h"
#include <stdio.h>
#include "contiki-net.h"

#define MAX_NUM_QUEUED_PACKETS 2
L3_MEMB(packets_memb, struct uip_packetqueue_packet, MAX_NUM_QUEUED_PACKETS);

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
static void
packet_timedout(void *ptr)
{
  struct uip_packetqueue_handle *h = ptr;

  PRINTF("uip_packetqueue_free timed out %p\n", h);
  l3_memb_free(&packets_memb, h->packet);
  h->packet = NULL;
}
/*---------------------------------------------------------------------------*/
void
uip_packetqueue_new(struct uip_packetqueue_handle *handle)
{
  PRINTF("uip_packetqueue_new %p\n", handle);
  handle->packet = NULL;
}
/*---------------------------------------------------------------------------*/
struct uip_packetqueue_packet *
uip_packetqueue_alloc(struct uip_packetqueue_handle *handle, clock_time_t lifetime)
{
  PRINTF("uip_packetqueue_alloc %p\n", handle);
  if(handle->packet != NULL) {
    PRINTF("alloced\n");
    return NULL;
  }
  handle->packet = l3_memb_alloc(&packets_memb);
  if(handle->packet != NULL) {
    l3_ctimer_set(&handle->packet->lifetimer, lifetime,
               packet_timedout, handle);
  } else {
    PRINTF("uip_packetqueue_alloc failed\n");
  }
  return handle->packet;
}
/*---------------------------------------------------------------------------*/
void
uip_packetqueue_free(struct uip_packetqueue_handle *handle)
{
  PRINTF("uip_packetqueue_free %p\n", handle);
  if(handle->packet != NULL) {
    l3_ctimer_stop(&handle->packet->lifetimer);
    l3_memb_free(&packets_memb, handle->packet);
    handle->packet = NULL;
  }
}
/*---------------------------------------------------------------------------*/
uint8_t *
uip_packetqueue_buf(struct uip_packetqueue_handle *h)
{
  return h->packet != NULL? h->packet->queue_buf: NULL;
}
/*---------------------------------------------------------------------------*/
uint16_t
uip_packetqueue_buflen(struct uip_packetqueue_handle *h)
{
  return h->packet != NULL? h->packet->queue_buf_len: 0;
}
/*---------------------------------------------------------------------------*/
void
uip_packetqueue_set_buflen(struct uip_packetqueue_handle *h, uint16_t len)
{
  if(h->packet != NULL) {
    h->packet->queue_buf_len = len;
  }
}
/*---------------------------------------------------------------------------*/
