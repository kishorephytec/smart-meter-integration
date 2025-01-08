/** \file sw_timer.h
 *******************************************************************************
 ** \brief Software Timer public header.
 **   
 ** This file contains the public APIs and structures of Software Timer
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
 
#ifndef _SW_TIMER_H_
#define _SW_TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "list_latest.h"
  //for "p3_list_t"
  
/**
 *******************************************************************************
 * @ingroup sysdoc
 *
 * @{
 ******************************************************************************/

/**
 *******************************************************************************
 * @defgroup Swtmr Software Timer Module
 * @brief This module provides a number of software timers based on a single 
 * hardware timer.
 *
 *    The software timer module provides multiple timers using a single  
 *    hardware timer. It generates interrupts only when a software timer  
 *    expires or hardware timer roundup happens.
 *   
 *    One can run multiple instances of software timers in the same system. 
 *    Each instance can be tied to a particular HW timer block.
 *
 *    User can decide on the type of stime_t type used to define the time. Here
 *    stime_t stands for system time. For that one has to update the following
 *    MACROS:-\n
 *            - sw_tmr_is_stime_greater\n
 *            - sw_tmr_stime_copy\n
 *            - sw_tmr_stime_add\n
 *            - sw_tmr_stime_sub\n
 *
 *    To use any of the software timer functionality user has to initialize 
 *    the sw timer initializing all the required hardware timer function 
 *    pointers in hw_timer_if_t structure of sw_tmr_module_t structure. The
 *    software timer module, handles the hw timer expire in a thread context
 *    which is expected to be the highest priority thread in the system.\n
 *
 *    The following steps describes how to use the software timer.\n
 *    (1) create and initialize a software timer instance\n
 *    (2) Then start the software timer. \n
 *    (3) Once time timer expires, the callback function passed during software 
 *        timer creation is invoked from the timer thread context. \n
 *    (4) The callback is expected to be executed with few cpu cycle like
 *        in an ISR. \n
 *    (5) This will ensure the timing for other sotware timers are 
 *        maintained properly.\n
 *
 * This module will be running as a thread which can take the folling events.\n
 * 1) event to start a software timer\n
 * 2) event to stop a runing software timer\n
 * 3) event to indicate expiry of a running software timer\n
 *
 * As part of processing of the start event the timer module thread adds a new 
 * software timer to the list of running timers. The list is arranged in the 
 * ascending order of the expiry time. The thread picks up the first element 
 * from the list to start the hardware timer.\n
 * As part of the processing of the stop event, the timer module thread will 
 * delete the indicated timer instance from the running timer list.
 * When the timer module thread gets a  timer expiry event, it checks all the 
 * running timers and see if they have expired and if so, invoke the respective 
 * call back functions registered during the software timer creation.
 *        
*******************************************************************************/

/**
 *******************************************************************************
 * 
 * @}     
 ******************************************************************************/

/*
** ============================================================================
** Public Macro definitions
** ============================================================================
*/

/**
 ** \defgroup sw_timer_defs  Software Timer Definitions
 ** \ingroup Swtmr
 */

/*@{*/
 
/*! Indicates a software timer is just created  */
#define SW_TMR_CREATED 1

/*! Indicates that a software timer is active/running  */
#define SW_TMR_ACTIVE  2

/*! Indicates that a software timer is expired  */
#define SW_TMR_EXPIRED 3

/*! Indicates that a software timer is stopped */
#define SW_TMR_STOPPED 4

/*! Indicates that the software timer was requested to be stopped from timer 
*    interrupt context */
//#define SW_TMR_ISR_REQ_STOP  1

/*! Indicates that the software timer was requested to be start from timer 
*    interrupt context */
//#define SW_TMR_ISR_REQ_START 2

/*! defining the greater time */
#define sw_tmr_is_stime_greater( time1, time2 ) time1 > time2
  
/*! assigns the time from time2 to time1 */
#define sw_tmr_stime_copy( time1, time2 ) time1 = time2;

/*! adds and assigns the added time to time1*/
#define sw_tmr_stime_add( time1, time2, time3 ) (time1) = (time2) + (time3);

/*! subtracts and assigns the time to time1. */
#define sw_tmr_stime_sub( time1, time2, time3 ) (time1) = (time2) - (time3);	


#define RAIL_TIMER_INTERFACE_USED    1  
  
/*
** ============================================================================
** Public Structures, Unions & enums Type Definitions
** ============================================================================
*/
	
/**
 *******************************************************************************
 * @enum Soft Timer Type.
 *    enum for soft timer timer type.
 ******************************************************************************/

#if defined (RL78_TARGET_CUBESUITE) || defined (RL78_TARGET_IAR) || defined (EFM32_TARGET_IAR)|| defined (USE_STM32L4XX_NUCLEO)

typedef enum sw_tmr_type_struct
{
    SW_TMR_ONESHOT,
    SW_TMR_PERIODIC
}sw_tmr_type_t;
#else

typedef  unsigned char  sw_tmr_type_t;
#define SW_TMR_ONESHOT			0
#define SW_TMR_PERIODIC			1
#endif


/**
 *******************************************************************************
 * @enum Soft Timer Type.
 *    enum for soft timer timer error type.
 ******************************************************************************/
typedef enum 
{
    SW_TIMER_SUCCESS,
    SW_TIMER_ERROR
} sw_tmr_error_t;

/* Function prototypes */



/**
 *******************************************************************************
 * @brief typedef for function pointer to store timer expiry call back function.
 ******************************************************************************/
typedef void  ( *sw_tmr_cb_t )( void* context, void  *pTmr );

/**
 *******************************************************************************
 * @brief typedef for function pointer to store the Hardware timer init
 * function.
 ******************************************************************************/
typedef void ( *hw_tmr_init_t )( void *hw_tmr_ins, void *sw_tmr_ins );

/**
 *******************************************************************************
 * @brief typedef for function pointer to store the Hardware timer start
 * function.
 ******************************************************************************/
typedef void ( *hw_tmr_start_t )( void *hw_tmr_ins, p3time_t exp );

/**
 *******************************************************************************
 * @brief typedef for function pointer to store the Hardware stop timer 
 *         function.
 ******************************************************************************/
typedef void ( *hw_tmr_stop_t )( void *hw_tmr_ins );

/**
 *******************************************************************************
 * @brief typedef for function pointer to store the Hardware timer get time 
 *          function. 
 ******************************************************************************/
typedef p3time_t ( *hw_tmr_get_time_t)( void *hw_tmr_ins );

/**
 *******************************************************************************
 * @brief typedef for function pointer to store the Hardware timer module's get 
 * random number function .
 ******************************************************************************/
typedef uint8_t ( *hw_tmr_rand_t)( void *hw_tmr_ins );

/**
 *******************************************************************************
 * @brief typedef for function pointer to store the Hardware timer delay 
 * function 
 ******************************************************************************/
typedef void ( *hw_tmr_delay_t)( void *hw_tmr_ins, uint32_t duration );

/**
 *******************************************************************************
 * @brief typedef for function pointer to store the Hardware timer delay 
 * function 
 ******************************************************************************/
typedef uint16_t ( *hw_tmr_sys_time_high_t)( void );

/**
 *******************************************************************************
 * @struct typedef for binding all the function pointers storing the APIs of the 
 *          hardware timer driver 
 ******************************************************************************/
typedef struct hw_timer_if_struct
{
    void                *pHw_tmr_ins;           /**< The pointer to point to the hardware timer object */        
    hw_tmr_init_t       hw_tmr_init;            /**< The function pointer for the init callback function */
    hw_tmr_start_t      hw_tmr_start;           /**< The function pointer for the start callback function */
    hw_tmr_stop_t       hw_tmr_stop;            /**< The function pointer for stop callback function */
    hw_tmr_get_time_t   hw_tmr_get_time;        /**< The function pointer for get time callback function */
    hw_tmr_rand_t       hw_tmr_rand;            /**< The fucntion pointer for rand callback function */
    hw_tmr_delay_t      hw_tmr_delay;           /**< The function pointer for delay callback function */
    hw_tmr_sys_time_high_t   hw_tmr_sys_time_high;  
}hw_timer_if_t;

/**
 *******************************************************************************
 * @struct SoftWare timer structure.
 *    Timer structure to store the information of the software timer.
 ******************************************************************************/
typedef struct sw_tmr_module_struct
{
    base_t  prio;         /**< Thread priority of the software timer */
    p3_list_t  tmr_active_list; /**< Software Timer ACTIVE Q. Timers that are not
                          started but not expired are stored in this queue in 
                          assending order of their expire time */
	p3_list_t  tmr_pend_list; /**< Software Timer PENDING Q. Timers that are not
                          started, not expired and that are not to be started until a 
						  system time rolls back to zero are stored in this queue in 
                          assending order of their expire time. This queue will be 
						  copied to the actual active Queue once a roll over occurs*/

    p3_list_t  tmr_exp_list; /**< Software Timer EXPIRE Q. Timers are expired but
                       its callback is not yet called are queued in this */
    hw_timer_if_t hw_timer_if;/**< The hardware timer instance for holding the 
                                   hardware timer information*/
	
    p3_list_t       isr_req_q;/**< Queue holding events raised from timer 
                                  interrupt context*/

} sw_tmr_module_t;

/**
 *******************************************************************************
 * @struct SoftWare timer structure.
 *    Timer structure to store the information of the software timer.
 ******************************************************************************/
typedef struct sw_tmr_struct
{
    list_item_t*  next; /**< Next soft timer in the linked-list */
    uint8_t       type; /**< Timer Type */
    uint8_t       isr_req_type; /**< Stores the request given from ISR */
    uint8_t       state; /**< Status of the timer */
    p3time_t      exp_time; /**< Time at which the timer will next 
                                expire */

    stime_t       period; /**< Timer expiration period:For periodic timers, 
                           period is added to expireTime. For one-shot timers, 
                           period is 0 */

    sw_tmr_cb_t   cb;/**< The function pointer for the callback function */

    void*         cb_param;/**< cb(cb_param, &pTimer) is called when timer 
                           expires */
} sw_tmr_t;

/*@}*/

/*
** ============================================================================
** Public Variable Declarations
** ============================================================================
*/

//extern hw_tmr_t hw_tmr_ins; /* The reference to the Hardware Timer Object */
extern sw_tmr_module_t* gptmr_mod_ins;

/*
**=========================================================================
**  Public Function Prototypes
**=========================================================================
*/

/**
 ** \defgroup sw_timer_req  Software Timer APIs
 ** \ingroup Swtmr
 */

/*@{*/

/**
 *******************************************************************************
 * @brief Initialization for SW Timer module.
 *    This function performs initialization of SW timer module. 
 * @param *pTmr_mod_ins[in] - pointer to the instance of timer module structure.
 * @retval true or false.
 * @note
 *    This function must be called during INIT before calling other sw timer
 *     API.
 ******************************************************************************/
void sw_tmr_init( sw_tmr_module_t *pTmr_mod_ins );

/**
 *******************************************************************************
 * @brief Creation of a soft timer.
 *    This function creates a soft timer with the spcified value.
 * @param *pTmr_mod_ins[in] - pointer to the instance of timer module structure.
 * @param *pTmr_ins[in] - pointer to memory location where the sw timer  
 * information is stored.
 * @param type[in] - periodic or One-Shot.
 * @param period[in] - for Periodic Timers, the period of repeating 
 *    expirations, after initial expiration. For One-Shot, period is ignored.
 * @param cb[in] - callback function to be called once the timer expires.
 * @param *context[in] - Context to be stored by the soft timer module and to be
 * passed with the callback function once the timer expires.
 * @retval Timer handle will have a non-zero value if Operation is successful.
 * @retval zero if failed.
 *    
 ******************************************************************************/
bool sw_tmr_create( sw_tmr_t *pTmr_ins, stime_t period, sw_tmr_cb_t cb, void* context );

/**
 *******************************************************************************
 * @brief Start a soft timer.
 *    This function starts a soft timer with the spcified value.
 * @param *pTmr_mod_ins[in] - pointer to the instance of timer module structure.
 * @param *pTmr_ins[in] - pointer to memory location where the sw timer  
 * information is stored.
 * @retval Timer handle will have a non-zero value if Operation is successful.
 * @retval zero if failed.
 *    
 ******************************************************************************/
bool sw_tmr_start( sw_tmr_module_t *pTmr_mod_ins, sw_tmr_t *pTmr_ins );

/**
 *******************************************************************************
 * @brief Start with expiry of a soft timer.
 *    This function starts a soft timer with the spcified value.
 * @param *pTmr_mod_ins[in] - pointer to the instance of timer module structure.
 * @param *pTmr_ins[in] - pointer to memory location where the sw timer  
 * information is stored.
 * @param expire_time[in] - the time at which the timer should expire.
 * @retval Timer handle will have a non-zero value if Operation is successful.
 * @retval zero if failed.
 *    
 ******************************************************************************/
// Sagar: Not Used
/*bool sw_tmr_start_with_expire( sw_tmr_module_t *pTmr_mod_ins, sw_tmr_t *pTmr_ins,  stime_t expire_time );*/

/**
 *******************************************************************************
 * @brief Software timer to be added to the active list.
 * @param *pTmr_mod_ins[in] - pointer to the instance of timer module structure.
 * @param *pTmr_ins[in] - pointer to memory location where the sw timer  
 *			information is stored.
 * @retval None
 *    
 ******************************************************************************/
void sw_tmr_add_to_active_list( sw_tmr_module_t *pTmr_mod_ins, sw_tmr_t *timer,uint8_t add_to_pend);
    
/**
 *******************************************************************************
 * @brief Gets the current time.
 *    This function gives the current time.
 * @param *pTmr_mod_ins[in] - pointer to the instance of timer module structure.
 * @retval current time.
 *    
 ******************************************************************************/
p3time_t sw_current_time_get( sw_tmr_module_t *pTmr_mod_ins );

/**
 *******************************************************************************
 * @brief Get random number.
 *    A Timer Callback routine gets passed:\n
 *      (1) the pointer to the Hardware Timer Object
 * @param *pTmr_mod_ins[in] - pointer to the software timer module object.
 * @retval stime_t - a random number.
 ******************************************************************************/
uint8_t sw_tmr_rand_get( sw_tmr_module_t *pTmr_mod_ins );

/**
 *******************************************************************************
 * @brief Software timer delay callback prototype.
 *    A Timer Callback routine gets passed:\n
 *      (1) the pointer to the Hardware Timer Object\n
 *      (2) the delay duration
 * @param *pTmr_mod_ins[in] - pointer to the software timer module object.
 * @param duration_tks[in] - Number of ticks.
 * @retval None
 ******************************************************************************/
void sw_tmr_delay( sw_tmr_module_t *pTmr_mod_ins, uint32_t duration_tks );

/**
 *******************************************************************************
 * @brief Stop a soft timer.
 *    This function stops the soft timer.
 * @param *pTmr_mod_ins[in] - pointer to the instance of timer module structure.
 * @param *pTmr_ins[in] - timer handle returned in the timer start API.
 * @retval SUCCESS if Operation is successful i.e. the timerHandle passed
 *    is a active and it is removed from the list.
 * @retval FAILURE if timer is not active.
 ******************************************************************************/
void sw_tmr_stop( sw_tmr_module_t *pTmr_mod_ins, sw_tmr_t *pTmr_ins );

/**
 *******************************************************************************
 * @brief Remaining time get.
 *    Returns the remaining time left to expire for a give timer.
 * @param *pTmr_mod_ins[in] - pointer to the software timer module object.
 * @param *pTmr_ins[in] - timer handle returned in the timer start API.
 * @retval remaining time in timer ticks.
 * @note
 *    User should pass a valid timer handle.
 ******************************************************************************/
//Sagar Not Used
stime_t sw_tmr_remaining_time_get( sw_tmr_module_t *pTmr_mod_ins, sw_tmr_t *pTmr_ins );

/**
 *******************************************************************************
 * @brief Minimum Remaining time of all active timer get.
 *    Returns the minimum remaining time left to expire of all the active 
 *    timer.
 * @param *pTmr_mod_ins[in] - pointer to the software timer module object.
 * @retval remaining time in timer ticks. retunrns 0xFFFFFFFFFFFFFFFF (~0L)
 *    if no timer is active.
 * @note
 *    User should pass a valid timer handle.
 ******************************************************************************/
//Sagar Not Used
/*stime_t sw_tmr_get_min_of_all_remaining_time( sw_tmr_module_t *pTmr_mod_ins );*/

/**
 *******************************************************************************
 * @brief A timer is active or not.
 *    Returns whether the timer is active or not.
 * @param *pTmr_mod_ins[in] - pointer to the software timer module object.
 * @param *pTmr_ins[in] - timer handle returned in the timer start API.
 * @retval TRUE if the timer is active else FALSE
 * @note
 *    User should pass a valid timer handle.
 ******************************************************************************/
//Sagar Not Used
/*bool sw_tmr_is_active( sw_tmr_t *pTmr_ins );*/

/**
 *******************************************************************************
 * @brief Period of the timer.
 *    Returns the period timer is active or not.
 * @param *pTmr_mod_ins[in] - pointer to the software timer module object.
 * @param *pTmr_ins[in] - timer handle returned in the timer start API.
 * @retval - period.
 * @note
 *    User should pass a valid timer handle.
 ******************************************************************************/
//Sagar Not Used
/*stime_t sw_tmr_period_get( sw_tmr_module_t *pTmr_mod_ins, sw_tmr_t *pTmr_ins );*/

/**
 *******************************************************************************
 * @brief Request to update time for the respective timer instance
 * @param *pTmr_mod_ins[in] - pointer to the software timer module object.
 * @param cb[in] - function for which the timer needs to be updated.
 * @param *param[in] - 
 * @retval - None
 ******************************************************************************/
//Sagar Not Used
/*void sw_tmr_time_update_req ( sw_tmr_module_t *pTmr_mod_ins, sw_tmr_cb_t cb, void *param );*/

/**
 *******************************************************************************
 * @brief Shutdown the soft timer module.
 *   Stops the hardware timer.
 * @param *pTmr_mod_ins[in] - pointer to the software timer module object.
 * @retval None
 ******************************************************************************/
//Sagar Not Used
//void sw_tmr_deinit( sw_tmr_module_t *pTmr_mod_ins );

/**
 *******************************************************************************
 * @brief Notifies the expiry of hardware and soft timer module.
 * @param *pTmr_mod_ins[in] - pointer to the software timer module object.
 * @retval None
 ******************************************************************************/
void sw_tmr_hw_timer_exp_notif( sw_tmr_module_t *pTmr_mod_ins );

/**
 *******************************************************************************
 * @brief Notifies the expiry of timer module.
 * @param *pTmr_mod_ins[in] - pointer to the software timer module object.
 * @retval None
 ******************************************************************************/
void update_expired_tmr_list(sw_tmr_module_t *pTmr_mod_ins);

/**
 *******************************************************************************
 * @brief Notifies the expird timer callbacks.
 * @param *pTmr_mod_ins[in] - pointer to the software timer module object.
 * @retval None
 ******************************************************************************/
void invoke_expired_timer_cbs(sw_tmr_module_t *pTmr_mod_ins);


/**
 *******************************************************************************
 * @brief Function to get the system high time
 * @param *pTmr_mod_ins[in] - pointer to the software timer module object.
 * @retval sys time
 ******************************************************************************/
/* uint32_t  sw_tmr_get_sys_time_high( sw_tmr_module_t *pTmr_mod_ins );*/ //Sagar Not Used


/*@}*/

#ifdef __cplusplus
}
#endif
#endif /* _SW_TIMER_H_ */



