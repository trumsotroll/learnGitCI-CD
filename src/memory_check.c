/*
 * memory_check.c
 *
 *  Created on: Mar 29, 2026
 *      Author: khang
 */

#include "memory_check.h"
#include "flash_driver.h"

/** Function -----------------------------------------------------------------
*  Function name: is_in_ram
*  Description: check if x (Stack pointer) is in RAM address or not
* -------------------------------------------------------------------------- */
__attribute__((section(".code_ram")))
uint32_t is_in_ram(uint32_t x)
{
    if (x >= SRAM_L_START && x <= SRAM_L_END)
    {
    	return 1;
    }
    if (x >= SRAM_U_START && x <= SRAM_U_END)
    {
    	return 1;
    }
    return 0;
}
