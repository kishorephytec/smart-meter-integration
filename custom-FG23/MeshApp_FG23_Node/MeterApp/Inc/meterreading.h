/** \file meterreading.h
 *******************************************************************************
 ** \brief Implements the UART Queue required functions.
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

/*******************************************************************************
* File inclusion
*******************************************************************************/

#ifndef _METERREADING_H
#define _METERREADING_H
/*******************************************************************************/
typedef struct EIC_metere_data_hdr_struct
{
		uint8_t         slaveId;  // Slave ID ix 0x01  
		uint8_t         fcode;		//Functional code [03 (0x03) Read Holding Registers]
		uint8_t	      data_len;	//data length of the response received form Meter
}EIC_metere_data_hdr_struct_t;
/*******************************************************************************/
typedef struct  EIC_Meter_Holding_Register_Data_struct
{             
     uint8_t current_datetime[12];
     uint8_t ir[4]; 
     uint8_t iy[4]; 
     uint8_t ib[4];  
     uint8_t vr[2];  
     uint8_t vy[2]; 
     uint8_t vb[2]; 
     uint8_t pfr[2];  
     uint8_t pfy[2]; 
     uint8_t pfb[2];  
     uint8_t pf_total[2];  
     uint8_t frequency[2]; 
     uint8_t kva_total[4]; 
     uint8_t kw_total[4]; 
     uint8_t kvar_total[4] ; 
     uint8_t cum_kwh_total[4]; 
     uint8_t cum_kvar_lagh_total[4] ;
     uint8_t cum_kvar_leadh_total[4] ; 
     uint8_t cum_kvah_total[4] ; 
     uint8_t num_power_offs[4] ;
     uint8_t cum_power_off_duration[4]; 
     uint8_t cum_tamper_count[4] ;
     uint8_t cum_md_reset_count[4]; 
     uint8_t cum_programming_count[4];
     uint8_t cum_last_md_event_datetime[12] ;
     uint8_t kw_max_demand [4] ; 
     uint8_t kva_max_demand[4];    
} EIC_Meter_Holding_Data_struct_t;
/*******************************************************************************/
//[Kimbal]
//uint8_t hif2MeterCommunicationInterface( uint8_t* pBuff,uint16_t len);
void meter_data_wr_for_read(uint8_t *buff, uint8_t length);
     
/*******************************************************************************/       
enum {
  APP_EV_UART_DATA_RCVD,
  APP_EV_UART_DATA_SEND_OVER_UDP,
  APP_EV_UART_DATA_SEND,
  APP_EV_RECVD_UDP_SEND_TO_UART,
  APP_EV_DUMMY
};

#endif