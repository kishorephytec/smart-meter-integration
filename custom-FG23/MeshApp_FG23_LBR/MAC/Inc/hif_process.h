/** \file hif_process.h
 *******************************************************************************
 ** \brief Provides information about the HIF Process 
 **
 ** \cond STD_FILE_HEADER
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


#ifndef CONTIKI_HIF_PROCESS_H
#define CONTIKI_HIF_PROCESS_H

#if(APP_HIF_PROCESS_FEATURE_ENABLED == 1)
#ifdef __cplusplus
extern "C" {
#endif


void hif_process_init (void *data);
void hif_process_tx (void *data);
void hif_process_rx (void *data);

enum {
  HIF_PROCESS_INIT,
  HIF_PROCESS_TX,
  HIF_PROCESS_RX,
};


#ifdef __cplusplus
}
#endif

#endif

#endif /* CONTIKI_HIF_PROCESS_H */