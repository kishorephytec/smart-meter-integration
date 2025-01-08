/*
 * Copyright (c) 2015, SICS Swedish ICT.
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
 */

/**
 * \file
 *         IEEE 802.15.4e Information Element (IE) creation and parsing.
 * \author
 *         Simon Duquennoy <simonduq@sics.se>
 */

#include "StackMACConf.h"
#include <string.h>
#include "common.h"
#include "queue_latest.h"
#include "list_latest.h"
#include "hw_tmr.h"
#include "sw_timer.h"
#include "phy.h"
#include "mac_config.h"
#include "mac.h"
#include "mac_defs_sec.h"
#include "ie_element_info.h"
#include "mac_defs.h"
#include "mac_pib.h"
#include "sm.h"
//#include "fec.h"
#include "ie_manager.h"
#include "fan_mac_interface.h"
#include "timer_service.h"
#include "ccasm.h"
#include "trxsm.h"
#include "mac_frame_parse.h"
#include "fan_config_param.h"
#include "mac_nbr_manager.h"
#include "fan_mac_nbr_info.h"
   


//#define DEBUG DEBUG_NONE
//#include "net/net-debug.h"
   
#ifdef WISUN_FAN_MAC
int wisun_parse_payload_short_ie(const uint8_t *buf, int len,
                                 uint8_t sub_id, struct ieee802154_ies *ies);
int wisun_parse_payload_long_ie(const uint8_t *buf, int len,
                                 uint8_t sub_id, struct ieee802154_ies *ies);
int wisun_parse_mpx_ie (const uint8_t *buf, int len,
                                uint8_t sub_id, struct ieee802154_ies *ies);

int wisun_parse_header_ie(const uint8_t *buf, int len, 
                          struct ieee802154_ies *ies);
#endif

/* c.f. IEEE 802.15.4e Table 4b */
enum ieee802154e_header_ie_id {
  HEADER_IE_LE_CSL = 0x1a,
  HEADER_IE_LE_RIT,
  HEADER_IE_DSME_PAN_DESCRIPTOR,
  HEADER_IE_RZ_TIME,
  HEADER_IE_ACK_NACK_TIME_CORRECTION,
  HEADER_IE_GACK,
  HEADER_IE_LOW_LATENCY_NETWORK_INFO,
#ifdef WISUN_FAN_MAC
  HEADER_WISUN_IE = 0x2a,
#endif
  HEADER_IE_LIST_TERMINATION_1 = 0x7e,
  HEADER_IE_LIST_TERMINATION_2 = 0x7f,
};

/* c.f. IEEE 802.15.4e Table 4c */
enum ieee802154e_payload_ie_id {
  PAYLOAD_IE_ESDU = 0,
  PAYLOAD_IE_MLME,
#ifdef WISUN_FAN_MAC
  WISUN_MPX_IE = 0x03,
  PAYLOAD_WISUN_IE = 0x04,
#endif
  PAYLOAD_IE_LIST_TERMINATION = 0xf,
};

/* c.f. IEEE 802.15.4e Table 4d */
enum ieee802154e_mlme_short_subie_id {
  MLME_SHORT_IE_TSCH_SYNCHRONIZATION = 0x1a,
  MLME_SHORT_IE_TSCH_SLOFTRAME_AND_LINK,
  MLME_SHORT_IE_TSCH_TIMESLOT,
  MLME_SHORT_IE_TSCH_HOPPING_TIMING,
  MLME_SHORT_IE_TSCH_EB_FILTER,
  MLME_SHORT_IE_TSCH_MAC_METRICS_1,
  MLME_SHORT_IE_TSCH_MAC_METRICS_2,
};

/* c.f. IEEE 802.15.4e Table 4e */
enum ieee802154e_mlme_long_subie_id {
  MLME_LONG_IE_TSCH_CHANNEL_HOPPING_SEQUENCE = 0x9,
};

#define WRITE16(buf, val) \
  do { ((uint8_t *)(buf))[0] = (val) & 0xff; \
       ((uint8_t *)(buf))[1] = ((val) >> 8) & 0xff; } while(0);

#define READ16(buf, var) \
  (var) = ((uint8_t *)(buf))[0] | ((uint8_t *)(buf))[1] << 8

/* Create a header IE 2-byte descriptor */
static void
create_header_ie_descriptor(uint8_t *buf, uint8_t element_id, int ie_len)
{
  uint16_t ie_desc;
  /* Header IE descriptor: b0-6: len, b7-14: element id:, b15: type: 0 */
  ie_desc = (ie_len & 0x7f) + ((element_id & 0xff) << 7);
  WRITE16(buf, ie_desc);
}

/* Create a payload IE 2-byte descriptor */
static void
create_payload_ie_descriptor(uint8_t *buf, uint8_t group_id, int ie_len)
{
  uint16_t ie_desc;
  /* MLME Long IE descriptor: b0-10: len, b11-14: group id:, b15: type: 1 */
  ie_desc = (ie_len & 0x07ff) + ((group_id & 0x0f) << 11) + (1 << 15);
  WRITE16(buf, ie_desc);
}

/* Create a MLME short IE 2-byte descriptor */
static void
create_mlme_short_ie_descriptor(uint8_t *buf, uint8_t sub_id, int ie_len)
{
  uint16_t ie_desc;
  /* MLME Short IE descriptor: b0-7: len, b8-14: sub id:, b15: type: 0 */
  ie_desc = (ie_len & 0xff) + ((sub_id & 0x7f) << 8);
  WRITE16(buf, ie_desc);
}

/* Create a MLME long IE 2-byte descriptor */
static void
create_mlme_long_ie_descriptor(uint8_t *buf, uint8_t sub_id, int ie_len)
{
  uint16_t ie_desc;
  /* MLME Long IE descriptor: b0-10: len, b11-14: sub id:, b15: type: 1 */
  ie_desc = (ie_len & 0x07ff) + ((sub_id & 0x0f) << 11) + (1 << 15);
  WRITE16(buf, ie_desc);
}

/* Header IE. ACK/NACK time correction. Used in enhanced ACKs */
int
frame80215e_create_ie_header_ack_nack_time_correction(uint8_t *buf, int len,
    struct ieee802154_ies *ies)
{
  int ie_len = 2;
  if(len >= 2 + ie_len && ies != NULL) {
    int16_t drift_us;
    uint16_t time_sync_field;
    drift_us = ies->ie_time_correction;
    time_sync_field = drift_us & 0x0fff;
    if(ies->ie_is_nack) {
      time_sync_field |= 0x8000;
    }
    WRITE16(buf+2, time_sync_field);
    create_header_ie_descriptor(buf, HEADER_IE_ACK_NACK_TIME_CORRECTION, ie_len);
    return 2 + ie_len;
  } else {
    return -1;
  }
}

/* Header IE. List termination 1 (Signals the end of the Header IEs when
 * followed by payload IEs) */
int
frame80215e_create_ie_header_list_termination_1(uint8_t *buf, int len,
    struct ieee802154_ies *ies)
{
  int ie_len = 0;
  if(len >= 2 + ie_len && ies != NULL) {
    create_header_ie_descriptor(buf, HEADER_IE_LIST_TERMINATION_1, 0);
    return 2 + ie_len;
  } else {
    return -1;
  }
}

/* Header IE. List termination 2 (Signals the end of the Header IEs when
 * followed by an unformatted payload) */
int
frame80215e_create_ie_header_list_termination_2(uint8_t *buf, int len,
    struct ieee802154_ies *ies)
{
  int ie_len = 0;
  if(len >= 2 + ie_len && ies != NULL) {
    create_header_ie_descriptor(buf, HEADER_IE_LIST_TERMINATION_2, 0);
    return 2 + ie_len;
  } else {
    return -1;
  }
}

/* Payload IE. List termination */
int
frame80215e_create_ie_payload_list_termination(uint8_t *buf, int len,
    struct ieee802154_ies *ies)
{
  int ie_len = 0;
  if(len >= 2 + ie_len && ies != NULL) {
    create_payload_ie_descriptor(buf, PAYLOAD_IE_LIST_TERMINATION, 0);
    return 2 + ie_len;
  } else {
    return -1;
  }
}

/* Payload IE. MLME. Used to nest sub-IEs */
int
frame80215e_create_ie_mlme(uint8_t *buf, int len,
    struct ieee802154_ies *ies)
{
  int ie_len = 0;
  if(len >= 2 + ie_len && ies != NULL) {
    /* The length of the outer MLME IE is the total length of sub-IEs */
    create_payload_ie_descriptor(buf, PAYLOAD_IE_MLME, ies->ie_mlme_len);
    return 2 + ie_len;
  } else {
    return -1;
  }
}

/* MLME sub-IE. TSCH synchronization. Used in EBs: ASN and join priority */
int
frame80215e_create_ie_tsch_synchronization(uint8_t *buf, int len,
    struct ieee802154_ies *ies)
{
  int ie_len = 6;
  if(len >= 2 + ie_len && ies != NULL) {
    buf[2] = ies->ie_asn.ls4b;
    buf[3] = ies->ie_asn.ls4b >> 8;
    buf[4] = ies->ie_asn.ls4b >> 16;
    buf[5] = ies->ie_asn.ls4b >> 24;
    buf[6] = ies->ie_asn.ms1b;
    buf[7] = ies->ie_join_priority;
    create_mlme_short_ie_descriptor(buf, MLME_SHORT_IE_TSCH_SYNCHRONIZATION, ie_len);
    return 2 + ie_len;
  } else {
    return -1;
  }
}

/* MLME sub-IE. TSCH slotframe and link. Used in EBs: initial schedule */
int
frame80215e_create_ie_tsch_slotframe_and_link(uint8_t *buf, int len,
    struct ieee802154_ies *ies)
{
  if(ies != NULL) {
    int i;
    int num_slotframes = ies->ie_tsch_slotframe_and_link.num_slotframes;
    int num_links = ies->ie_tsch_slotframe_and_link.num_links;
    int ie_len = 1 + num_slotframes * (4 + 5 * num_links);
    if(num_slotframes > 1 || num_links > FRAME802154E_IE_MAX_LINKS
       || len < 2 + ie_len) {
      /* We support only 0 or 1 slotframe in this IE and a predefined maximum number of links */
      return -1;
    }
    /* Insert IE */
    buf[2] = num_slotframes;
    /* Insert slotframe */
    if(num_slotframes == 1) {
      buf[2 + 1] = ies->ie_tsch_slotframe_and_link.slotframe_handle;
      WRITE16(buf + 2 + 2, ies->ie_tsch_slotframe_and_link.slotframe_size);
      buf[2 + 4] = num_links;
      /* Loop over links */
      for(i = 0; i < num_links; i++) {
        /* Insert links */
        WRITE16(buf + 2 + 5 + i * 5, ies->ie_tsch_slotframe_and_link.links[i].timeslot);
        WRITE16(buf + 2 + 5 + i * 5 + 2, ies->ie_tsch_slotframe_and_link.links[i].channel_offset);
        buf[2 + 5 + i * 5 + 4] = ies->ie_tsch_slotframe_and_link.links[i].link_options;
      }
    }
    create_mlme_short_ie_descriptor(buf, MLME_SHORT_IE_TSCH_SLOFTRAME_AND_LINK, ie_len);
    return 2 + ie_len;
  } else {
    return -1;
  }
}

/* MLME sub-IE. TSCH timeslot. Used in EBs: timeslot template (timing) */
int
frame80215e_create_ie_tsch_timeslot(uint8_t *buf, int len,
    struct ieee802154_ies *ies)
{
  int ie_len;
  if(ies == NULL) {
    return -1;
  }
  /* Only ID if ID == 0, else full timing description */
  ie_len = ies->ie_tsch_timeslot_id == 0 ? 1 : 25;
  if(len >= 2 + ie_len) {
    buf[2] = ies->ie_tsch_timeslot_id;
    if(ies->ie_tsch_timeslot_id != 0) {
      int i;
      for(i = 0; i < tsch_ts_elements_count; i++) {
        WRITE16(buf + 3 + 2 * i, ies->ie_tsch_timeslot[i]);
      }
    }
    create_mlme_short_ie_descriptor(buf, MLME_SHORT_IE_TSCH_TIMESLOT, ie_len);
    return 2 + ie_len;
  } else {
    return -1;
  }
}

/* MLME sub-IE. TSCH channel hopping sequence. Used in EBs: hopping sequence */
int
frame80215e_create_ie_tsch_channel_hopping_sequence(uint8_t *buf, int len,
    struct ieee802154_ies *ies)
{
  int ie_len;
  if(ies == NULL || ies->ie_hopping_sequence_len > sizeof(ies->ie_hopping_sequence_list)) {
    return -1;
  }
  ie_len = ies->ie_channel_hopping_sequence_id == 0 ? 1 : 12 + ies->ie_hopping_sequence_len;
  if(len >= 2 + ie_len && ies != NULL) {
    buf[2] = ies->ie_channel_hopping_sequence_id;
    buf[3] = 0; /* channel page */
    WRITE16(buf + 4, 0); /* number of channels */
    WRITE16(buf + 6, 0); /* phy configuration */
    WRITE16(buf + 8, 0);
    /* Extended bitmap. Size: 0 */
    WRITE16(buf + 10, ies->ie_hopping_sequence_len); /* sequence len */
    memcpy(buf + 12, ies->ie_hopping_sequence_list, ies->ie_hopping_sequence_len); /* sequence list */
    WRITE16(buf + 12 + ies->ie_hopping_sequence_len, 0); /* current hop */
    create_mlme_long_ie_descriptor(buf, MLME_LONG_IE_TSCH_CHANNEL_HOPPING_SEQUENCE, ie_len);
    return 2 + ie_len;
  } else {
    return -1;
  }
}

/* Parse a header IE */
static int
frame802154e_parse_header_ie(const uint8_t *buf, int len,
    uint8_t element_id, struct ieee802154_ies *ies)
{
  switch(element_id) {
    case HEADER_IE_ACK_NACK_TIME_CORRECTION:
      if(len == 2) {
        if(ies != NULL) {
          /* If the originator was a time source neighbor, the receiver adjust
           * its own clock by incorporating the received drift correction */
          uint16_t time_sync_field = 0;
          int16_t drift_us = 0;
          /* Extract drift correction from Sync-IE, cast from 12 to 16-bit,
           * and convert it to RTIMER ticks.
           * See page 88 in IEEE Std 802.15.4e-2012. */
          READ16(buf, time_sync_field);
          /* First extract NACK */
          ies->ie_is_nack = (time_sync_field & (uint16_t)0x8000) ? 1 : 0;
          /* Then cast from 12 to 16 bit signed */
          if(time_sync_field & 0x0800) { /* Negative integer */
            drift_us = time_sync_field | 0xf000;
          } else { /* Positive integer */
            drift_us = time_sync_field & 0x0fff;
          }
          /* Convert to RTIMER ticks */
          ies->ie_time_correction = drift_us;
        }
        return len;
      }
      break;
#ifdef WISUN_FAN_MAC
    case HEADER_WISUN_IE:
      {
        int status;
        status = wisun_parse_header_ie (buf, len, ies);
        return status;
      }
      /*Debdeep added this line for 802.15.4 IE*/
  default:
    return 0;
#endif
  }
  return -1;
}

/* Parse a MLME short IE */
static int
frame802154e_parse_mlme_short_ie(const uint8_t *buf, int len,
    uint8_t sub_id, struct ieee802154_ies *ies)
{
  switch(sub_id) {
    case MLME_SHORT_IE_TSCH_SLOFTRAME_AND_LINK:
      if(len >= 1) {
        int i;
        int num_slotframes = buf[0];
        int num_links = buf[4];
        if(num_slotframes == 0) {
          return len;
        }
        if(num_slotframes <= 1 && num_links <= FRAME802154E_IE_MAX_LINKS
            && len == 1 + num_slotframes * (4 + 5 * num_links)) {
          if(ies != NULL) {
            /* We support only 0 or 1 slotframe in this IE and a predefined maximum number of links */
            ies->ie_tsch_slotframe_and_link.num_slotframes = buf[0];
            ies->ie_tsch_slotframe_and_link.slotframe_handle = buf[1];
            READ16(buf + 2, ies->ie_tsch_slotframe_and_link.slotframe_size);
            ies->ie_tsch_slotframe_and_link.num_links = buf[4];
            for(i = 0; i < num_links; i++) {
              READ16(buf + 5 + i * 5, ies->ie_tsch_slotframe_and_link.links[i].timeslot);
              READ16(buf + 5 + i * 5 + 2, ies->ie_tsch_slotframe_and_link.links[i].channel_offset);
              ies->ie_tsch_slotframe_and_link.links[i].link_options = buf[5 + i * 5 + 4];
            }
          }
          return len;
        }
      }
      break;
    case MLME_SHORT_IE_TSCH_SYNCHRONIZATION:
      if(len == 6) {
        if(ies != NULL) {
          ies->ie_asn.ls4b = (uint32_t)buf[0];
          ies->ie_asn.ls4b |= (uint32_t)buf[1] << 8;
          ies->ie_asn.ls4b |= (uint32_t)buf[2] << 16;
          ies->ie_asn.ls4b |= (uint32_t)buf[3] << 24;
          ies->ie_asn.ms1b = (uint8_t)buf[4];
          ies->ie_join_priority = (uint8_t)buf[5];
        }
        return len;
      }
      break;
    case MLME_SHORT_IE_TSCH_TIMESLOT:
      if(len == 1 || len == 25) {
        if(ies != NULL) {
          ies->ie_tsch_timeslot_id = buf[0];
          if(len == 25) {
            int i;
            for(i = 0; i < tsch_ts_elements_count; i++) {
              READ16(buf+1+2*i, ies->ie_tsch_timeslot[i]);
            }
          }
        }
        return len;
      }
      break;
  }
  return -1;
}

/* Parse a MLME long IE */
static int
frame802154e_parse_mlme_long_ie(const uint8_t *buf, int len,
    uint8_t sub_id, struct ieee802154_ies *ies)
{
  switch(sub_id) {
    case MLME_LONG_IE_TSCH_CHANNEL_HOPPING_SEQUENCE:
      if(len > 0) {
        if(ies != NULL) {
          ies->ie_channel_hopping_sequence_id = buf[0];
          if(len > 1) {
            READ16(buf+8, ies->ie_hopping_sequence_len); /* sequence len */
            if(ies->ie_hopping_sequence_len <= sizeof(ies->ie_hopping_sequence_list)
                && len == 12 + ies->ie_hopping_sequence_len) {
              memcpy(ies->ie_hopping_sequence_list, buf+10, ies->ie_hopping_sequence_len); /* sequence list */
            }
          }
        }
        return len;
      }
      break;
  }
  return -1;
}

/* Parse all IEEE 802.15.4e Information Elements (IE) from a frame */
int
frame802154e_parse_information_elements(const uint8_t *buf, uint16_t buf_size,
    struct ieee802154_ies *ies)
{
  const uint8_t *start = buf;
  uint16_t ie_desc;
  uint8_t type;
  uint8_t id;
  uint16_t len = 0;
  int nested_mlme_len = 0;
  enum {
    PARSING_HEADER_IE, 
    PARSING_PAYLOAD_IE, 
    PARSING_MLME_SUBIE,
#ifdef WISUN_FAN_MAC
    PARSING_WISUN_PAYLOAD_SUBIE,
#endif
  } parsing_state;

  if(ies == NULL) {
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
    stack_print_debug ("### IEs NULL\n");
#endif
    return -1;
  }

  /* Always look for a header IE first (at least "list termination 1") */
  parsing_state = PARSING_HEADER_IE;
  ies->ie_payload_ie_offset = 0;

  /* Loop over all IEs */
  while(buf_size > 0) {
    if(buf_size < 2) { /* Not enough space for IE descriptor */
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
      stack_print_debug ("### Not enough space for IE descriptor [%d]\n", buf_size);
#endif
      return -1;
    }
    READ16(buf, ie_desc);
    buf_size -= 2;
    buf += 2;
    type = ie_desc & 0x8000 ? 1 : 0; /* b15 */

    switch(parsing_state) {
      case PARSING_HEADER_IE:
        if(type != 0) {
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
          stack_print_debug ("### hdr type error [%d]\n", type);
#endif
          return -1;
        }
        /* Header IE: 2 bytes descriptor, c.f. fig 48n in IEEE 802.15.4e */
        len = ie_desc & 0x007f; /* b0-b6 */
        id = (ie_desc & 0x7f80) >> 7; /* b7-b14 */
        switch(id) {
          case HEADER_IE_LIST_TERMINATION_1:
            if(len == 0) {
              /* End of payload IE list, now expect payload IEs */
              parsing_state = PARSING_PAYLOAD_IE;
              ies->ie_payload_ie_offset = buf - start; /* Save IE header len */
            } else {
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
              stack_print_debug ("### len after header TIE 1 error [%d]\n", len);
#endif
              return -1;
            }
            break;
          case HEADER_IE_LIST_TERMINATION_2:
            /* End of IE parsing */
            if(len == 0) {
              ies->ie_payload_ie_offset = buf - start; /* Save IE header len */
              return buf + len - start;
            } else {
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
              stack_print_debug ("### len after header TIE 2 error [%d]\n", len);
#endif
              return -1;
            }          
          default:
            if(len > buf_size || frame802154e_parse_header_ie(buf, len, id, ies) == -1) {
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
              stack_print_debug ("### HDR IE parsing error\n");
#endif
              return -1;
            }
            break;
        }
        break;
      case PARSING_PAYLOAD_IE:
        if(type != 1) {
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
          stack_print_debug ("### pld type error [%d]\n", type);
#endif
          return -1;
        }
        /* Payload IE: 2 bytes descriptor, c.f. fig 48o in IEEE 802.15.4e */
        len = ie_desc & 0x7ff; /* b0-b10 */
        id = (ie_desc & 0x7800) >> 11; /* b11-b14 */
        switch(id) {
          case PAYLOAD_IE_MLME:
            /* Now expect 'len' bytes of MLME sub-IEs */
            parsing_state = PARSING_MLME_SUBIE;
            nested_mlme_len = len;
            len = 0; /* Reset len as we want to read subIEs and not jump over them */
            break;     
#ifdef WISUN_FAN_MAC
          case WISUN_MPX_IE:
            if(wisun_parse_mpx_ie(buf, len, id, ies) == -1) {
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
              stack_print_debug ("### mpx IE parsing error\n");
#endif
              return -1;
          }
            break;
          case PAYLOAD_WISUN_IE:
            /* Now expect 'len' bytes of MLME sub-IEs */
            parsing_state = PARSING_WISUN_PAYLOAD_SUBIE;
            nested_mlme_len = len;
            len = 0; /* Reset len as we want to read subIEs and not jump over them */
            break; 
#endif 
          case PAYLOAD_IE_LIST_TERMINATION:
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
            if (len != 0)
              stack_print_debug ("### pld TIE error [%d]\n", len);
#endif
            return (len == 0) ? buf + len - start : -1;
          default:
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
            stack_print_debug ("### pld ID error [%d]\n", id);
#endif
            return -1;
        }
        break;
      case PARSING_MLME_SUBIE:
        /* MLME sub-IE: 2 bytes descriptor, c.f. fig 48q in IEEE 802.15.4e */
        /* type == 0 means short sub-IE, type == 1 means long sub-IE */
        if(type == 0) {
          /* Short sub-IE, c.f. fig 48r in IEEE 802.15.4e */
          len = ie_desc & 0x00ff; /* b0-b7 */
          id = (ie_desc & 0x7f00) >> 8; /* b8-b14 */
          if(len > buf_size || frame802154e_parse_mlme_short_ie(buf, len, id, ies) == -1) {
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
            stack_print_debug ("### mlme short subIE parsing error\n");
#endif
            return -1;
          }
        } else {
          /* Long sub-IE, c.f. fig 48s in IEEE 802.15.4e */
          len = ie_desc & 0x7ff; /* b0-b10 */
          id = (ie_desc & 0x7800) >> 11; /* b11-b14 */
          if(len > buf_size || frame802154e_parse_mlme_long_ie(buf, len, id, ies) == -1) {
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
            stack_print_debug ("### mlme long subIE parsing error\n");
#endif
            return -1;
          }
        }
#ifdef WISUN_FAN_MAC
        case PARSING_WISUN_PAYLOAD_SUBIE:
        /* MLME sub-IE: 2 bytes descriptor, c.f. fig 48q in IEEE 802.15.4e */
        /* type == 0 means short sub-IE, type == 1 means long sub-IE */
        if(type == 0) {
          /* Short sub-IE, c.f. fig 48r in IEEE 802.15.4e */
          len = ie_desc & 0x00ff; /* b0-b7 */
          id = (ie_desc & 0x7f00) >> 8; /* b8-b14 */
          if(len > buf_size || wisun_parse_payload_short_ie(buf, len, id, ies) == -1) {
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
            stack_print_debug ("### wisun short subIE parsing error\n");
#endif
            return -1;
          }
        } else {
          /* Long sub-IE, c.f. fig 48s in IEEE 802.15.4e */
          len = ie_desc & 0x7ff; /* b0-b10 */
          id = (ie_desc & 0x7800) >> 11; /* b11-b14 */
          if(len > buf_size || wisun_parse_payload_long_ie(buf, len, id, ies) == -1) {
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
            stack_print_debug ("### wisun long subIE parsing error\n");
#endif
            return -1;
          }
        }
#endif
        /* Update remaining nested MLME len */
        nested_mlme_len -= 2 + len;
        if(nested_mlme_len < 0) {
          /* We found more sub-IEs than initially advertised */
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
          stack_print_debug ("### more sub-IEs than initially advertised\n");
#endif
          return -1;
        }
        if(nested_mlme_len == 0) {
          /* End of IE parsing, look for another payload IE */
          parsing_state = PARSING_PAYLOAD_IE;
        }
        break;
    }
    buf += len;
    buf_size -= len;
  }

  if(parsing_state == PARSING_HEADER_IE) {
    ies->ie_payload_ie_offset = buf - start; /* Save IE header len */
  }

  return buf - start;
}


/*
** ============================================================================
** Private Macro definitions
** ============================================================================
*/
#define HEADER_IE_TYPE      0
#define PAYLOAD_IE_TYPE     1

/*
** ============================================================================
** Private Structures, Unions & enums Type Definitions
** ============================================================================
*/

/* None */

/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/

/*

1. Wi-SUN Header IEs (WH-IEs). MAC management information carried in the frame header. 

WH-IE Identifier value (0x2A). 
The Type field MUST be set to 0 (short).

WH-IE Sub ID 
0x01    UTT-IE
0x02    BT-IE
0x03    FC-IE
0x04    RSL-IE
0x05    MHDS-IE
0x06    VH-IE
0x07    EA-IE

2. Wi-SUN Payload IEs (WP-IE). MAC management information carried in the frame payload

A WP-IE MUST adhere to the format of the MLME Information Element described in 7.4.3.2 of [IEEE802.15.4]. 
The Group ID field MUST be set to the WP-IE Identifier value (0x04). 
The Type field MUST be set to 1 (Long).
A WP IE MAY only contain one or more Nested-IEs .

Sub-IE Sub-ID   Sub-IE Type     Name
0x01            1 (long)        US-IE
0x02            1 (long)        BS-IE
0x03            1 (long)        VP-IE
0x04            0 (short)       PAN-IE
0x05            0 (short)       NETNAME-IE
0x06            0 (short)       PANVER-IE       
0x07            0 (short)       GTKHASH-IE

3. MPX Information Element (MPX-IE)
The Group ID field MUST be set to (0x03). 
The Type field MUST be set to 1 (Long).


*/


/*
** ============================================================================
** Public Variable Definitions
** ============================================================================
*/


/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/
#if (CFG_MAC_STARTSM_ENABLED == 1)     
extern startsm_t *startsm_p;
#endif
extern self_info_fan_mac_t mac_self_fan_info;
extern int32_t rcvd_siganl_strength;
extern uint8_t authnt_interfac_id [8];
extern fan_mac_param_t fan_mac_params;
extern trxsm_t *trxsm_p;
extern uint8_t get_node_type( void );
/*
** ============================================================================
** Public Function Prototypes
** ============================================================================
*/

#if(FAN_EAPOL_FEATURE_ENABLED == 1)
#if (APP_LBR_ROUTER )
extern parent_child_info_tag eapol_parent_child_info;
#endif
extern void fan_eapol_request (void);
void update_pan_version_from_eapol_parent (void);
#endif



mac_nbr_descriptor_t* get_nbr_desc_from_addr (uint8_t* p_nbr_addr);
extern mac_nbr_descriptor_t* add_device_to_nbr_desc(mac_rx_t *mrp);
extern void get_self_extended_address_reverse(uint8_t *self_mac_addr);
extern uint32_t  mac_cb_update_ufsi( uint32_t proc_delay );
extern void make_nbr_valid_channel_list(mac_nbr_descriptor_t* nbr_desc);
void mark_change_in_gtkl (uint8_t index);
//void gtk_need_to_update (void);
uint8_t is_gtk_lifetime_expired (uint8_t index);
//void uip_ds6_nbr_update_rsl_ie(uint8_t* rcvd_addr, uint8_t* self_addr, int32_t rssi);
void calu_pan_cost_recv_pan();
void update_self_pan_version (uint16_t src_pan_version);
int get_join_state (void);
uint8_t get_node_type( void );
void reset_gtk_buf_in_supp_cred (uint8_t indx);

void trickle_timer_inconsistency_pc(void);
void trickle_timer_consistency_pc(void);

extern uint8_t get_LQI_from_RSSI( int8_t rssi_val );
uint8_t is_security_key_index_changed (uint8_t nbr_key_index);
extern void responce_delay(uint8_t responce_delay_time,void *pkt_ptr);
extern uint8_t dont_change_ubcast_channel;

#if(FAN_EDFE_FEATURE_ENABLED == 1)
extern edfe_info_t edfe_information; 
extern void enable_disable_edfe_frame(uint8_t value,uint8_t edfe_frame_type);
void stop_edfe_transmit_timer();
#endif


/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/
static uchar extract_header_ie( uchar *content, int content_len, mac_rx_t *mrp, 
                        mac_nbr_descriptor_t* p_nbr_desc);
static uchar extract_payload_short_ie(uint8_t sub_id, uint8_t *p_content_location ,
                               mac_rx_t *mrp , uint16_t sub_ie_contlen, 
                               mac_nbr_descriptor_t* p_nbr_desc_temp);
static uchar extract_payload_long_ie(uint8_t sub_id, uint8_t *p_content_location ,
                         mac_rx_t *mrp , uint16_t sub_ie_contlen, 
                         mac_nbr_descriptor_t* nbr_desc);
static int fan_mac_nbr_update_ies (mac_nbr_descriptor_t* nbr_table_instance, 
                             mac_nbr_descriptor_t* src_instance, 
                             uint32_t hdr_bitmap, uint32_t pld_bitmap);
static uint8_t device_should_be_added (uint8_t frame_type);
static uint32_t get_ie_mask_from_id (uint8_t subIE, uint8_t type);
static bool is_subIE_supported (mac_rx_t *mrp, uint8_t subIE, uint8_t type);
static ushort create_sub_ie_desc( uchar type, uchar sub_ie_id,ushort content_len);

static ushort create_ie_desc(uchar type, uchar ie_id, ushort content_len);
void reset_incoming_frame_counter_for_stale_key (uint8_t stale_key_index);

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

/******************************************************************************/
/*
Raka :: 21-May-2017

Header IE:
*******************************************************************
// Parse IE 2 Byte header Flags. 
ie_length			  = ie_2_byte_header & 0x7F;
ie_element_id	      = (ie_2_byte_header & 0x7F80) >> 7;
ie_type			      = (ie_2_byte_header & 0x8000) >> 15;

Payloead IE:
**********************************************************
// Parse IE 2 Byte header Flags.
payload_ie_length	  = ie_2_byte_header & 0x07FF;
ie_element_id	      = (ie_2_byte_header & 0x7800) >> 11;
ie_type			      = (ie_2_byte_header & 0x8000) >> 15;


Sub ID's
***************************************************************
ie_sub_header_type = (ie_sub_header_byte & 0x8000) >> 15;

if (ie_sub_header_type == 1)
{
  ie_subdescriptor_length	  = ie_sub_header_byte & 0x07FF;
  ie_subdescriptor_sub_id	  = (ie_sub_header_byte & 0x7800) >> 11;

}
else
{
  
  ie_subdescriptor_length	  = ie_sub_header_byte & 0x00FF;
  ie_subdescriptor_sub_id	  = (ie_sub_header_byte & 0x7F00) >> 8;

}
*/
#ifdef WISUN_FAN_MAC
parse_result_t mac_frame_find_hdr_ie_offset (mac_rx_t *mrp, uint8_t *data)
{
  uint16_t header_ie_offset = mrp->header_ie_offset;
  uint16_t total_length = mrp->payload_length;
  uint8_t *data_p = data;
  uint16_t ie_desc = 0xFFFF;
  uint16_t content_len;
  uint8_t subIE;
  
  if (header_ie_offset >= total_length)
    return PARSE_DISCARD;
  
  while (1)
  {
    ie_desc = get_ushort(data_p);
    
    if (((ie_desc & HEADER_IE_ID_MASK) >> HDR_IE_LENGTH_FLD_LEN_IN_BITS)
        == WH_IE)
    {
      subIE = *(data_p + IE_DISCRIPTOR_LEN);
      if (is_subIE_supported (mrp, subIE, HEADER_IE_TYPE) == FALSE)
        return PARSE_DISCARD;
      
      if (subIE == WH_IE_SUBID_UTT_IE_SHORT)
        mrp->recived_frame_type = data_p[3];

      if (!device_should_be_added (mrp->recived_frame_type))
        return PARSE_DISCARD;
      
      content_len = ( ie_desc & HEADER_IE_CONTENT_LEN_MASK );
      data_p += (IE_DISCRIPTOR_LEN + content_len);
      header_ie_offset += (IE_DISCRIPTOR_LEN + content_len);
      
      if (header_ie_offset > total_length)
        return PARSE_DISCARD;
      
      if (header_ie_offset == total_length)
        break;
    }
    else if (((ie_desc & HEADER_IE_ID_MASK) >> HDR_IE_LENGTH_FLD_LEN_IN_BITS) 
        == HIE_TERMINATION_IE1)
    {
      mrp->pld_ies_present = 1;
      data_p += IE_DISCRIPTOR_LEN;
      break;
    }
    else if (((ie_desc & HEADER_IE_ID_MASK) >> HDR_IE_LENGTH_FLD_LEN_IN_BITS) 
        == HIE_TERMINATION_IE2)
    {
      data_p += IE_DISCRIPTOR_LEN;
      break;
    }
    else
    {
      //      return PARSE_DISCARD;
      content_len = ( ie_desc & HEADER_IE_CONTENT_LEN_MASK );
      data_p += (IE_DISCRIPTOR_LEN + content_len);
      header_ie_offset += (IE_DISCRIPTOR_LEN + content_len);
      
      if (header_ie_offset > total_length)
        return PARSE_DISCARD;
      
      if (header_ie_offset == total_length)
        break;
    }
  }
  
  mrp->headerIEFieldLen = (data_p - data);
  mrp->headerIEList = data;
  return PARSE_STORED;
}

uint8_t is_valid_btie_bsie_received (mac_rx_t *mrp)
{  
  if((mrp->recived_frame_type == PAN_ADVERT_FRAME) ||
       (mrp->recived_frame_type == PAN_ADVERT_SOLICIT) ||
         (mrp->recived_frame_type == PAN_CONFIG_SOLICIT))
    return 0;   /*Failure*/
  
  if ((mrp->wisun_fan_ies->hdr_subIE_bitmap & BT_IE_MASK) &&
      (mrp->wisun_fan_ies->pld_subIE_bitmap & BS_IE_MASK))
    return 1;   /*Success*/
  else
    return 0;   /*Failure*/
}

uint8_t is_valid_fcie_paresent(mac_rx_t *mrp)
{
  if(mrp->recived_frame_type != FAN_DATA_PKT)
    return 0;   /*Failure*/
   
  if (mrp->wisun_fan_ies->hdr_subIE_bitmap & FC_IE_MASK)
    
    return 1;   /*Success*/
  else
    return 0;   /*Failure*/
  
}

uint8_t extract_ies (mac_rx_t *mrp)
{
  mac_nbr_descriptor_t p_nbr_desc_st;    
  mac_nbr_descriptor_t *nbr_table_entry = NULL;
  int status;
  
  memset (&p_nbr_desc_st, 0, sizeof(p_nbr_desc_st));
  
  /*At least UTT-IE should be present*/
  if (mrp->wisun_fan_ies->hdr_subIE_bitmap == 0)
  {
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
    stack_print_debug ("### NO HDR IE\n");
#endif
    return PARSE_DISCARD;
  }
  
  mrp->wisun_fan_ies->mac_nbr_desc = (mac_nbr_descriptor_t*) &p_nbr_desc_st;
  status = frame802154e_parse_information_elements (mrp->headerIEList,
                                           mrp->payload_length +
                                             mrp->headerIEFieldLen,
                                             mrp->wisun_fan_ies);
  if (status == -1)
    return PARSE_DISCARD;
  
  if (!device_should_be_added (mrp->recived_frame_type))
    return PARSE_DISCARD;
  
  nbr_table_entry = add_device_to_nbr_desc (mrp);
  if (nbr_table_entry != NULL)
  {
/* Debdeep :: RSSI value is required to filter out EAPOL parent candidate based on threshold RSSI */
    nbr_table_entry->rssi = mrp->pd_rxp->rssi;
    status = fan_mac_nbr_update_ies (nbr_table_entry, &p_nbr_desc_st,
                            mrp->wisun_fan_ies->hdr_subIE_bitmap, 
                            mrp->wisun_fan_ies->pld_subIE_bitmap);
    
    if(status == -1)
      return PARSE_DISCARD;
    
    if ((mrp->recived_frame_type == PAN_CONFIG) || 
        (mrp->recived_frame_type == FAN_DATA_PKT) ||
          (mrp->recived_frame_type == FAN_ACK))
      nbr_table_entry->ulp_rx_time = mrp->pd_rxp->rx_current_hw_time;
    
#if (APP_LBR_ROUTER == 1)   
  if(!(mrp->pd_rxp->psdu[0] & MAC_ACK_REQUIRED))
    {
#if(FAN_EDFE_FEATURE_ENABLED == 1)
      if(is_valid_fcie_paresent(mrp))
      {
        if(mrp->src.address_mode != 0) 
          mem_rev_cpy(edfe_information.edfe_ini_mac_addr,nbr_table_entry->mac_addr,8);
        if(dont_change_ubcast_channel != 0x88)
        {
          PLME_set_request(phyCurrentChannel,2,&mrp->pd_rxp->channel);
          dont_change_ubcast_channel = 0x88;
        }

      }
#endif
    }
#endif    
    return PARSE_STORED;
  }
  else
  {
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
    stack_print_debug ("### No nbr table entry\n");
#endif
    return PARSE_DISCARD;
  }
}

int wisun_parse_header_ie(const uint8_t *buf, int len, 
                          struct ieee802154_ies *ies)
{
  if (extract_header_ie ((uint8_t *)buf, len, ies->mrp_ctx, 
                         ies->mac_nbr_desc) == 0)
    return 0;
  
  return -1;
}

int wisun_parse_payload_short_ie(const uint8_t *buf, int len,
                                 uint8_t sub_id, struct ieee802154_ies *ies)
{
  if (is_subIE_supported (ies->mrp_ctx, sub_id, PAYLOAD_IE_TYPE) == FALSE)
    return -1;
  
  if (extract_payload_short_ie (sub_id, (uint8_t *)buf, ies->mrp_ctx, len, 
                                ies->mac_nbr_desc) != 0)
    return -1;
    
  return 0;
}

int wisun_parse_payload_long_ie(const uint8_t *buf, int len,
                                uint8_t sub_id, struct ieee802154_ies *ies)
{
  if (is_subIE_supported (ies->mrp_ctx, sub_id, PAYLOAD_IE_TYPE) == FALSE)
    return -1;
  
  if (extract_payload_long_ie (sub_id, (uint8_t *)buf, ies->mrp_ctx, len, 
                                ies->mac_nbr_desc) != 0)
    return -1;
  
  return 0;
}

int wisun_parse_mpx_ie (const uint8_t *buf, int len,
                                uint8_t sub_id, struct ieee802154_ies *ies)
{
  mac_rx_t *mrp = (mac_rx_t *)ies->mrp_ctx;
  
  if ((mrp == NULL) || (buf == NULL))
    return -1;
  
  mrp->payload = (uint8_t *)buf;
  mrp->payload_length = len;
  return 0;
}

/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/

static uint8_t extract_header_ie( uint8_t *content, int content_len, mac_rx_t *mrp, 
                        mac_nbr_descriptor_t* p_nbr_desc)
{
  uint8_t sub_id = *content;
  
  switch(sub_id)
  {
  case WH_IE_SUBID_UTT_IE_SHORT:/*0x01*/
    mrp->recived_frame_type = content[1];
    memcpy(((uint8_t*)&(p_nbr_desc->ufsi)),&content[2],3);
    if (mrp->recived_frame_type != PAN_ADVERT_SOLICIT)
      p_nbr_desc->ut_ie_rx_time = mrp->pd_rxp->rx_current_hw_time;
    break;
    
  case WH_IE_SUBID_BT_IE_SHORT:/*0x02*/ 
    if((mrp->recived_frame_type == PAN_ADVERT_FRAME) ||
       (mrp->recived_frame_type == PAN_ADVERT_SOLICIT) ||
         (mrp->recived_frame_type == PAN_CONFIG_SOLICIT))
      break;    /*Packet is not dropped but IE is not stored*/
    memcpy(((uint8_t*)&(p_nbr_desc->broad_cast_slno)),&content[1],2);
    memcpy(((uint8_t*)&(p_nbr_desc->broad_frac_int_offset)),&content[3],3);
    p_nbr_desc->btie_rcvd_timestamp = mrp->pd_rxp->rx_current_hw_time;
    break; 
    
  case WH_IE_SUBID_FC_IE_SHORT:/*0x03*/
    if(mrp->recived_frame_type == FAN_DATA_PKT)
      p_nbr_desc->recv_fcie.transmit_flow_cont =  content[1];
      p_nbr_desc->recv_fcie.receive_flow_cont =  content[2];
      break;    /*Packet is not dropped but IE is not stored*/
    /*TO-DO*/
    break;
    
  case WH_IE_SUBID_RSL_IE_SHORT:/*0x04*/
    if(mrp->recived_frame_type != FAN_ACK)
      break;    /*Packet is not dropped but IE is not stored*/
    p_nbr_desc->rsl = content[1];           
    break;
    
  case WH_IE_SUBID_MHDS_IE_SHORT:/*0x05*/
    if(mrp->recived_frame_type != PAN_CONFIG)
      break;    /*Packet is not dropped but IE is not stored*/
    /*TO-DO*/
    break;
    
  case WH_IE_SUBID_VH_IE_SHORT:/*0x06*/
    if(mrp->recived_frame_type != FAN_DATA_PKT)
      break;    /*Packet is not dropped but IE is not stored*/
    /*TO-DO*/
    break;                    
  case WH_IE_SUBID_EA_IE_SHORT:

#if(FAN_EAPOL_FEATURE_ENABLED == 1)
    if(mrp->recived_frame_type != EAPOL)
      break;    /*Packet is not dropped but IE is not stored*/
#endif
    mem_rev_cpy((uint8_t *)&p_nbr_desc->eui_infor, &content[1],8);
    if(fan_mac_information_data.state_ind != JOIN_STATE_5)
    {
      memcpy(authnt_interfac_id,(uint8_t *)&p_nbr_desc->eui_infor,8);
    }
    break;  
  default:
    break;
  }
  return 0;
}   

//static uint8_t hash_ie_zero (uint8_t *hash)
//{
//  uint8_t zero_buf[8] = {0};
//  
//  if (memcmp (hash, zero_buf, 8))
//    return 0;
//  else
//    return 1;
//}

static uchar extract_payload_short_ie(uint8_t sub_id, uint8_t *p_content_location,
                                      mac_rx_t *mrp, uint16_t sub_ie_content_len, 
                                      mac_nbr_descriptor_t* nbr_desc)
{
  uint8_t* ie_ptr  = p_content_location;
  
  switch (sub_id)
  {
  case WP_IE_SUBIE_SUBID_PAN_IE_SHORT:
    if (mrp->recived_frame_type == PAN_ADVERT_FRAME)
    {      
      memcpy (&nbr_desc->rev_pan_metrics.pan_size, ie_ptr, 2);
      ie_ptr += 2;
      memcpy (&nbr_desc->rev_pan_metrics.routing_cost, ie_ptr, 2);
      ie_ptr += 2;
      
      nbr_desc->rev_pan_metrics.parent_bs_ie_use = (*ie_ptr)&0x01;
      nbr_desc->rev_pan_metrics.routing_methood = (((*ie_ptr)&0x02)>>1);
      nbr_desc->rev_pan_metrics.fan_tps_version = (((*ie_ptr)&0xE0)>>5);
      
      nbr_desc->rev_pan_metrics.pan_id = mrp->src.pan_id;
      
      if ((nbr_desc->rev_pan_metrics.pan_size != mac_self_fan_info.pan_metrics.pan_size) &&
        (nbr_desc->rev_pan_metrics.routing_cost != mac_self_fan_info.pan_metrics.routing_cost) &&
          (nbr_desc->rev_pan_metrics.parent_bs_ie_use != mac_self_fan_info.pan_metrics.parent_bs_ie_use) &&
            (nbr_desc->rev_pan_metrics.routing_methood != mac_self_fan_info.pan_metrics.routing_methood) &&
              (nbr_desc->rev_pan_metrics.fan_tps_version != mac_self_fan_info.pan_metrics.fan_tps_version))                       
        return 1;       //fail
    }
    break; 
    
  case WP_IE_SUBIE_SUBID_NETNAME_IE_SHORT:
    if ((mrp->recived_frame_type == PAN_ADVERT_FRAME) ||
        (mrp->recived_frame_type == PAN_ADVERT_SOLICIT) ||
          (mrp->recived_frame_type == PAN_CONFIG_SOLICIT))
    {
      memcpy (nbr_desc->net_name, ie_ptr, sub_ie_content_len);
      nbr_desc->net_name_length = sub_ie_content_len;   
      
      if (memcmp (nbr_desc->net_name, mac_self_fan_info.net_name, 
                  nbr_desc->net_name_length))
        return 1;       //fail
    }
    break;
    
  case WP_IE_SUBIE_SUBID_PANVER_IE_SHORT:
    if(mrp->recived_frame_type == PAN_CONFIG)
    {
      memcpy (&nbr_desc->pan_ver, ie_ptr, 2);
      
      /* Debdeep: 16-nov-2018:: In join state 5 we update self pan version 
         after getting new key from eapol parent. Also we update self pan version 
         if any gtk hash is memset to zero.*/
#if (APP_LBR_ROUTER == 1)                  
      if (get_node_type() == 0x01)    //i.e, only for router
      {
#if(FAN_EAPOL_FEATURE_ENABLED == 1)
        if (memcmp (mrp->src.address.ieee_address, eapol_parent_child_info.sle_eapol_parent, 8))
          break;
#endif
        if (mac_self_fan_info.pan_ver != nbr_desc->pan_ver)
        {
          if (get_join_state () != 5)
            update_self_pan_version (nbr_desc->pan_ver);
          
#if (PRINT_DEBUG_LEVEL == CONSOLE_ERROR_DEBUG)
          stack_print_debug ("PAN version changed :: self = %d, rcv = %d\n", mac_self_fan_info.pan_ver, nbr_desc->pan_ver);
#endif          
          
          if (is_security_key_index_changed (mrp->sec_param.key_identifier[0]) == 1)
          {         
            //set_live_index_in_supplicant (mrp->sec_param.key_identifier[0]);
            update_self_pan_version (nbr_desc->pan_ver);
          }
        }
        
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
        send_runtime_log ("RECVD PC from "LOG_MAC_ADDRESS(mrp->src.address.ieee_address));
        send_runtime_log ("--PAN version = %d", nbr_desc->pan_ver);
#endif        
      }
#endif  
      
    }
    break;
    
  case WP_IE_SUBIE_SUBID_GTKHASH_IE_SHORT:
    if(mrp->recived_frame_type == PAN_CONFIG)
    {
      memcpy (&nbr_desc->recv_gtk_hash.gtk0_hash, ie_ptr, 8);
      ie_ptr += 8;
      memcpy (&nbr_desc->recv_gtk_hash.gtk1_hash, ie_ptr, 8);
      ie_ptr += 8;
      memcpy (&nbr_desc->recv_gtk_hash.gtk2_hash, ie_ptr, 8);
      ie_ptr += 8;
      memcpy (&nbr_desc->recv_gtk_hash.gtk3_hash, ie_ptr, 8);
#if (APP_LBR_ROUTER == 1)        
#if !(WITH_SECURITY)   
#if !(ENABLE_FAN_MAC_WITHOUT_SECURITY)
      
#if(FAN_EAPOL_FEATURE_ENABLED == 1)
      if (memcmp (mrp->src.address.ieee_address, eapol_parent_child_info.sle_eapol_parent, 8))
        break;
#endif
      
      if (get_node_type() == 0x01)    //i.e, only for router
      {
        if (memcmp (nbr_desc->recv_gtk_hash.gtk0_hash, mac_self_fan_info.gtk_hash_ele.gtk0_hash, 8))
        {
//          mark_change_in_gtkl (0);
          reset_incoming_frame_counter_for_stale_key (0);
          trickle_timer_consistency_pc ();
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
          send_runtime_log ("GTK 1 Mismatch");
#endif           
        }
        if (memcmp(nbr_desc->recv_gtk_hash.gtk1_hash,mac_self_fan_info.gtk_hash_ele.gtk1_hash,8))
        {
//          mark_change_in_gtkl (1);
          reset_incoming_frame_counter_for_stale_key (1);
          trickle_timer_consistency_pc ();
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
          send_runtime_log ("GTK 2 Mismatch");
#endif           
        }
        if (memcmp(nbr_desc->recv_gtk_hash.gtk2_hash,mac_self_fan_info.gtk_hash_ele.gtk2_hash,8))
        {
//          mark_change_in_gtkl (2);
          reset_incoming_frame_counter_for_stale_key (2);
          trickle_timer_consistency_pc ();
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
          send_runtime_log ("GTK 3 Mismatch");
#endif           
        }
        if (memcmp(nbr_desc->recv_gtk_hash.gtk3_hash,mac_self_fan_info.gtk_hash_ele.gtk3_hash,8))
        {
//          mark_change_in_gtkl (3);
          reset_incoming_frame_counter_for_stale_key (3);
          trickle_timer_consistency_pc ();
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
          send_runtime_log ("GTK 4 Mismatch");
#endif           
        }
      }
#endif   
#endif 
#endif//APP_LBR_ROUTER  
    }
    break;
  }
  return 0;             //Success
}

static uchar extract_payload_long_ie(uint8_t sub_id, uint8_t *p_content_location ,
                         mac_rx_t *mrp , uint16_t sub_ie_contlen, 
                         mac_nbr_descriptor_t* nbr_desc)
{
  uint8_t channel_control = 0;
  switch(sub_id)
  {
  case WP_IE_SUBIE_SUBID_US_IE_LONG:/*01*/
    
    if(mrp->recived_frame_type == FAN_ACK)
      break;    /*Packet is not dropped but IE is not stored*/
    
    nbr_desc->ucl_sched.us_schedule.dwell_interval = *p_content_location++;
    nbr_desc->ucl_sched.us_schedule.clock_drift = *p_content_location++;
    nbr_desc->ucl_sched.us_schedule.timing_accuracy = *p_content_location++;
    channel_control = *p_content_location;
    nbr_desc->ucl_sched.us_schedule.channel_plan = *p_content_location & 0x07;
    nbr_desc->ucl_sched.us_schedule.channel_function = ((channel_control>>3)&0x07);
    nbr_desc->ucl_sched.us_schedule.excludded_channel_control = (channel_control>>6)&0x07;
    if(((*p_content_location & 0x07) == CHANNEL_PLAN_REG_DOMAIN/*00*/))
    {        
      p_content_location++;
      nbr_desc->ucl_sched.un_channel_plan.ch_reg_op.reg_domain = *p_content_location++;
      nbr_desc->ucl_sched.un_channel_plan.ch_reg_op.op_class = *p_content_location++;
    }
    else
    {  
      p_content_location++;
      /*application specific*/
      memcpy(((uint8_t*)&(nbr_desc->ucl_sched.un_channel_plan.ch_explicit.ch0)),p_content_location,3);
      p_content_location += 3;                                 
      nbr_desc->ucl_sched.un_channel_plan.ch_explicit.channel_spacing = (*p_content_location & 0x0F);
      p_content_location++;
      memcpy((uint8_t*)&(nbr_desc->ucl_sched.un_channel_plan.ch_explicit.num_chans),p_content_location,2);
      p_content_location += 2;
    }
    
    switch(((channel_control>>3)&0x07))
    {                   
    case CF_FIXED_CHANNEL:
      memcpy((uint8_t* )&nbr_desc->ucl_sched.channel_fixed.fixed_chan,p_content_location,2);
      p_content_location+=2;
      break;     
#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
    case CF_TR51:/*fall through*/
    case CF_DH1:
      {
        uint64_t time_local = 0;
        uint32_t ufsi = (nbr_desc->ufsi);
        time_local = (uint64_t)((uint64_t)(65536)*(uint64_t)ufsi*(uint64_t)(1001));
        time_local = (uint64_t)(time_local/16777216); 
        time_local+=2;
        time_local = time_local*1000;
        nbr_desc->ucl_sched.us_schedule.local_time_node = time_local;
      }
      break;  
#endif
    case CF_VENDOR_SPECIFIC:
      nbr_desc->ucl_sched.us_schedule.chan_hop_count = *p_content_location++;
      memcpy(nbr_desc->ucl_sched.us_schedule.chan_hop_list,p_content_location,nbr_desc->ucl_sched.us_schedule.chan_hop_count);
      p_content_location += nbr_desc->ucl_sched.us_schedule.chan_hop_count; 
      break;
    default:
      break;
    }
    
    switch(((channel_control>>6)&0x07))
    {
    case EXC_CHANNEL_CTRL_NIL:
      break;
    case EXC_CHANNEL_CTRL_RANGE:
      nbr_desc->ucl_sched.us_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges = *p_content_location++;
      memcpy(nbr_desc->ucl_sched.us_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range,(p_content_location),
             ( (nbr_desc->ucl_sched.us_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges)*4)); 
      break; 
    case EXC_CHANNEL_CTRL_BITMASK: 
      memcpy( nbr_desc->ucl_sched.us_schedule.excluded_channels.excluded_channel_mask,p_content_location,8); 
      break;
    default:
      break;
    }
    break;/*WP_IE_SUBIE_SUBID_US_IE_LONG*/
    
  case WP_IE_SUBIE_SUBID_BS_IE_LONG:/*02*/
    
    if ((mrp->recived_frame_type == PAN_ADVERT_FRAME) ||
        (mrp->recived_frame_type == PAN_ADVERT_SOLICIT) ||
          (mrp->recived_frame_type == PAN_CONFIG_SOLICIT) ||
            (mrp->recived_frame_type == FAN_ACK))
      break;    /*Packet is not dropped but IE is not stored*/
    
    memcpy(&nbr_desc->bcl_sched.bcast_interval,p_content_location,4);
    p_content_location+=4;
    memcpy(&nbr_desc->bcl_sched.bcast_sched_id,p_content_location,2);
    p_content_location+=2;
    nbr_desc->bcl_sched.bs_schedule.dwell_interval = *p_content_location++;
    nbr_desc->bcl_sched.bs_schedule.clock_drift = *p_content_location++;
    nbr_desc->bcl_sched.bs_schedule.timing_accuracy = *p_content_location++;
     channel_control = *p_content_location;
    nbr_desc->bcl_sched.bs_schedule.channel_plan = *p_content_location & 0x07;
    nbr_desc->bcl_sched.bs_schedule.channel_function = ((channel_control>>3)&0x07);
    nbr_desc->bcl_sched.bs_schedule.excludded_channel_control = (channel_control>>6)&0x07;
    if(((*p_content_location &0x07) == CHANNEL_PLAN_REG_DOMAIN))
    {        
      p_content_location++;
      nbr_desc->bcl_sched.un_channel_plan.ch_reg_op.reg_domain =  *p_content_location++;
      nbr_desc->bcl_sched.un_channel_plan.ch_reg_op.op_class = *p_content_location++;
    }
    else
    {  
      p_content_location++;
      /*application specific*/
      memcpy(((uint8_t*)&(nbr_desc->bcl_sched.un_channel_plan.ch_explicit.ch0)),p_content_location,3);
      p_content_location += 3;                                 
      nbr_desc->bcl_sched.un_channel_plan.ch_explicit.channel_spacing = (*p_content_location & 0x0F);
      p_content_location++;                                
      nbr_desc->bcl_sched.un_channel_plan.ch_explicit.num_chans = get_ushort(p_content_location);
      p_content_location += 2;
    }
    
    switch((channel_control>>3)&07)
    {                      
    case CF_FIXED_CHANNEL:
      memcpy((uint8_t* )&nbr_desc->bcl_sched.channel_fixed.fixed_chan,p_content_location,2);
      p_content_location+=2;
      break;
#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
    case CF_TR51:/*fall through*/
    case CF_DH1:
      break;
#endif      
    case CF_VENDOR_SPECIFIC:
      nbr_desc->bcl_sched.bs_schedule.chan_hop_count = *p_content_location++;
      memcpy(nbr_desc->bcl_sched.bs_schedule.chan_hop_list,p_content_location,nbr_desc->bcl_sched.bs_schedule.chan_hop_count);
      p_content_location += nbr_desc->bcl_sched.bs_schedule.chan_hop_count; 
      break;
    default:
      break;
    }
    
    switch((channel_control>>6)&0x07)
    {
    case EXC_CHANNEL_CTRL_NIL:
      break;
    case EXC_CHANNEL_CTRL_RANGE:
      nbr_desc->bcl_sched.bs_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges = *p_content_location++;
      memcpy(nbr_desc->bcl_sched.bs_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range,(p_content_location),
             ( (nbr_desc->bcl_sched.bs_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges)*4)); 
      break;
    case EXC_CHANNEL_CTRL_BITMASK: 
      memcpy( nbr_desc->bcl_sched.bs_schedule.excluded_channels.excluded_channel_mask,p_content_location,8); 
      break;
    default:
      break;
    }
    break;/*WP_IE_SUBIE_SUBID_BS_IE_LONG*/
  }
  return 0;     //Success
}

static int fan_mac_nbr_update_ies (mac_nbr_descriptor_t* nbr_table_instance, 
                             mac_nbr_descriptor_t* src_instance, 
                             uint32_t hdr_bitmap, uint32_t pld_bitmap)
{  
  /* Header Sub IEs */
  if (hdr_bitmap & UTT_IE_MASK)
  {
    nbr_table_instance->ufsi = src_instance->ufsi;
    nbr_table_instance->ut_ie_rx_time = src_instance->ut_ie_rx_time;
  }
  if (hdr_bitmap & BT_IE_MASK)
  {
    nbr_table_instance->broad_cast_slno = src_instance->broad_cast_slno;
    nbr_table_instance->broad_frac_int_offset = 
      src_instance->broad_frac_int_offset;
    nbr_table_instance->btie_rcvd_timestamp = src_instance->btie_rcvd_timestamp;
  }
  if (hdr_bitmap & FC_IE_MASK)
  {
    nbr_table_instance->recv_fcie = src_instance->recv_fcie;
    /*TO-DO*/
  }
  if (hdr_bitmap & RSL_IE_MASK)
  {
    nbr_table_instance->rsl = src_instance->rsl;
  }
  if (hdr_bitmap & MHDS_IE_MASK)
  {
    /*TO-DO*/
  }
  if (hdr_bitmap & VH_IE_MASK)
  {
    /*TO-DO*/
  }
  if (hdr_bitmap & EA_IE_MASK)
  {
    memcpy ((uint8_t *)nbr_table_instance->eui_infor.src_addr, 
            (uint8_t *)src_instance->eui_infor.src_addr, 8);
  }
  
  /* Payload Sub IEs */
  if (pld_bitmap & US_IE_MASK)
  {
    /*suneet :: A FAN node MUST support operation with UDI values of 15 to 255 (inclusive).*/
    if((src_instance->ucl_sched.us_schedule.dwell_interval >= 15)
       || (src_instance->ucl_sched.us_schedule.channel_function == 0))
    {
      nbr_table_instance->ucl_sched.us_schedule.dwell_interval = 
        src_instance->ucl_sched.us_schedule.dwell_interval;
      nbr_table_instance->ucl_sched.us_schedule.clock_drift = 
        src_instance->ucl_sched.us_schedule.clock_drift;
      nbr_table_instance->ucl_sched.us_schedule.timing_accuracy = 
        src_instance->ucl_sched.us_schedule.timing_accuracy;
      nbr_table_instance->ucl_sched.us_schedule.channel_plan = 
        src_instance->ucl_sched.us_schedule.channel_plan;
      nbr_table_instance->ucl_sched.us_schedule.channel_function = 
        src_instance->ucl_sched.us_schedule.channel_function;
      nbr_table_instance->ucl_sched.us_schedule.excludded_channel_control = 
        src_instance->ucl_sched.us_schedule.excludded_channel_control;
      nbr_table_instance->ucl_sched.un_channel_plan.ch_reg_op.reg_domain = 
        src_instance->ucl_sched.un_channel_plan.ch_reg_op.reg_domain;
      nbr_table_instance->ucl_sched.un_channel_plan.ch_reg_op.op_class = 
        src_instance->ucl_sched.un_channel_plan.ch_reg_op.op_class;
      nbr_table_instance->ucl_sched.un_channel_plan.ch_explicit.ch0 = 
        src_instance->ucl_sched.un_channel_plan.ch_explicit.ch0;
      nbr_table_instance->ucl_sched.un_channel_plan.ch_explicit.channel_spacing = 
        src_instance->ucl_sched.un_channel_plan.ch_explicit.channel_spacing;                    
      nbr_table_instance->ucl_sched.un_channel_plan.ch_explicit.num_chans = 
        src_instance->ucl_sched.un_channel_plan.ch_explicit.num_chans;
      nbr_table_instance->ucl_sched.channel_fixed.fixed_chan = 
        src_instance->ucl_sched.channel_fixed.fixed_chan;
      nbr_table_instance->ucl_sched.us_schedule.local_time_node = 
        src_instance->ucl_sched.us_schedule.local_time_node;
      nbr_table_instance->ucl_sched.us_schedule.chan_hop_count = 
        src_instance->ucl_sched.us_schedule.chan_hop_count;
      memcpy(nbr_table_instance->ucl_sched.us_schedule.chan_hop_list,
             src_instance->ucl_sched.us_schedule.chan_hop_list,
             nbr_table_instance->ucl_sched.us_schedule.chan_hop_count); 
      nbr_table_instance->ucl_sched.us_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges = 
        src_instance->ucl_sched.us_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges;
      memcpy (nbr_table_instance->ucl_sched.us_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range,
              src_instance->ucl_sched.us_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range,
              ((nbr_table_instance->ucl_sched.us_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges)*4)); 
      memcpy (nbr_table_instance->ucl_sched.us_schedule.excluded_channels.excluded_channel_mask,
              src_instance->ucl_sched.us_schedule.excluded_channels.excluded_channel_mask,8);
      make_nbr_valid_channel_list(nbr_table_instance);//Suneet :: create channel list for nbr
    }
    else
      return -1;
  }
  if (pld_bitmap & BS_IE_MASK)
  {
    /*suneet :: A FAN  node MUST support operation with BDI values of 100 to 255 (inclusive).*/
    if((src_instance->bcl_sched.bcast_interval >= 100)
       ||(src_instance->bcl_sched.bs_schedule.channel_function == 0))
    {
      nbr_table_instance->bcl_sched.bcast_interval = 
        src_instance->bcl_sched.bcast_interval;
      nbr_table_instance->bcl_sched.bcast_sched_id = 
        src_instance->bcl_sched.bcast_sched_id;
      nbr_table_instance->bcl_sched.bs_schedule.dwell_interval = 
        src_instance->bcl_sched.bs_schedule.dwell_interval;
      nbr_table_instance->bcl_sched.bs_schedule.clock_drift = 
        src_instance->bcl_sched.bs_schedule.clock_drift;
      nbr_table_instance->bcl_sched.bs_schedule.timing_accuracy = 
        src_instance->bcl_sched.bs_schedule.timing_accuracy;
      nbr_table_instance->bcl_sched.bs_schedule.channel_plan = 
        src_instance->bcl_sched.bs_schedule.channel_plan;
      nbr_table_instance->bcl_sched.bs_schedule.channel_function = 
        src_instance->bcl_sched.bs_schedule.channel_function;
      nbr_table_instance->bcl_sched.bs_schedule.excludded_channel_control = 
        src_instance->bcl_sched.bs_schedule.excludded_channel_control;
      nbr_table_instance->bcl_sched.un_channel_plan.ch_reg_op.reg_domain = 
        src_instance->bcl_sched.un_channel_plan.ch_reg_op.reg_domain;
      nbr_table_instance->bcl_sched.un_channel_plan.ch_reg_op.op_class = 
        src_instance->bcl_sched.un_channel_plan.ch_reg_op.op_class;
      nbr_table_instance->bcl_sched.un_channel_plan.ch_explicit.ch0 = 
        src_instance->bcl_sched.un_channel_plan.ch_explicit.ch0;
      nbr_table_instance->bcl_sched.un_channel_plan.ch_explicit.channel_spacing = 
        src_instance->bcl_sched.un_channel_plan.ch_explicit.channel_spacing;
      nbr_table_instance->bcl_sched.un_channel_plan.ch_explicit.num_chans = 
        src_instance->bcl_sched.un_channel_plan.ch_explicit.num_chans;
      nbr_table_instance->bcl_sched.channel_fixed.fixed_chan = 
        src_instance->bcl_sched.channel_fixed.fixed_chan;
      nbr_table_instance->bcl_sched.bs_schedule.chan_hop_count = 
        src_instance->bcl_sched.bs_schedule.chan_hop_count;
      memcpy(nbr_table_instance->bcl_sched.bs_schedule.chan_hop_list,
             src_instance->bcl_sched.bs_schedule.chan_hop_list,
             nbr_table_instance->bcl_sched.bs_schedule.chan_hop_count);
      nbr_table_instance->bcl_sched.bs_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges = 
        src_instance->bcl_sched.bs_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges;
      memcpy (nbr_table_instance->bcl_sched.bs_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range,
              src_instance->bcl_sched.bs_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range,
              ((nbr_table_instance->bcl_sched.bs_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges)*4)); 
      memcpy (nbr_table_instance->bcl_sched.bs_schedule.excluded_channels.excluded_channel_mask,
              src_instance->bcl_sched.bs_schedule.excluded_channels.excluded_channel_mask, 8); 
    }
    else
      return -1;
  }
  if (pld_bitmap & VP_IE_MASK)
  {
    /*TO-DO*/
  }
  if (pld_bitmap & PAN_IE_MASK)
  {
    nbr_table_instance->rev_pan_metrics.pan_size = 
      src_instance->rev_pan_metrics.pan_size;
    nbr_table_instance->rev_pan_metrics.routing_cost = 
      src_instance->rev_pan_metrics.routing_cost;
    nbr_table_instance->rev_pan_metrics.parent_bs_ie_use = 
      src_instance->rev_pan_metrics.parent_bs_ie_use;
    nbr_table_instance->rev_pan_metrics.routing_methood = 
      src_instance->rev_pan_metrics.routing_methood;
    nbr_table_instance->rev_pan_metrics.fan_tps_version = 
      src_instance->rev_pan_metrics.fan_tps_version;
    nbr_table_instance->rev_pan_metrics.pan_id = 
      src_instance->rev_pan_metrics.pan_id;

/* Debdeep :: PAN cost should be calculated here while storing */    
    calu_pan_cost_recv_pan();
  }
  if (pld_bitmap & NETNAME_IE_MASK)
  {
    nbr_table_instance->net_name_length = src_instance->net_name_length;
    memcpy (nbr_table_instance->net_name, src_instance->net_name, 
            nbr_table_instance->net_name_length);
  }
  if (pld_bitmap & PAN_VER_IE_MASK)
  {
    nbr_table_instance->pan_ver = src_instance->pan_ver;
  }
  if (pld_bitmap & GTK_HASH_IE_MASK)
  {
    memcpy (nbr_table_instance->recv_gtk_hash.gtk0_hash, 
            src_instance->recv_gtk_hash.gtk0_hash, 8);
    memcpy (nbr_table_instance->recv_gtk_hash.gtk1_hash, 
            src_instance->recv_gtk_hash.gtk1_hash, 8);
    memcpy (nbr_table_instance->recv_gtk_hash.gtk2_hash, 
            src_instance->recv_gtk_hash.gtk2_hash, 8);
    memcpy (nbr_table_instance->recv_gtk_hash.gtk3_hash, 
            src_instance->recv_gtk_hash.gtk3_hash, 8);
  }
  return 0;
}        

static uint8_t device_should_be_added (uint8_t frame_type)
{
  if (((frame_type == PAN_ADVERT_FRAME) || (frame_type == PAN_ADVERT_SOLICIT)) 
      && (get_join_state() == 1))
    return 1;
  
  if ((
#if(FAN_EAPOL_FEATURE_ENABLED == 1)         
       (frame_type == EAPOL) || 
#endif       
       (frame_type == FAN_ACK))
      && (get_join_state() == 2))
    return 1;
  if (((frame_type == PAN_CONFIG) || (frame_type == PAN_CONFIG_SOLICIT) ||
#if(FAN_EAPOL_FEATURE_ENABLED == 1)       
       (frame_type == EAPOL) &&
#endif      
      
      (get_join_state() == 3)))
    return 1;
  if (((frame_type == FAN_DATA_PKT) || (frame_type == FAN_ACK) || 
#if(FAN_EAPOL_FEATURE_ENABLED == 1)       
       (frame_type == EAPOL) || 
#endif         
         (frame_type == PAN_CONFIG)) && (get_join_state() == 4))
    return 1;
  
  if (get_join_state() == 5)
    return 1;
  
  return 0;
}

static uint32_t get_ie_mask_from_id (uint8_t subIE, uint8_t type)
{
  uint32_t mask = 0;
  
  if (type == HEADER_IE_TYPE)
  {
    switch (subIE)
    {
    case WH_IE_SUBID_UTT_IE_SHORT:
      mask = UTT_IE_MASK;
      break;
    case WH_IE_SUBID_BT_IE_SHORT:
      mask = BT_IE_MASK;
      break;
    case WH_IE_SUBID_FC_IE_SHORT:
      mask = FC_IE_MASK;
      break;
    case WH_IE_SUBID_RSL_IE_SHORT:
      mask = RSL_IE_MASK;
      break;
    case WH_IE_SUBID_MHDS_IE_SHORT:
      mask = MHDS_IE_MASK;
      break;
    case WH_IE_SUBID_VH_IE_SHORT:
      mask = VH_IE_MASK;
      break;
    case WH_IE_SUBID_EA_IE_SHORT:
      mask = EA_IE_MASK;
      break;
    default:
      break;
    }
  }
  else
  {
    switch (subIE)
    {
    case WP_IE_SUBIE_SUBID_US_IE_LONG:
      mask = US_IE_MASK;
      break;
    case WP_IE_SUBIE_SUBID_BS_IE_LONG:
      mask = BS_IE_MASK;
      break;
    case WP_IE_SUBIE_SUBID_VP_IE_LONG:
      mask = VP_IE_MASK;
      break;
    case WP_IE_SUBIE_SUBID_PAN_IE_SHORT:
      mask = PAN_IE_MASK;
      break;
    case WP_IE_SUBIE_SUBID_NETNAME_IE_SHORT:
      mask = NETNAME_IE_MASK;
      break;
    case WP_IE_SUBIE_SUBID_PANVER_IE_SHORT:
      mask = PAN_VER_IE_MASK;
      break;
    case WP_IE_SUBIE_SUBID_GTKHASH_IE_SHORT:
      mask = GTK_HASH_IE_MASK;
      break;
    }
  }
  
  return mask;
}

static bool is_subIE_supported (mac_rx_t *mrp, uint8_t subIE, uint8_t type)
{
  uint32_t ie_mask;
  
  if (type == HEADER_IE_TYPE)        //Header subIE
  {    
    ie_mask = get_ie_mask_from_id (subIE, type);
    
    if (mrp->wisun_fan_ies->hdr_subIE_bitmap & ie_mask)
    {
      if (ie_mask != VH_IE_MASK)
      return FALSE;
    }
    
    mrp->wisun_fan_ies->hdr_subIE_bitmap |= ie_mask;
    return TRUE;
  }
  else                  //Payload subIE
  {    
    ie_mask = get_ie_mask_from_id (subIE, type);
    
    if (mrp->wisun_fan_ies->pld_subIE_bitmap & ie_mask)
    {
      if (ie_mask != VP_IE_MASK)
        return FALSE;
    }
    
    mrp->wisun_fan_ies->pld_subIE_bitmap |= ie_mask;
    return TRUE;
  }
}
/******************************************************************************/

/******************************************************************************/
/*Function used for constructing both PLD and HDR IEs with contents if they are supported*/

static uint16_t build_uttie (uint8_t *buf)
{
  uint8_t *start = buf;
  uint32_t ufsi = 0;
  uint16_t ie_desc = 0;
  
  buf += IE_DISCRIPTOR_LEN;  
  *buf++ = WH_IE_SUBID_UTT_IE_SHORT;
  *buf++ = fan_mac_params.type;
  ufsi = 0xFF;//mac_cb_update_ufsi(proc_delay);
  memcpy(buf, (uint8_t*)&ufsi, 3);
  buf += 3;
  
  ie_desc = create_ie_desc (0, WH_IE, (buf - start - IE_DISCRIPTOR_LEN - 1)); 
  start[0] = ie_desc % 256;
  start[1] = ie_desc / 256;
  
  return buf - start;
}

static uint16_t build_btie (uint8_t *buf)
{
  uint8_t *start = buf;
  uint32_t BFIO = 0x00;
  uint16_t ie_desc = 0;
  
  buf += IE_DISCRIPTOR_LEN; 
  *buf++ = WH_IE_SUBID_BT_IE_SHORT;
  
  /*Suneet: Refer to  spec to write Broadcast Slot Number
  Broadcast Fractional Interval Offset use spec wirte formula*/
  memcpy(buf, ((uint8_t*)&mac_self_fan_info.bcast_slot_no), 2);
  buf += 2;
  
  /*The Broadcast Interval Offset is a 24-bit unsigned integer representing 
  the time, in milliseconds, since the beginning of the current (most recent) 
  broadcast slot. Current fan spec 1.21*/
  BFIO = trxsm_p->bc_chan_hop_seq_current_slot_time;
  BFIO = (BFIO << 8);  //Suneet :: it's shift first byte and fill three packt
  memcpy(buf, (uint8_t*)&BFIO, 3);
  buf += 3;
  
  ie_desc = create_ie_desc (0, WH_IE, (buf - start - IE_DISCRIPTOR_LEN - 1)); 
  start[0] = ie_desc % 256;
  start[1] = ie_desc / 256;
  
  return buf - start;
}

#if(FAN_EDFE_FEATURE_ENABLED == 1)
static uint16_t build_fcie (uint8_t *buf)
{
  uint8_t *start = buf;
  uint16_t ie_desc = 0;
  
  buf += IE_DISCRIPTOR_LEN; 
  *buf++ = WH_IE_SUBID_FC_IE_SHORT;
#if 0 // suneet :: open of required   
  if(edfe_information.edfe_frame_tx_type == INITIAL_FRAME)
  {
     edfe_information.edfe_transmit_flow_contrl = 0xFF;
    if(edfe_information.edfe_trigger_packt == 0x00)
    {
      *buf++ = 0x32;//Transmit Flow Control
    }
    else if(edfe_information.edfe_trigger_packt == 0x01)
    {
      *buf++ = 0x00;//Transmit Flow Control
    }
    else if(edfe_information.edfe_trigger_packt > 0x01)
    {
      *buf++ = 0x32;//Transmit Flow Control
    }
    *buf++ = 0xFF;//Receive Flow Control
    edfe_information.edfe_receiver_flow_contrl = 0xFF;
  }
  else if(edfe_information.edfe_frame_tx_type == RESPONSE_FRAME)
  {
    if(edfe_information.edfe_trigger_packt == edfe_information.edfe_sent_pkt)
    {
      edfe_information.edfe_transmit_flow_contrl = 0x00;
      *buf++ = 0x00;//Transmit Flow Control
    }
    else if(edfe_information.edfe_trigger_packt != 0x00)
    {
      *buf++ = 0x32;//Transmit Flow Control
    }
    else
    {
      *buf++ = 0x00;//Transmit Flow Control
      edfe_information.edfe_transmit_flow_contrl = 0x00;
    }
    *buf++ = 0xFF;//Receive Flow Control
  }
  else if(edfe_information.edfe_frame_tx_type == FINAL_RESPONSE_FRAME)
  {
    *buf++ = 0x00;//Transmit Flow Control
    *buf++ = 0x00;//Receive Flow Control
    edfe_information.edfe_frame_tx_type = 0x99;
  }
    
  if(edfe_information.edfe_trigger_packt != 0x00)
  {
      edfe_information.edfe_trigger_packt--;
  }

#endif 
  
  ie_desc = create_ie_desc (0, WH_IE, (buf - start - IE_DISCRIPTOR_LEN - 1)); 
  start[0] = ie_desc % 256;
  start[1] = ie_desc / 256;
  
  return buf - start;
}
#endif  //#if(FAN_EDFE_FEATURE_ENABLED == 1)

static uint16_t build_rslie (uint8_t *buf)
{
  uint8_t rsl_value = 0; 
  uint8_t *start = buf;
  uint16_t ie_desc = 0;
  
  buf += IE_DISCRIPTOR_LEN; 
  *buf++ = WH_IE_SUBID_RSL_IE_SHORT;
  
  /*RSL MUST be calculated as the received signal level relative to stand-
  ard thermal noise (290oK) at 1 Hz bandwidth or -919 174 dBm. This pro-
  vides a range of -174 (0) to +80 (254) dBm.(255 is reserved). The RSL 
  920 calculation is subsequently fed into an EWMA using a smoothing fa-
  ctor of 1/8.
  */
  rsl_value = get_LQI_from_RSSI((int8_t)rcvd_siganl_strength);
  *buf++ = rsl_value;
  ie_desc = create_ie_desc (0, WH_IE, (buf - start - IE_DISCRIPTOR_LEN - 1)); 
  start[0] = ie_desc % 256;
  start[1] = ie_desc / 256;
  
  return buf - start;
}

static uint16_t build_mhdsie (uint8_t *buf)
{
  uint8_t *start = buf;
  uint16_t ie_desc = 0;
  
  buf += IE_DISCRIPTOR_LEN; 
  *buf++ = WH_IE_SUBID_MHDS_IE_SHORT;
  
  ie_desc = create_ie_desc (0, WH_IE, (buf - start - IE_DISCRIPTOR_LEN - 1)); 
  start[0] = ie_desc % 256;
  start[1] = ie_desc / 256;
  
  return buf - start;
}

static uint16_t build_vhie (uint8_t *buf)
{
  uint8_t *start = buf;
  uint16_t ie_desc = 0;
  
  buf += IE_DISCRIPTOR_LEN; 
  *buf++ = WH_IE_SUBID_VH_IE_SHORT;
  
  *buf++=0xDD;
  *buf++=0xFF;

  ie_desc = create_ie_desc (0, WH_IE, (buf - start - IE_DISCRIPTOR_LEN - 1)); 
  start[0] = ie_desc % 256;
  start[1] = ie_desc / 256;
  
  return buf - start;
}

static uint16_t build_eaie (uint8_t *buf)
{
  uint8_t *start = buf;
  uint8_t self_addr[8] = {0};
  uint16_t ie_desc = 0;
  
  buf += IE_DISCRIPTOR_LEN; 
  *buf++ = WH_IE_SUBID_EA_IE_SHORT;
  
  if(fan_mac_information_data.fan_node_type == 0x00)
  {
    get_self_extended_address_reverse(self_addr);
  }
  else
  {
    mem_rev_cpy(self_addr,authnt_interfac_id,8);
  }
  memcpy(buf,self_addr,8);
  buf += 8;

  ie_desc = create_ie_desc (0, WH_IE, (buf - start - IE_DISCRIPTOR_LEN - 1)); 
  start[0] = ie_desc % 256;
  start[1] = ie_desc / 256;
  
  return buf - start;
}

static uint16_t build_unknown_sub_hdr_ie (uint8_t *buf)
{
  uint8_t *start = buf;
  uint16_t ie_desc = 0;
  
  buf += IE_DISCRIPTOR_LEN; 
  *buf++ = UNKNOWN_SUB_HDR_IE_SHORT;
  
  *buf++ = 0x0A;
  *buf++ = 0x0B;
  *buf++ = 0xCD;

  ie_desc = create_ie_desc (0, WH_IE, (buf - start - IE_DISCRIPTOR_LEN - 1)); 
  start[0] = ie_desc % 256;
  start[1] = ie_desc / 256;
  
  return buf - start;
}

static uint16_t build_usie (uint8_t *buf)
{
  int ii;
  uint8_t *start = buf;
  us_ch_schedule_t* chan_sched = &mac_self_fan_info.unicast_listening_sched;
  uint16_t sub_ie_desc = 0;
  
  buf += IE_DISCRIPTOR_LEN;
  
  *buf++ = chan_sched->us_schedule.dwell_interval;
  *buf++ = chan_sched->us_schedule.clock_drift;
  *buf++ = chan_sched->us_schedule.timing_accuracy;
  *buf++ = (((chan_sched->us_schedule.channel_plan) & 0x07)) |
    ((chan_sched->us_schedule.channel_function << 3) & 0x38) |
      ((chan_sched->us_schedule.excludded_channel_control << 6) & 0xC0);
  
  switch( chan_sched->us_schedule.channel_plan)
  {
  case CHANNEL_PLAN_REG_DOMAIN:
    *buf++ = chan_sched->un_channel_plan.ch_reg_op.reg_domain;
    *buf++ = chan_sched->un_channel_plan.ch_reg_op.op_class;
    break;
  case CHANNEL_PLAN_APP_SPEC:
    memcpy (buf, (uint8_t*)&chan_sched->un_channel_plan.ch_explicit.ch0, 3);
    buf += 3;                                              
    memcpy (buf, 
           (uint8_t*)&chan_sched->un_channel_plan.ch_explicit.channel_spacing,
           1);
    buf += 1;
    memcpy (buf, (uint8_t*)&(chan_sched->un_channel_plan.ch_explicit.num_chans), 2);
    buf += 2;
    break;
  default:
    break;
  }
  
  switch( chan_sched->us_schedule.channel_function)
  {
  case CF_FIXED_CHANNEL:
    put_ushort (buf,(chan_sched->channel_fixed.fixed_chan));
    buf += 2;
    break;
#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
  case CF_TR51:
  case CF_DH1:
    break;
#endif
    
  case CF_VENDOR_SPECIFIC:
    *buf++ = chan_sched->us_schedule.chan_hop_count;
    memcpy (buf, chan_sched->us_schedule.chan_hop_list, 
            chan_sched->us_schedule.chan_hop_count);
    buf += chan_sched->us_schedule.chan_hop_count;
    break;
  default:
    break;
  }  
  
  switch(chan_sched->us_schedule.excludded_channel_control)
  {
  case EXC_CHANNEL_CTRL_NIL:
    break;
  case EXC_CHANNEL_CTRL_RANGE:
    *buf++ = chan_sched->us_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges;
    for(ii=0; ii<chan_sched->us_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges; ii++)
    {
      memcpy (buf, &chan_sched->us_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[ii].start_ch, 2);
      buf+=2;
      memcpy (buf, &chan_sched->us_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[ii].end_ch, 2);
      buf+=2;  
    }
    break;
  case EXC_CHANNEL_CTRL_BITMASK:
    memcpy (buf, chan_sched->us_schedule.excluded_channels.excluded_channel_mask, 8);
    buf += 8;
    break;
  default:
    break;
  }
  
  sub_ie_desc = create_sub_ie_desc (LONG_SUB_IE, WP_IE_SUBIE_SUBID_US_IE_LONG, 
                                    buf - start - IE_DISCRIPTOR_LEN);
  
  start[0] = sub_ie_desc % 256;
  start[1] = sub_ie_desc / 256;
  
  return buf - start;
}

static uint16_t build_bsie (uint8_t *buf)
{
  int ii;
  uint8_t *start = buf;  
  bc_ch_schedule_t* chan_sched = &mac_self_fan_info.bcast_sched;
  uint16_t sub_ie_desc = 0;
  
  buf += IE_DISCRIPTOR_LEN;
  
  if((mac_self_fan_info.pan_metrics.parent_bs_ie_use == 0x01)
     &&(get_node_type() == 0x00))
  {     
    memcpy(buf,&chan_sched->bcast_interval,4);
    buf+=4;
    memcpy(buf,&chan_sched->bcast_sched_id,2);
    buf+=2;
    *buf++ = chan_sched->bs_schedule.dwell_interval;
    *buf++ = chan_sched->bs_schedule.clock_drift;
    *buf++ = chan_sched->bs_schedule.timing_accuracy;
    *buf++ = (((chan_sched->bs_schedule.channel_plan)&0x07)) |
      ((chan_sched->bs_schedule.channel_function<<3)&0x38) |
        ((chan_sched->bs_schedule.excludded_channel_control<<6)&0xC0);
    
    switch( chan_sched->bs_schedule.channel_plan)
    {
    case CHANNEL_PLAN_REG_DOMAIN:
      *buf++ = chan_sched->un_channel_plan.ch_reg_op.reg_domain;
      *buf++ = chan_sched->un_channel_plan.ch_reg_op.op_class;
      break;
    case CHANNEL_PLAN_APP_SPEC:
      memcpy (buf, (uint8_t*)&chan_sched->un_channel_plan.ch_explicit.ch0, 3);
      buf += 3;                                              
      memcpy (buf, (uint8_t*)&chan_sched->un_channel_plan.ch_explicit.channel_spacing ,1);
      buf += 1;
      memcpy (buf, (uint8_t*)&chan_sched->un_channel_plan.ch_explicit.num_chans, 2);
      buf += 2;
      break;
    default:
      break;
    }
    
    switch(chan_sched->bs_schedule.channel_function)
    {
    case CF_FIXED_CHANNEL:
      put_ushort(buf,(chan_sched->channel_fixed.fixed_chan));
      buf += 2;
      break;
#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
    case CF_TR51:
    case CF_DH1:
      break;
#endif
      
    case  CF_VENDOR_SPECIFIC:
      *buf++ = chan_sched->bs_schedule.chan_hop_count;
      memcpy (buf, chan_sched->bs_schedule.chan_hop_list,
              chan_sched->bs_schedule.chan_hop_count);
      buf += chan_sched->bs_schedule.chan_hop_count;
      break;
    default:
      break;
    }
    
    switch(chan_sched->bs_schedule.excludded_channel_control)
    {   
    case EXC_CHANNEL_CTRL_NIL:
      break;
    case EXC_CHANNEL_CTRL_RANGE:
      *buf++ = chan_sched->bs_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges;
      for (ii = 0; ii < chan_sched->bs_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges; ii++)
      {
        memcpy (buf, &chan_sched->bs_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[ii].start_ch, 2);
        buf += 2;
        memcpy (buf, &chan_sched->bs_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[ii].end_ch, 2);
        buf += 2;  
      }
      break;
    case EXC_CHANNEL_CTRL_BITMASK:
      memcpy (buf, chan_sched->bs_schedule.excluded_channels.excluded_channel_mask, 8);
      buf += 8;
      break;
    default:
      break;
    }
  }
  
 else if((mac_self_fan_info.pan_metrics.parent_bs_ie_use == 0x01)
          &&(get_node_type() == 0x01))
  {

#if APP_LBR_ROUTER
    
#if(FAN_EAPOL_FEATURE_ENABLED == 1)
    mac_nbr_descriptor_t *p_nbr_desc = (mac_nbr_descriptor_t *)
      get_nbr_desc_from_addr(eapol_parent_child_info.sle_eapol_parent);
#else
     mac_nbr_descriptor_t *p_nbr_desc = NULL;
    // TO Do : Raka .. what to do ? [ Sept - 29 -2023 ]
#endif
    
    if (p_nbr_desc == NULL)
      return 0;
   
    memcpy (buf, &p_nbr_desc->bcl_sched.bcast_interval, 4);
    buf += 4;
    memcpy (buf, &p_nbr_desc->bcl_sched.bcast_sched_id, 2);
    buf += 2;
    *buf++ = p_nbr_desc->bcl_sched.bs_schedule.dwell_interval;
    *buf++ = p_nbr_desc->bcl_sched.bs_schedule.clock_drift;
    *buf++ = p_nbr_desc->bcl_sched.bs_schedule.timing_accuracy;
    *buf++ = (((p_nbr_desc->bcl_sched.bs_schedule.channel_plan) & 0x07)) |
      ((p_nbr_desc->bcl_sched.bs_schedule.channel_function<<3) & 0x38) |
        ((p_nbr_desc->bcl_sched.bs_schedule.excludded_channel_control << 6) & 0xC0);
    
    switch (p_nbr_desc->bcl_sched.bs_schedule.channel_plan)
    {
    case CHANNEL_PLAN_REG_DOMAIN:
      *buf++ = p_nbr_desc->bcl_sched.un_channel_plan.ch_reg_op.reg_domain;
      *buf++ = p_nbr_desc->bcl_sched.un_channel_plan.ch_reg_op.op_class;
      break;
    case CHANNEL_PLAN_APP_SPEC:
      memcpy (buf, (uint8_t*)&p_nbr_desc->bcl_sched.un_channel_plan.ch_explicit.ch0, 3);
      buf += 3;                                              
      memcpy (buf, (uint8_t*)&p_nbr_desc->bcl_sched.un_channel_plan.ch_explicit.channel_spacing, 1);
      buf += 1;
      memcpy (buf, (uint8_t*)&p_nbr_desc->bcl_sched.un_channel_plan.ch_explicit.num_chans, 2);
      buf += 2;
      break;
    default:
      break;
    }
    
    switch (p_nbr_desc->bcl_sched.bs_schedule.channel_function)
    {
    case CF_FIXED_CHANNEL:
      put_ushort (buf, p_nbr_desc->bcl_sched.channel_fixed.fixed_chan);
      buf += 2;
      break;
#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
    case CF_TR51:
    case CF_DH1:
      break;
#endif
      
    case CF_VENDOR_SPECIFIC:
      *buf++ = p_nbr_desc->bcl_sched.bs_schedule.chan_hop_count;
      memcpy (buf, p_nbr_desc->bcl_sched.bs_schedule.chan_hop_list,
              p_nbr_desc->bcl_sched.bs_schedule.chan_hop_count);
      buf += p_nbr_desc->bcl_sched.bs_schedule.chan_hop_count;
      break;
    default:
      break;
    }  
    switch (p_nbr_desc->bcl_sched.bs_schedule.excludded_channel_control)
    {
    case EXC_CHANNEL_CTRL_NIL:
      break;
    case EXC_CHANNEL_CTRL_RANGE:
      *buf++ = p_nbr_desc->bcl_sched.bs_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges;      
      for(ii = 0; ii < p_nbr_desc->bcl_sched.bs_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges; ii++)
      {
        memcpy (buf, &p_nbr_desc->bcl_sched.bs_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[ii].start_ch, 2);
        buf += 2;
        memcpy (buf, &p_nbr_desc->bcl_sched.bs_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[ii].end_ch, 2);
        buf += 2;  
      }
      break;
    case EXC_CHANNEL_CTRL_BITMASK:
      memcpy (buf, p_nbr_desc->bcl_sched.bs_schedule.excluded_channels.excluded_channel_mask, 8);
      buf += 8;
      break;
    default:
      break;
    } 
#endif
  }
  
  sub_ie_desc = create_sub_ie_desc (LONG_SUB_IE, WP_IE_SUBIE_SUBID_BS_IE_LONG, 
                                    buf - start - IE_DISCRIPTOR_LEN);
  
  start[0] = sub_ie_desc % 256;
  start[1] = sub_ie_desc / 256;
  
  return buf - start;
}

static uint16_t build_vpie (uint8_t *buf)
{
  uint8_t *start = buf;
  uint16_t sub_ie_desc = 0;
  
  buf += IE_DISCRIPTOR_LEN;
  *buf++ = 0xAA;
  
  sub_ie_desc = create_sub_ie_desc (LONG_SUB_IE, WP_IE_SUBIE_SUBID_VP_IE_LONG, 
                                    buf - start - IE_DISCRIPTOR_LEN);
  
  start[0] = sub_ie_desc % 256;
  start[1] = sub_ie_desc / 256;
  
  return buf - start;
}

static uint16_t build_panie (uint8_t *buf)
{
  uint8_t *start = buf;
  uint8_t fan_var = 0x00;
  uint16_t sub_ie_desc = 0;
  
  buf += IE_DISCRIPTOR_LEN;
  
  memcpy (buf, (uint8_t*)&mac_self_fan_info.pan_metrics.pan_size, 2);
  buf += 2;
  memcpy (buf, (uint8_t*)&mac_self_fan_info.pan_metrics.routing_cost, 2);
  buf += 2;
  fan_var |= mac_self_fan_info.pan_metrics.fan_tps_version<<5;
  *buf++ = ((mac_self_fan_info.pan_metrics.parent_bs_ie_use & 0x01) |
          ((mac_self_fan_info.pan_metrics.routing_methood << 1) & 0x02) |
            (fan_var & 0xE0));
  
  sub_ie_desc = create_sub_ie_desc (SHORT_SUB_IE, WP_IE_SUBIE_SUBID_PAN_IE_SHORT, 
                                    buf - start - IE_DISCRIPTOR_LEN);
  
  start[0] = sub_ie_desc % 256;
  start[1] = sub_ie_desc / 256;
  
  return buf - start;
}

static uint16_t build_netnameie (uint8_t *buf)
{
  uint8_t *start = buf;
  uint16_t sub_ie_desc = 0;
  
  buf += IE_DISCRIPTOR_LEN;

  memcpy (buf, mac_self_fan_info.net_name, mac_self_fan_info.net_name_length);
  buf += mac_self_fan_info.net_name_length;
  
  sub_ie_desc = create_sub_ie_desc (SHORT_SUB_IE, WP_IE_SUBIE_SUBID_NETNAME_IE_SHORT, 
                                    buf - start - IE_DISCRIPTOR_LEN);
  
  start[0] = sub_ie_desc % 256;
  start[1] = sub_ie_desc / 256;
  
  return buf - start;
}

static uint16_t build_panverie (uint8_t *buf)
{
  uint8_t *start = buf;
  uint16_t sub_ie_desc = 0;
  
  buf += IE_DISCRIPTOR_LEN;
  
  /*current value of the PAN Version disseminated by the PAN Border Router*/
  memcpy (buf, (uint8_t*)&mac_self_fan_info.pan_ver, 2);
  buf += 2;
  
  sub_ie_desc = create_sub_ie_desc (SHORT_SUB_IE, WP_IE_SUBIE_SUBID_PANVER_IE_SHORT, 
                                    buf - start - IE_DISCRIPTOR_LEN);
  
  start[0] = sub_ie_desc % 256;
  start[1] = sub_ie_desc / 256;
  
  return buf - start;
}

static uint16_t build_gtkhashie (uint8_t *buf)
{
  uint8_t *start = buf;
  uint16_t sub_ie_desc = 0;
  
  buf += IE_DISCRIPTOR_LEN;
  
  /*GTK0 Hash, GTK1 Hash, GTK2 Hash, and GTK3 Hash are 64 bit unsigned integers 
  which MUST be set to the low order 64 bits of the SHA256 hash of the respective GTK*/
  memcpy (buf, (uint8_t*)&mac_self_fan_info.gtk_hash_ele.gtk0_hash[0], 8);
  buf += 8;
  memcpy (buf, (uint8_t*)&mac_self_fan_info.gtk_hash_ele.gtk1_hash[0], 8);
  buf += 8;
  memcpy (buf, (uint8_t*)&mac_self_fan_info.gtk_hash_ele.gtk2_hash[0], 8);
  buf += 8;
  memcpy (buf, (uint8_t*)&mac_self_fan_info.gtk_hash_ele.gtk3_hash[0], 8);
  buf += 8;
  
  sub_ie_desc = create_sub_ie_desc (SHORT_SUB_IE, WP_IE_SUBIE_SUBID_GTKHASH_IE_SHORT, 
                                    buf - start - IE_DISCRIPTOR_LEN);
  
  start[0] = sub_ie_desc % 256;
  start[1] = sub_ie_desc / 256;
  
  return buf - start;
}

static uint16_t build_unknown_sub_pld_ie (uint8_t *buf)
{
  uint8_t *start = buf;
  uint16_t sub_ie_desc = 0;
  
  buf += IE_DISCRIPTOR_LEN;
  
  *buf++ = 0x01;
  *buf++ = 0x11;
  *buf++ = 0x22;
  *buf++ = 0x33;
  
  sub_ie_desc = create_sub_ie_desc (SHORT_SUB_IE, UNKNOWN_SUB_PLD_IE_SHORT, 
                                    buf - start - IE_DISCRIPTOR_LEN);
  
  start[0] = sub_ie_desc % 256;
  start[1] = sub_ie_desc / 256;
  
  return buf - start;
}

static uint16_t build_mpxie (uint8_t *buf, struct mpx_data_st *mpx_data)
{
  uint8_t *start = buf;
  uint16_t ie_desc = 0;
  
  buf += IE_DISCRIPTOR_LEN;
  
  *buf++ = mpx_data->transfer_type;
  memcpy(buf, (uint8_t*)&mpx_data->multiplex_id, 2);
  buf += 2;
  
  if(mpx_data->multiplex_id == 0x0001)
  {
    *buf++ = mpx_data->kmp_id;
    memcpy (buf, mpx_data->msdu, mpx_data->msdu_len);
  }
  else
    memcpy (buf, mpx_data->msdu, mpx_data->msdu_len);
  buf += mpx_data->msdu_len;
  
  ie_desc = create_ie_desc (PAYLOAD_IE_TYPE, MPX_IE, 
                            buf - start - IE_DISCRIPTOR_LEN);
  
  start[0] = ie_desc % 256;
  start[1] = ie_desc / 256;
  
  return buf - start;
}

uint16_t build_ie (uint8_t *buf, uint8_t type, uint32_t ie_bitmap, 
                   uint32_t sub_ie_bitmap, void *data)
{
  uint8_t *base = buf;
  
  if (type == HEADER_IE_TYPE)
  {
    if (ie_bitmap & WH_IE_MASK)
    {
#if(FAN_EDFE_FEATURE_ENABLED == 1)
      if (sub_ie_bitmap & FC_IE_MASK)
        buf += build_fcie (buf);
#endif
      if (sub_ie_bitmap & UTT_IE_MASK)
        buf += build_uttie (buf);
      if (sub_ie_bitmap & BT_IE_MASK)
        buf += build_btie (buf);
      if (sub_ie_bitmap & RSL_IE_MASK)
        buf += build_rslie (buf);
      if (sub_ie_bitmap & MHDS_IE_MASK)
        buf += build_mhdsie (buf);
      if (sub_ie_bitmap & VH_IE_MASK)
        buf += build_vhie (buf);
      if (sub_ie_bitmap & EA_IE_MASK)
        buf += build_eaie (buf);
      if (sub_ie_bitmap & UNKNOWN_SUB_HDR_MASK)
        buf += build_unknown_sub_hdr_ie (buf);
    }
    if (ie_bitmap & HEADER_TIE1_MASK)
    {
      uint16_t ie_desc = create_ie_desc (0, HIE_TERMINATION_IE1, 0); 
      *buf++ = ie_desc % 256;
      *buf++ = ie_desc / 256;
    }
    if (ie_bitmap & HEADER_TIE2_MASK)
    {
      /*TO-DO*/
    }
  }
  else
  {
    if (ie_bitmap & WP_IE_MASK)
    {
      uint16_t ie_desc = 0;
      
      buf += IE_DISCRIPTOR_LEN;
      if (sub_ie_bitmap & US_IE_MASK)
        buf += build_usie (buf);
      if (sub_ie_bitmap & BS_IE_MASK)
        buf += build_bsie (buf);
      if (sub_ie_bitmap & VP_IE_MASK)
        buf += build_vpie (buf);
      if (sub_ie_bitmap & PAN_IE_MASK)
        buf += build_panie (buf);
      if (sub_ie_bitmap & NETNAME_IE_MASK)
        buf += build_netnameie (buf);
      if (sub_ie_bitmap & PAN_VER_IE_MASK)
        buf += build_panverie (buf);
      if (sub_ie_bitmap & GTK_HASH_IE_MASK)
        buf += build_gtkhashie (buf);
      if (sub_ie_bitmap & UNKNOWN_SUB_PLD_MASK)
        buf += build_unknown_sub_pld_ie (buf);
      
      ie_desc = create_ie_desc (PAYLOAD_IE_TYPE, WP_IE, 
                                buf - base - IE_DISCRIPTOR_LEN);
      base[0] = ie_desc % 256;
      base[1] = ie_desc / 256;
    }
    if (ie_bitmap & MLME_IE_MASK)
    {
      /*TO-DO*/
    }
    if (ie_bitmap & MPX_IE_MASK)
    {
      struct mpx_data_st *mpx_data = (struct mpx_data_st *)data;
      buf += build_mpxie (buf, mpx_data);
    }
    if (ie_bitmap & PAYLOAD_TIE_MASK)
    {
      uint16_t ie_desc = create_ie_desc (PAYLOAD_IE_TYPE, PIE_TERMINATION, 0); 
      *buf++ = ie_desc % 256;
      *buf++ = ie_desc / 256;
    }
  }
  
  return buf - base;
}

static ushort create_sub_ie_desc(  uchar type, uchar sub_ie_id,  ushort content_len )
{
  ushort sub_ie_desc = 0;
  uint16_t val = sub_ie_id;
  
  if(!type) // short
  {
    /*put the length field*/
    sub_ie_desc = (uint8_t)content_len;		
    sub_ie_desc |= (ushort) ( ( val & 0x007F )  << SHORT_SUB_IE_LENGTH_FLD_LEN_IN_BITS );		
  }
  else // long
  {
    sub_ie_desc = content_len & 0x07FF;
    sub_ie_desc |= 0x8000;
    sub_ie_desc |= (ushort) ( ( val & 0x000F )  << LONG_SUB_IE_LENGTH_FLD_LEN_IN_BITS );		
  }	
  return 	sub_ie_desc;			
}

static ushort create_ie_desc(uchar type,uchar ie_id, ushort content_len )
{
  ushort ie_desc = 0;
  uint16_t val = ie_id;
  if( !type ) //hdr IE
  {
    if ( ie_id  != 0x7E)
      content_len += 1;// to incude the sib id also which will be part of the content
    
    ie_desc = (uint8_t)(content_len & 0x7F);		
    ie_desc |= (ushort) ( ( val & 0x00FF )  << HDR_IE_LENGTH_FLD_LEN_IN_BITS );
    ie_desc |= (ushort)( ( type & 0x01 ) << ( HDR_IE_LENGTH_FLD_LEN_IN_BITS + HDR_IE_ID_FLD_LEN_IN_BITS ) );		
  }
  else //pld IE
  {
    ie_desc = (content_len & 0x07ff);		
    ie_desc |= 0x8000;		
    ie_desc |= (ushort)( (val & 0x000f) << PLD_IE_LENGTH_FLD_LEN_IN_BITS );		
  }	
  return ie_desc;
}


/******************************************************************************/

/* Not used in FAN */
ushort build_ie_list(
                     uchar* buf,
                     uchar type, 
                     uchar ie_list_len, 
                     uchar* p_ie_ids_list,
                     uchar ie_flags 
                       )
{
  return 0;
}
bool any_ie_supported(
                        uchar type,
                        ushort num_of_ids,
                        uchar* p_ids_list
                       )
{
	return false;
}
uchar extract_mlme_pld_ie(
                              uchar sub_ie_type,
                              uchar sub_ie_id,	
                              uchar payloadIEListLen,	 
                              uchar *payloadIEList,
                              ushort payloadIEFieldLen,
                              uchar* p_out,
                              ushort* p_out_len
                           )
{	
	return 0;
} 
#endif