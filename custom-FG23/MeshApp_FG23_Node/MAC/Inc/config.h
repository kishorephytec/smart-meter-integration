/** \file config.h
 *******************************************************************************
 ** \brief Defines configuration
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

#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/*
** ============================================================================
** Public Macro definitions
** ============================================================================
*/
#define PARAM_IN
#define PARAM_OUT

#if defined(__GNUC__) || defined(_LANGUAGE_C)
/* C definitions */

#ifndef section
#define section(x) __attribute__((section (x)))
#endif

#ifndef align
#define align(x)   __attribute__((aligned (x)))
#endif

#else // __GNUC__ || _LANGUAGE_C
/* ASM definitions */

#ifdef ADI
#define ZXTND (Z)
#define AC    AC0
#define ARGOFFSET 12 
#define EXTRACT_REG(REG1,REG2)         extract(REG1,REG2)(Z)
#else
#define ZXTND
#define AC    ac
#define ARGOFFSET 0
#define EXTRACT_REG(REG1,REG2)         extract(REG1,REG2)
#endif

#endif // __GNUC__ || _LANGUAGE_C

/*
** ============================================================================
** Public Structures, Unions & enums Type Definitions
** ============================================================================
*/

/* None */

/*
** ============================================================================
** Public Variable Declarations
** ============================================================================
*/
	
/* None*/

/*
**=========================================================================
**  Public Function Prototypes
**=========================================================================
*/

/* None */

#ifdef __cplusplus
}
#endif
#endif // CONFIG_H
