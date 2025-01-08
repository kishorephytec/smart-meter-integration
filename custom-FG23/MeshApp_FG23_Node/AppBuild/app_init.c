/***************************************************************************//**
 * @file
 * @brief app_init.c
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
#include "sl_rail_util_init.h"
#include "em_gpio.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
/******************************************************************************
 * The function is used for some basic initialization related to the app.
 *****************************************************************************/
RAIL_Handle_t app_init(void)
{
  // Get RAIL handle, used later by the application
  RAIL_Handle_t rail_handle = sl_rail_util_get_handle(SL_RAIL_UTIL_HANDLE_INST0);

  /////////////////////////////////////////////////////////////////////////////
  // Put your application init code here!                                    //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////

  return rail_handle;
}

// -----------------------------------------------------------------------------
//                          LED Function Definitions
// -----------------------------------------------------------------------------

void APP_LED_GPIO_init ()
{
  
  // Configure the GPIO pins F4 and F5 for the LEDs as output pins  
    
    GPIO_PinModeSet(gpioPortC, 0, gpioModePushPull, 0);


  
}

void APP_LED_ON ()
{
  
  GPIO_PinOutSet (gpioPortC, 0);
  
  
}


void APP_LED_OFF ()
{
  
  GPIO_PinOutClear (gpioPortC, 0); //(LED_PORT, LED1);
  
}


void APP_LED_TOGGLE ()
{
  
    GPIO_PinOutToggle(gpioPortC, 0);
  
}



void SET_NETWORK_STATE_HIGH()
{
  
  // Configure the GPIO pins F4 and F5 for the LEDs as output pins  
    
    GPIO_PinModeSet(gpioPortB, 1, gpioModePushPull, 0);
    GPIO_PinOutSet (gpioPortB, 1);


  
}



/*****************************************************************************/
//For Led 2 Operations

void APP_LED_2_GPIO_init ()
{
  
  // Configure the GPIO pins F4 and F5 for the LEDs as output pins  
    
    GPIO_PinModeSet(gpioPortD, 2, gpioModePushPull, 0);


  
}

void APP_LED_2_ON ()
{
  
  GPIO_PinOutSet (gpioPortD, 2);
  
  
}


void APP_LED_2_OFF ()
{
  
  GPIO_PinOutClear (gpioPortD, 2); //(LED_PORT, LED2);
  
}


void APP_LED_2_TOGGLE ()
{
  
    GPIO_PinOutToggle(gpioPortD, 2);
  
}



















// -----------------------------------------------------------------------------
//                          LED Function Definitions
// -----------------------------------------------------------------------------
