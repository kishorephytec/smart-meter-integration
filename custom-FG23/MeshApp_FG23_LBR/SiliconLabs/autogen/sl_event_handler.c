#include "sl_event_handler.h"

#include "em_chip.h"
#include "sl_device_init_nvic.h"
#include "sl_device_init_dcdc.h"
#include "sl_device_init_hfxo.h"
#include "sl_device_init_clocks.h"
#include "sl_device_init_emu.h"
#include "pa_conversions_efr32.h"
#include "sl_rail_util_pti.h"
#include "sl_rail_util_rssi.h"
#include "sl_rail_util_init.h"
#include "sl_mpu.h"

/***********************************************************/
/***********************************************************/

int8_t App_RAIL_rf_device_register(void);
extern int process_run(void);
static int r;

/***********************************************************/
/***********************************************************/

void sl_platform_init(void)
{
  CHIP_Init();
  sl_device_init_nvic();
  sl_device_init_dcdc();
  sl_device_init_hfxo();
  sl_device_init_clocks();
  sl_device_init_emu();
}

void sl_driver_init(void)
{
}

void sl_service_init(void)
{
  sl_mpu_disable_execute_from_ram();
}

void sl_stack_init(void)
{
  sl_rail_util_pa_init();
  sl_rail_util_pti_init();
  sl_rail_util_rssi_init();
  App_RAIL_rf_device_register();
}

void sl_internal_app_init(void)
{
  
}

void sl_platform_process_action(void)
{
}

void sl_service_process_action(void)
{
}

void sl_stack_process_action(void)
{
  
      r = process_run();
  
}

void sl_internal_app_process_action(void)
{
  
}

