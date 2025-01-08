/***************************************************************************//**
 * @file
 * @brief main.c
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include "common.h"
#include "StackAppConf.h"
#include "sl_component_catalog.h"
#include "sl_system_init.h"

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  #include "sl_power_manager.h"
#endif

#include "app_init.h"
#include "app_process.h"

#if defined(SL_CATALOG_KERNEL_PRESENT)
  #include "sl_system_kernel.h"
  #include "app_task_init.h"
#else // SL_CATALOG_KERNEL_PRESENT
  #include "sl_system_process_action.h"
#endif // SL_CATALOG_KERNEL_PRESENT

#include "em_rmu.h"
#include "em_wdog.h"

/*********************SW utility header file **********************************/
#include "uart_hal.h"
#include "list_latest.h"
#include "queue_latest.h"
#include "buff_mgmt.h"
#include "buffer_service.h"
#include "event_manager.h"
#include "sw_timer.h"
#include "timer_service.h"
#include "hif_utility.h"
#include "hif_service.h"

/*********************MAC header file *****************************************/
#include "fan_mac_ie.h"
#include "mac_interface_layer.h"
#include "mac.h"
#include "sm.h"
#include "fan_mac_interface.h"
#include "ie_element_info.h"
#include "network-manager.h"
/*********************Application header file *********************************/
#include "fan_app_test_harness.h"
#include "fan_app_auto.h"
#include "fan_api.h"

#if(APP_HIF_PROCESS_FEATURE_ENABLED == 1)   
#include "hif_process.h"
#endif
   
#include "fan_factorycmd.h"
/********************Contiki Header file***************************************/
#include "contiki-net.h"
#include "contiki_mac_interface.h"
/*********************Layer Interface header file ******************************/
   
#include "em_cmu.h"
#include "em_gpio.h"
#include "app_log.h"
#include "sl_app_log.h"

#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 1)
#include "ext_FLASH_app.h"
#endif
   
#include "nvm3_default.h"
#include "sl_iostream_init_eusart_instances.h"
#include "sl_iostream_init_instances.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

#define NVM3_DEFAULT_HANDLE_1 nvm3_defaultHandle

#define SL_RAIL_UTIL_PA_POWER_DECI_DBM 100
#include "em_timer.h"
unsigned long resetCause;
volatile uint32_t startTicks = 0;
volatile uint32_t endTicks = 0;
volatile bool timerExpired = false;
void TIMER2_IRQHandler();
void timer_2_init(void);
uint16_t timer_2_cout = 0;
int data;
uint32_t key_1 = 1;
uint16_t counter =0;



L3_PROCESS_NAME(etimer_process);
L3_PROCESS_NAME(udp_process);

L3_PROCESS_NAME(ping6_process);

#if(APP_NWK_MANAGER_PROCESS_FEATURE_ENABLED == 1)
L3_PROCESS_NAME(nwk_manager_process);
#endif

#if ENABLE_DISABLE_DHCP_SERVICE 
L3_PROCESS_NAME(dhcpv6_process);
#endif

L3_PROCESS_NAME(simple_udp_process);
L3_PROCESS_NAME(trickle_protocol_process);


#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
L3_PROCESS_NAME(freq_hopping_process);
#endif

#if(APP_HIF_PROCESS_FEATURE_ENABLED == 1)
L3_PROCESS_NAME(hif_process);
#endif

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

void initWDOG(void);
void clean_mac_neighbour_table ();
void delete_all_device_from_mac_pib (void);
uint8_t root_device=0;
uint32_t disable_int_count = 0;
uint32_t enable_int_count = 0;
hif_t hif = {0};
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
extern uint8_t cold_boot_flag;
extern uint8_t response_laye_ID;
extern linkaddr_t linkaddr_node_addr;
extern fan_nwk_manager_sm_t fan_nwk_manager_app;
extern fan_mac_information_sm_t fan_mac_information_data;
extern white_list_t white_mac_list;
// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
//#if !defined(SL_CATALOG_KERNEL_PRESENT)
///// A static handle of a RAIL instance
//static RAIL_Handle_t rail_handle;
//#endif
#if ENABLE_DISABLE_DHCP_SERVICE 
extern void server_service_init();
#endif

extern void initialize_rpl (void);
extern uint8_t send_hif_conf_cb( uint8_t cmd_id,uint8_t status );
extern void configure_device_run_param();

void test_UART_write ();
int main_testApp_wrapper( void );
void store_global_address();
uint8_t global_addr_device[16];
// GLOBAL VARIABLES 
unsigned long resetCause;
uint8_t TANSIT_KMP_ID = 0x00;
uint32_t processor_active;
extern uint8_t rx_buffer[];
void cca_sample( void );
void APP_Task( void );
sw_tmr_t uhf_tmr; 
extern void fan_nwk_manager_init( );

void mem_reverse_cpy(uint8_t* dest, uint8_t* src, uint16_t len );
#ifdef MAC_ADDRESS_FILTERING_ENABLE
/*Umesh added here for mac filtering*/
typedef struct filter_addr
{
  uint8_t mac_addr[8];
} filter_addr_t ;
/*----------------------------------------------------------------------------*/
filter_addr_t address_filter_list[10];   // used only for testing to adding filter
/*----------------------------------------------------------------------------*/
#endif

#ifdef MAC_ADDRESS_FILTERING_ENABLE
#if(AUTO_CONFIG_ENABLE == 1)
void add_mac_filter();
#endif
#endif
//This function is a callback functionn to be called by Network manager once the
//network manager is in ready state. the action required in this function is to 
//be defined by upper layer.
void start_upper_layer(uint8_t* lladdress)
{   
  /* initialize the netstack */
  netstack_init();
  mem_reverse_cpy(linkaddr_node_addr.u8, lladdress, sizeof(uip_lladdr.addr));    
  memcpy(uip_lladdr.addr, &linkaddr_node_addr, sizeof(uip_lladdr.addr)); 
  /*buffer queue Initialisation*/
  queuebuf_init();
 
  /*stating TCP-IP Process*/
  l3_process_start(&tcpip_process, NULL);  
  /*stating PING Process*/
  l3_process_start(&ping6_process, NULL); 
  /*stating UDP Process*/
  l3_process_start(&udp_process, NULL);
  /*stating DHCPv6 Process*/
#if ENABLE_DISABLE_DHCP_SERVICE  
  l3_process_start(&dhcpv6_process, NULL);
#endif
  
  /*stating EAPOL Process*/
#if ( FAN_EAPOL_FEATURE_ENABLED == 1)                   //!(WITHOUT_EAPOL) //Umesh : used this macro for temp purpose
  l3_process_start(&eapol_relay_process, NULL);
#endif  

  /*Server Service Initialisation*/
#if ((EFM32GG512 == 1) || (EFR32FG13P_LBR == 1)) && (HIF_INTERFACE_FOR_LINUX_RUNNING_DHCP == 0)
#if ENABLE_DISABLE_DHCP_SERVICE  
  server_service_init();
#endif         
#endif  
  initialize_rpl ();
  
  if( fan_nwk_manager_app.node_basic_cfg.fan_device_type == PAN_COORD_NODE_TYPE )
  {
      root_device = 1;
      cold_boot_flag = 0;//only using incase of router
      rpl_dag_root_init_dag();
      send_hif_conf_cb(NODE_START_STOP_CONF,0x00);
#if(APP_NVM_FEATURE_ENABLED == 1)
      nvm_store_node_basic_info();   //Debdeep
#endif
  }
#if (EFR32FG13P_LBR == 0x00)   
    if((fan_mac_information_data.state_ind == JOIN_STATE_5)
     &&(fan_mac_information_data.fan_node_type != PAN_COORD_NODE_TYPE)
#if(APP_NVM_FEATURE_ENABLED == 1)
       &&(fan_mac_information_data.is_start_from_nvm == true)
#endif
         )
  {
    #if(APP_NVM_FEATURE_ENABLED == 1)
    if(fan_nwk_manager_app.nvm_write_to_start ==true)
    {
      rpl_instance_t *instance;
      store_global_address();  //suneet :: when device boot up from join state 
      nvm_load_link_stats_info();
      nvm_load_ds6_info();
      nvm_load_rpl_dio_info();
      instance = rpl_get_instance(instance_table[0].instance_id);
      rpl_set_default_instance(instance);
      nvm_store_node_basic_info();   

    }
    #endif
  }
#endif  
}





// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
/******************************************************************************
 * Main function
 *****************************************************************************/
 int main(void)
{
  // Initialize Silicon Labs device, system, service(s) and protocol stack(s).
  // Note that if the kernel is present, processing task(s) will be created by
  // this call.
  sl_system_init();
   
  sl_iostream_eusart_init_instances();
  
   //App_log Initilization
  app_log_init();
    // app_log("***********");  //For app Logs
     //or
      // sl_app_log("****************");
  
  
  //NVM3 Initilisation
  nvm3_initDefault();
  
   /* Store the cause of the last reset, and clear the reset cause register */
  resetCause = RMU_ResetCauseGet();

    /* Clear Reset causes so we know which reset matters the next time */
  RMU_ResetCauseClear();
  
  //Resetting by WatchDog
  if(resetCause==8)
  {
    printf("Reseting the system by WatchDog timer\r\n");
  }
  
   //Resetting by Normal Reset
  if(resetCause==64)
  {
    printf("Reseting the system by Normal Reset\r\n");
  }
  nvm3_readData(NVM3_DEFAULT_HANDLE_1, key_1, &data, 1);
  counter = data;
  counter++;
     //counter=0;
  nvm3_writeData(NVM3_DEFAULT_HANDLE_1,
                                      key_1,
                                     &counter,
                                      1);
  
    /*RTC Initialisation*/
    rtc_init();

    /*Buffer Service Intitialization*/
    buffer_service_init();
    
    /* HIF Interface Initialization*/
    hif_service_init(&hif);    
    APPhifForToolTest_Init(); 
    
     /* Timer Service Initialization*/
    tmr_service_init();
 
    /* initialize process manager. */
    process_init();    
  /*Cloclk intialisation*/
    clock_init();    
   initWDOG();
   
    
   
#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 1)    
    //flash init
//    MX25_init();
    //FlashReadWrite_Test();  
#endif   
    
#if(APP_HIF_PROCESS_FEATURE_ENABLED == 1)    
    /*Start The HIF Process*/
    l3_process_start(&hif_process, NULL);
#endif
    
    /*Start Etimer Process*/
    l3_process_start(&etimer_process, NULL);
    /*Initialising CTimer*/
    ctimer_init();
    /*Timers get initialized here : promac_process*/
    l3_process_start(&promac_process, NULL);
    
#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
    /*stating freq_hopping Process*/
    l3_process_start(&freq_hopping_process, NULL);    
#endif
    
#if(APP_NWK_MANAGER_PROCESS_FEATURE_ENABLED == 1)
    /*Start N/W manager Process*/
    l3_process_start(&nwk_manager_process, NULL);    
#endif
    
    APP_LED_GPIO_init ();
    
     //LED2 Initilisation
    APP_LED_2_GPIO_init ();
   // check_status_to_start_network();   //suneet :: status for boot from join state 5
    
#ifdef MAC_ADDRESS_FILTERING_ENABLE
#if(AUTO_CONFIG_ENABLE == 1)
    /*Adding mac adress for filtering*/
    add_mac_filter();
#endif    
#endif
    APP_LED_ON ();
    if((fan_mac_information_data.state_ind != JOIN_STATE_5)
#if(APP_NVM_FEATURE_ENABLED == 1)
       &&(fan_mac_information_data.is_start_from_nvm != true)
#endif
         )
    {
        /*starting auto run mode */
          configure_device_run_param();
    }

    if(fan_nwk_manager_app.node_basic_cfg.board_reset == 0x01)
    {
      fan_nwk_manager_app.node_basic_cfg.board_reset = 0x00;
#if(APP_NVM_FEATURE_ENABLED == 1)
      nvm_store_node_basic_info();
#endif
    }
    
    // Start the Network manager ...
    if((fan_nwk_manager_app.node_basic_cfg.operational_mode == RUN_MODE)
#if(APP_NVM_FEATURE_ENABLED == 1)
       &&(fan_mac_information_data.is_start_from_nvm == false)
#endif
         )
    {
        fan_nwk_manager_init ();
    }
  
    
     
    //setupSWOForPrint();
    //stack_print_debug ("...\n");
  while (1) {
    
    // Do not remove this call: Silicon Labs components process action routine
    // must be called from the super loop.
    sl_system_process_action();
    // Application process.

  
  WDOGn_Feed(DEFAULT_WDOG);
  
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    // Let the CPU go to sleep if the system allows it.
    sl_power_manager_sleep();
#endif
    
  }
  

}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

// time set for is 2 Min 

void initWDOG(void)
{
 // Enable clock for the WDOG module; has no effect on xG21
  CMU_ClockEnable(cmuClock_WDOG0, true);

  // Watchdog Initialize settings 
  WDOG_Init_TypeDef wdogInit = WDOG_INIT_DEFAULT;
  CMU_ClockSelectSet(cmuClock_WDOG0, cmuSelect_ULFRCO); /* ULFRCO as clock source */
  wdogInit.debugRun = true;
  wdogInit.em3Run = true;
  wdogInit.perSel = wdogPeriod_128k; // 2049 clock cycles of a 1kHz clock  wdogPeriod_2k for ~2 seconds period

  // Initializing watchdog with chosen settings 
  WDOGn_Init(DEFAULT_WDOG, &wdogInit);
  
  NVIC_ClearPendingIRQ(WDOG0_IRQn);
  WDOGn_IntClear(DEFAULT_WDOG, WDOG_IF_WARN);
  NVIC_EnableIRQ(WDOG0_IRQn);
  WDOGn_IntEnable(DEFAULT_WDOG, WDOG_IEN_WARN);
}


#define TIMER3_INIT_DEFAULT                                                            \
  {                                                                                   \
    true,                 /* Enable timer when initialization completes. */           \
    false,                /* Stop counter during debug halt. */                       \
    timerPrescale32,       /* No prescaling. */                                        \
    timerClkSelHFPerClk,  /* Select HFPER / HFPERB clock. */                          \
    false,                /* Not 2x count mode. */                                    \
    false,                /* No ATI. */                                               \
    false,                /* No RSSCOIST. */                                          \
    timerInputActionNone, /* No action on falling input edge. */                      \
    timerInputActionNone, /* No action on rising input edge. */                       \
    timerModeUp,          /* Up-counting. */                                          \
    false,                /* Do not clear DMA requests when DMA channel is active. */ \
    false,                /* Select X2 quadrature decode mode (if used). */           \
    false,                /* Disable one shot. */                                     \
    false,                /* Not started/stopped/reloaded by other timers. */         \
    false                 /* Disable ability to start/stop/reload other timers. */    \
  }


#define TIMER3_INITCC_DEFAULT                                                 \
  {                                                                          \
    timerEventEveryEdge,    /* Event on every capture. */                    \
    timerEdgeRising,        /* Input capture edge on rising edge. */         \
    0,                      /* Not used by default, select PRS channel 0. */ \
    timerOutputActionNone,  /* No action on underflow. */                    \
    timerOutputActionNone,  /* No action on overflow. */                     \
    timerOutputActionNone,  /* No action on match. */                        \
    timerCCModeOff,         /* Disable compare/capture channel. */           \
    false,                  /* Disable filter. */                            \
    false,                  /* No PRS input. */                              \
    false,                  /* Clear output when counter disabled. */        \
    false,                  /* Do not invert output. */                      \
    timerPrsOutputDefault,  /* Use default PRS output configuration. */      \
    timerPrsInputNone       /* No PRS input, so input type is none. */       \
  }

void TIMER2_IRQHandler()
{
   uint16_t intFlags = TIMER_IntGet(TIMER2);
  TIMER_IntClear(TIMER2, TIMER_IF_OF | TIMER_IF_CC0);
  
  /* Overflow interrupt occured */
  if(intFlags & TIMER_IF_OF)
  {
  printf("Rx timer is running\r\n");
    timerExpired = true;
  }
  
}


void timer_2_init(void)
{
CMU_ClockEnable(cmuClock_TIMER2, true);  

  
  TIMER_InitCC_TypeDef timerCCInit = TIMER3_INITCC_DEFAULT;
  timerCCInit.mode = timerCCModePWM;
  TIMER_InitCC(TIMER2, 0, &timerCCInit);
 
   /* Set Top Value */
  TIMER_TopSet(TIMER0, (0xFFFD-1));
  
   TIMER_Init_TypeDef timerInit = TIMER3_INIT_DEFAULT;
  timerInit.prescale = timerPrescale1024;
  TIMER_Init(TIMER2, &timerInit);
  
   TIMER_IntEnable(TIMER2, TIMER_IF_OF ); 
  NVIC_EnableIRQ(TIMER2_IRQn);
}


//void signal_event_to_mac_task( uint8_t prio )
//{
//  
//  
//}

void indicate_mac_2_nhle( void )
{
  /*enable the following code only in mac_test and not in any apps running
  over MIL
  event_set(MAC_2_NHLE_EVENT);
  signal_event_to_mac_task();*/
}



//void nvm_store_node_basic_info( void )
//{
// 
//}


//void nvm_load_read_fan_macself_info(void)
//{
//
//}
//
void reset_to_join_state_1 (void)
{
//  app_bm_free (supp_cred.wpa_sm_ptr);
//  memset (&supp_cred, 0, sizeof(supp_cred));
  delete_all_device_from_mac_pib ();
  clean_mac_neighbour_table ();
  change_join_state (5);
}
//
//void upload_parameter_from_nvm()
//{
//
//}
//void nvm_store_write_fan_join_info(void)
//{
//
//}
//
//void nvm_store_write_fan_macself_info(void)
//{
//}
//
//void nvm_store_write_fan_macsecurity_info(void)
//{
// 
//}
//
//void nvm_store_mac_white_list_info( void )
//{
//
//}
//
//void l3_random_init (unsigned short seed)
//{
//  
//}

//unsigned short l3_random_rand (void)
//{
//  return 0x1122;
//}

//void stack_print_debug(const char *format, ...)
//{
////    va_list vargs;
////    va_start(vargs, format);
////    vprintf(format, vargs);
////    va_end(vargs);
//}


#if(FAN_EDFE_FEATURE_ENABLED == 1)
void enable_disable_edfe_frame(uint8_t value,uint8_t edfe_frame_type)
{
  
}


void send_edfe_initial_frame(uint8_t *src_addr , uint8_t value,uint8_t edfe_frame_type)
{
  
}

#endif // #if(FAN_EDFE_FEATURE_ENABLED == 1)
/*
Raka .. This MACRO has been defined in mac.h and used in 
mac_rcv.c  function : process_pd_2_mac_incoming_frames
*/
#ifdef MAC_ADDRESS_FILTERING_ENABLE 

// Raka Need to check the condition of setting from tool and running in auto mode
#if(AUTO_CONFIG_ENABLE == 0)  

uint8_t validate_filter_mac_address(uint8_t* ieee_address)
{
  uint8_t i = 0;
  if(white_mac_list.wht_mac_index == 0x00)
  {
    return 0;
  }
  else
  {
    for(i=0;i<white_mac_list.wht_mac_index;i++)
    {
      if(!memcmp(&white_mac_list.wht_list_macaddr[i],ieee_address,8))/*To pass pkt fw use(!) before memcmp and if not then remove (!) it*/
        return 0;
    } 
    return 1;
  }
}
#else

/*** note -- filtering of mac address in mac ***/
/*add mac address here for it will accept pkt from device*/
#define MAC_ADDRESS_OF_DEV_0	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xA1 

#define MAC_ADDRESS_OF_DEV_1	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xA2  

#define MAC_ADDRESS_OF_DEV_2	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xA3



//#define MAC_ADDRESS_OF_DEV_3	 0x11,0x22,0x33,0x44,0x00,0x00,0x00,0x03
//#define MAC_ADDRESS_OF_DEV_4	 0x11,0x22,0x33,0x44,0x00,0x00,0x00,0x04
//
//#define MAC_ADDRESS_OF_DEV_5	 0x11,0x22,0x33,0x44,0x00,0x00,0x00,0x05
//#define MAC_ADDRESS_OF_DEV_6	 0x11,0x22,0x33,0x44,0x00,0x00,0x00,0x06
//#define MAC_ADDRESS_OF_DEV_7	 0x11,0x22,0x33,0x44,0x00,0x00,0x00,0x07
//#define MAC_ADDRESS_OF_DEV_8	 0xAA,0xBB,0xCC,0xDD,0x00,0x00,0x00,0x23
//#define MAC_ADDRESS_OF_DEV_9	 0xAA,0xBB,0xCC,0xDD,0x00,0x00,0x00,0x24
//#define MAC_ADDRESS_OF_DEV_10    0xAA,0xBB,0xCC,0xDD,0x00,0x00,0x00,0x25

uint8_t static_filter_index=0;

uint8_t validate_filter_mac_address(uint8_t* ieee_address)
{
  uint8_t i = 0;
  for(i=0;i<static_filter_index;i++)
  {
    if(!memcmp(address_filter_list[i].mac_addr,ieee_address,8))/*To pass pkt fw use(!) before memcmp and if not then remove (!) it*/
        return 0;
  } 
  return 1;
}

void add_address_to_filter_list(uint8_t *filter_addr)  // used only for testing to adding filter
{
  uint8_t temp[8]={0};
  mem_rev_cpy(temp,filter_addr,IEEE_ADDRESS_LENGTH);
    while(static_filter_index<10)
    {
      mem_rev_cpy(address_filter_list[static_filter_index].mac_addr,filter_addr,8);
      static_filter_index++;
      return;
    }  
} 

void add_mac_filter()
{
  uint8_t mac_addr_0[8]={MAC_ADDRESS_OF_DEV_0};
  add_address_to_filter_list(mac_addr_0);
  
  uint8_t mac_addr_1[8]={MAC_ADDRESS_OF_DEV_1};
  add_address_to_filter_list(mac_addr_1);  
  
  uint8_t mac_addr_2[8]={MAC_ADDRESS_OF_DEV_2};
  add_address_to_filter_list(mac_addr_2);
  
  
//  uint8_t mac_addr_3[8]={MAC_ADDRESS_OF_DEV_3};
//  add_address_to_filter_list(mac_addr_3);
//  uint8_t mac_addr_4[8]={MAC_ADDRESS_OF_DEV_4};
//  add_address_to_filter_list(mac_addr_4);
//  uint8_t mac_addr_5[8]={MAC_ADDRESS_OF_DEV_5};
//  add_address_to_filter_list(mac_addr_5);
//  uint8_t mac_addr_6[8]={MAC_ADDRESS_OF_DEV_6};
//  add_address_to_filter_list(mac_addr_6);
//  uint8_t mac_addr_7[8]={MAC_ADDRESS_OF_DEV_7};
//  add_address_to_filter_list(mac_addr_7);
//  uint8_t mac_addr_8[8]={MAC_ADDRESS_OF_DEV_8};
//  add_address_to_filter_list(mac_addr_8);
//  uint8_t mac_addr_9[8]={MAC_ADDRESS_OF_DEV_9};
//  add_address_to_filter_list(mac_addr_9);
//  uint8_t mac_addr_10[8]={MAC_ADDRESS_OF_DEV_10};
//  add_address_to_filter_list(mac_addr_10);
}
#endif 
#endif 
// -----------------------------------------------------------------------------
//                          Function Definitions
// -----------------------------------------------------------------------------

static int debug_reset_count = 0;
extern void rpl_free_instance_and_dag (void);
extern void rpl_cancel_probing_timer (void);
extern void clean_ds6_nbr_table();
extern void clean_rpl_nbr_table (void);
extern void clean_link_stat_nbr_table (void);
extern void reset_pan_timeout_state (void);
extern void process_schedule_end_pa (void);
extern void process_schedule_end_pc (void);


void kill_process_and_clean_rpl_nbr()
{
  fan_mac_information_data.upper_layer_started = 0;
  rpl_free_instance_and_dag ();
  rpl_cancel_probing_timer ();
  //clean_dhcp_credentials ();
  clean_ds6_nbr_table ();
  clean_rpl_nbr_table ();
  clean_link_stat_nbr_table ();
  reset_pan_timeout_state ();
  process_schedule_end_pc ();
  process_schedule_end_pa ();
  
#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
  stop_broadcast_schedule ();
#endif
  
  reset_to_join_state_1 ();
  debug_reset_count++;
}


/******************************************************************************/
#if (EFR32FG13P_LBR == 0x00)  
static uip_ipaddr_t glob_fipaddr;
void store_global_address()
{
  uint8_t *my_global_addr;
  my_global_addr = (uint8_t *)get_self_global_addr();
  memcpy(global_addr_device,my_global_addr,16);  
  memcpy(glob_fipaddr.u8,global_addr_device,16);
  uip_ds6_addr_add(&glob_fipaddr, 0, ADDR_DHCP);
}
#endif