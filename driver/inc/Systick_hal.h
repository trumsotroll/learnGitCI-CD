#ifndef SYSTICK_HAL_H
#define SYSTICK_HAL_H
#include "common_driver.h"
#include "Systick_driver.h"

/**
 * @brief: disable systick timer
 * use this function to disable the systick before config its register
 */
STATUS_ERROR_CODE Disable_Systick(void);
/**
 * brief enable systick timer
 * use this function to enable the systick timer after all register is set
 */
STATUS_ERROR_CODE Enable_Systick(void);
/**
 * brief select clocksource (processor clock) for systick
 * use this function to set clock for systick
 */
STATUS_ERROR_CODE ClockSource_Systick(void);
/**
 * brief reload the RVR value of systick
 */
STATUS_ERROR_CODE Reload_Systick(uint32_t);
/**
 * @brief enable the interrupt request of systick timer
 */
STATUS_ERROR_CODE TickInt_Enable(void);
#endif /* SYSTICK_HAL_H */
