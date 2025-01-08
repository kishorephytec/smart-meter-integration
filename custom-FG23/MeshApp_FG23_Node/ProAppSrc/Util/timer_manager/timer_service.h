/** \file timer_service.h
 *******************************************************************************
 ** \brief Implements the OS dependant part of the timer service
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

#ifndef TIMER_SERVICE_H
#define TIMER_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

/*
** ============================================================================
** Public Macro definitions
** ============================================================================
*/

/**
 ** \defgroup timer_service  Timer Service Module
 */

/*@{*/

/**
 ** \defgroup timer_def Timer Service Definitions
 ** \ingroup timer_service
 */

/*@{*/

/* None */

/*
** ============================================================================
** Public Structures, Unions & enums Type Definitions
** ============================================================================
*/

/* None */

/*@}*/

/*
** ============================================================================
** Public Variable Declarations
** ============================================================================
*/

extern sw_tmr_module_t* gpTmr_mod_ins;

/*
** ============================================================================
** Public Function Prototypes
** ============================================================================
*/


/**
 ** \defgroup timer_func Timer Service APIs
 ** \ingroup timer_service
 */

/*@{*/

/**
 *****************************************************************************
 * @brief Initializes the software timer service
 * @param None
 * @retval TRUE or FALSE
 * @note This function should be called atleast once during the system 
 * initialization and only after this, the timer services should be used.
 *****************************************************************************/
void tmr_service_init(void);

/**
 *****************************************************************************
 * @brief Implements the timer task functionality.
 * @param None
 * @retval None
 * @note This function should be called in the main functions while(1) loop.
 *****************************************************************************/
void timer_task(void);

/**
 *****************************************************************************
 * @brief Implements the delay functionality
 * @param usecs[in] - the delay in microseconds 
 * @retval None
 * @note The delay specified should be in terms of microseconds.
 *****************************************************************************/
void tmr_delay(uint32_t usecs);

/**
 *****************************************************************************
 * @brief Implements the software timer start functionality
 * @param *pTmr_ins[in] - the software timer to be started
 * @retval TRUE or FALSE
 * @note This function should be called only after creating a software timer.
 * @sa tmr_create_one_shot_timer, tmr_create_periodic_timer 
 *****************************************************************************/
bool tmr_start_relative( sw_tmr_t *pTmr_ins );

/**
 *****************************************************************************
 * @brief Implements the software timer start functionality
 * @param *pTmr_ins[in] - the software timer to be started
 * @param point_in_time - reference time wrt which the timer is started
 * @param period - the time period from the reference time after which the 
 *                 timer expires.
 * @retval TRUE or FALSE
 * @note This function should be called only after creating a software timer.
 * @sa tmr_create_one_shot_timer, tmr_create_periodic_timer 
 *****************************************************************************/
bool tmr_start_absolute( sw_tmr_t *pTmr_ins, p3time_t point_in_time, stime_t period );

/**
 *****************************************************************************
 * @brief Provides the remaining time left for the software timer to get expired
 * @param *pTmr_ins[in] - the currently running software timer
 * @retval stime_t - time in usecs after which the timer will expire
 * @note none
 *****************************************************************************/
stime_t tmr_get_remaining_time(sw_tmr_t *pTmr_ins);

/**
 *****************************************************************************
 * @brief Provides the time left for the next software timer to be expired.
 * @param None
 * @retval stime_t - remaining time after which a timer expires
 * @note 
 *****************************************************************************/
//Sagar Not Used
//stime_t tmr_get_min_of_all_rem_timer(void);

/**
 *****************************************************************************
 * @brief Initializes an one shot software timer 
 * @param *pTmr_ins[in] - sw timer to be initilialized
 * @param  period[in] - indicates the period after which the timer will expire
 * @param  cb[in] - holds the callback function which will be invoked after the timer expires
 * @param *param[in] - holds the param used for passing to the invoked expiry callback function
 * @retval TRUE or FALSE
 * @note The memory for the softtware timer should be defined before 
 * initializing it using this function
 *****************************************************************************/
void tmr_create_one_shot_timer( sw_tmr_t *pTmr_ins,stime_t period,sw_tmr_cb_t cb,void* param );

/**
 *****************************************************************************
 * @brief Initializes a periodic software timer
 * @param *pTmr_ins[in] - sw timer to be initilialized
 * @param  period[in]  - periodicity in usecs at which the timer should keep expiring
 * @param  cb[in] - holds the callback function which will be invoked after the timer expires
 * @param *param[in] - holds the param used for passing to the invoked expiry callback function
 * @retval TRUE or FALSE
 * @note The memory for the softtware timer should be defined before 
 * initializing it using this function
 *****************************************************************************/
// Sagar: Not Used
//bool tmr_create_periodic_timer( sw_tmr_t *pTmr_ins,stime_t period,sw_tmr_cb_t cb,void* param );

/**
 *****************************************************************************
 * @brief Implements the software timer stop service
 * @param *pTmr_ins[in] - active timer to be stopped
 * @retval TRUE or FALSE
 * @note None
 *****************************************************************************/
void tmr_stop( sw_tmr_t *pTmr_ins );


/**
 *****************************************************************************
 * @brief Implements the software current time read service
 * @param None
 * @retval p3time_t - Current time
 * @note None
 *****************************************************************************/
p3time_t timer_current_time_get(void);

p3time_t timer_current_time_get_high_32(void);

uint64_t get_time_now_64 (void);


/**
 *****************************************************************************
 * @brief Implements the software random number read service
 * @param None
 * @retval uint8_t - Random number used to assign DSN, BSN or EBSN etc
 * @note None
 *****************************************************************************/
uint8_t timer_rand_get(void);


/*@}*/

/*@}*/

#ifdef __cplusplus
}
#endif
#endif /* TIMER_SERVICE_H */
