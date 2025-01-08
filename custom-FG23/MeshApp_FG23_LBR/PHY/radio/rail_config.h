/***************************************************************************//**
 * @brief RAIL Configuration
 * @details
 *   WARNING: Auto-Generated Radio Config Header  -  DO NOT EDIT
 *   Radio Configurator Version: 2304.5.2
 *   RAIL Adapter Version: 2.4.33
 *   RAIL Compatibility: 2.x
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef __RAIL_CONFIG_H__
#define __RAIL_CONFIG_H__

#include <stdint.h>
#include "rail_types.h"

#define PHY_ACCELERATION_BUFFER phyAccelerationBuffer
extern uint32_t phyAccelerationBuffer[];

#define RADIO_CONFIG_XTAL_FREQUENCY 39000000UL

#define RAIL0_PHY_MODE1_PHY_IEEE802154_WISUN_866MHZ_2GFSK_50KBPS_1A_IN
#define RAIL0_PHY_MODE1_PROFILE_WISUN_FAN_1_0
extern const RAIL_ChannelConfig_t *channelConfigs[];

typedef struct RAIL_StackInfoWisun {
  RAIL_StackInfoCommon_t stackInfoCommon;
  uint8_t version;
  uint8_t wisunChannelParam;// wisunOperatingClass for version=0, wisunChannelPlanId for version=1
  uint8_t wisunRegDomain;
} RAIL_StackInfoWisun_t;

#endif // __RAIL_CONFIG_H__
