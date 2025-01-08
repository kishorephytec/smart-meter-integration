/** \file sm.h
 *******************************************************************************
 ** \brief This file includes common definitions for State Machine
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

#ifndef SM_H
#define SM_H

#ifdef __cplusplus
extern "C" {
#endif

/*
** ============================================================================
** Public Macro definitions
** ============================================================================
*/

/* None */

/*
** ============================================================================
** Public Structures, Unions & enums Type Definitions
** ============================================================================
*/

/**
 *******************************************************************************
 ** \enum sm_trigger_t
 ** Representation of a trigger to the state machine
 *******************************************************************************
 **/
typedef enum
{
    SM_TRIGGER_ENTRY,
    SM_TRIGGER_EXIT
} sm_trigger_t;

/**
 *******************************************************************************
 ** \struct sm_event_t
 ** Representation of an event to the state machine
 *******************************************************************************
 **/
typedef struct 
{
    sm_trigger_t trigger;
    union
    {
        uchar scalar;
        void *vector;
    } param;
} sm_event_t;

/* The following events are defined.*/
static sm_event_t const sm_entry_event = { SM_TRIGGER_ENTRY, { 0 } };
static sm_event_t const sm_exit_event = { SM_TRIGGER_EXIT, { 0 } };

/**
 *******************************************************************************
 ** \struct sm_t
 ** Representation of the state machine
 *******************************************************************************
 **/
typedef struct sm sm_t;

typedef void (*sm_result_t)(sm_t *, const sm_event_t *);
typedef sm_result_t (*sm_state_t)(sm_t *, const sm_event_t *);

/**
 *******************************************************************************
 ** \struct sm
 ** Data structure to store current state of a state machine
 *******************************************************************************
 **/
struct sm
{
    sm_state_t state; /**< current state */
	sm_state_t backup_state; /**< current state */
};



extern uint16_t a_big_problem;

static sm_result_t error_increment( sm_t *s )
{
  a_big_problem++;
  return NULL_POINTER;
}

//#define SM_DISPATCH(machine, event) (*(machine)->state)((machine), (event))
#define SM_DISPATCH(machine, event) (((machine != NULL)&&((machine)->state != NULL))?((*((machine)->state))((machine), (event))):(error_increment(NULL_POINTER)))

//inline static void SM_DISPATCH(sm_t* machine, const sm_event_t* event)
//{
//	
//    if(machine->state == NULL)
//    {
//      a_big_problem++;
//    }
// 
//    (*(machine)->state)((machine), (event));
//
//}


/*
** ============================================================================
** Public Variable Declarations
** ============================================================================
*/

/* None */

/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/

/* None */

/*
** =============================================================================
** Public Variables Definitions
** =============================================================================
**/

/* None */

/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/

/* None */

/*
**=========================================================================
**  Public Function Prototypes
**=========================================================================
*/

/* None */

/*
**=========================================================================
**  Public Function Definiation
**=========================================================================
*/

/**
 *******************************************************************************
 ** \brief Effect a state transition of the state machine
 ** \param *machine - state machine
 ** \param target - target state to be changed
 ** \retval - None
 ** \note - this is a critical function, should not be interrupted
 ******************************************************************************/

static void sm_transit(sm_t *machine, const sm_state_t target)
{
    //irq_state_t flags = irq_disable(); 

    /* execute the exit action of current state */
    SM_DISPATCH(machine, &sm_exit_event);
    if(target == NULL)
    {
      a_big_problem++;
    }
    /* transition to new state */
    machine->state = target;

    /* execute the entry action of new state */
    SM_DISPATCH(machine, &sm_entry_event);

    //irq_enable(flags);
}

static void sm_back_up_state( sm_t * machine )
{
	machine->backup_state = machine->state;
}

static void sm_restore_state( sm_t * machine )
{
	machine->state = machine->backup_state;
}


/******************************************************************************/

#ifdef __cplusplus
}
#endif
#endif /* SM_H */

