/** \file nvm_app.h
 *******************************************************************************
 ** \brief This file has the function to store and retrieve data from NVM
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

#ifndef NVM_APP_API_H
#define NVM_APP_API_H

#if(APP_NVM_FEATURE_ENABLED == 1)

#ifdef __cplusplus
extern "C" {
#endif

void init_nvm(void);
void check_status_to_start_network();
void nvm_load_read_node_basic_info( void );
void nvm_store_node_basic_info( void );
void nvm_load_read_mac_nbr(void);
void nvm_store_write_mac_nbr();
void nvm_load_read_fan_join_info(void);
void nvm_store_write_fan_join_info(void);
void nvm_load_read_fan_macself_info(void);
void nvm_store_write_fan_macself_info(void);
void nvm_load_read_fan_macsecurity_info(void);
void nvm_store_write_fan_macsecurity_info(void);
void nvm_load_read_fan_device_desc_info(void);
void nvm_store_write_fan_device_desc_info();
void nvm_load_mac_frame_counter(void);
void nvm_store_write_mac_frame_counter(void);
void nvm_load_ds6_info(void);
void store_l3_data_after_join_state_5();
void nvm_load_rpl_dio_info(void);
//void nvm_store_dao_info(rpl_dag_t *dag,const uip_ipaddr_t *child_address, const uip_ipaddr_t *parents_address);
void nvm_load_mac_white_list_info( void );
void nvm_store_mac_white_list_info( void );
void update_nvm_parameter();
void upload_parameter_from_nvm();
void rpl_update_info_from_nvm();
void format_nvm() ;
  
#ifdef __cplusplus
}
#endif

#endif //#if(APP_NVM_FEATURE_ENABLED == 1)

#endif /* NVM_APP_API_H */
