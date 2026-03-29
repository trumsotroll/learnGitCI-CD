#include "Systick_driver.h"
#include "Systick_hal.h"
#include "S32K144.h"

/*FUNCTION**********************************************************************
 *
 * Function Name : Systick_Config
 * Description   : this function is called in the begin of main
 *
 *END**************************************************************************/
void Systick_Config(uint32_t tick)
{
    Disable_Systick();
    ClockSource_Systick();
    TickInt_Enable();
    Reload_Systick(tick);
    Enable_Systick();
}
/*FUNCTION**********************************************************************
 *
 * Function Name : Systick_disable
 * Description   : this function disables the System tick
 *
 *END**************************************************************************/
void Systick_disable()
{
	S32_SysTick->CSR |= S32_SysTick_CSR_TICKINT(0);
	Disable_Systick();
}

/*FUNCTION**********************************************************************
 *
 * Function Name : GetCurrentTick
 * Description   : this function return the current System tick CRV
 *
 *END**************************************************************************/
uint32_t GetCurrentTick(void)
{
	return S32_SysTick->CSR & S32_SysTick_CVR_CURRENT_MASK;
}
