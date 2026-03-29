#include "../inc/Systick_hal.h"
#include "../../include/S32K144.h"
#include "../inc/common_driver.h"

extern uint32_t SystemCoreClock;
#define PROCESSOR_CLK 4

/*FUNCTION**********************************************************************
 *
 * Function Name : Disable_Systick
 * Description   : this function disable the systick
 *
 *END**************************************************************************/
STATUS_ERROR_CODE Disable_Systick()
{
    S32_SysTick->CSR = 0;
    /* clear pending Systick */
    S32_SCB->ICSR = S32_SCB_ICSR_PENDSTCLR_MASK;

    if((S32_SysTick->CSR & ~S32_SysTick_CSR_ENABLE_MASK) == 0)
    {
        return ARM_DRIVER_OK;
    }else
    {
        return ARM_DRIVER_ERROR;
    }
}
/*FUNCTION**********************************************************************
 *
 * Function Name : Enable_Systick
 * Description   : this function enable the systick
 *
 *END**************************************************************************/
STATUS_ERROR_CODE Enable_Systick()
{
    S32_SysTick->CSR |= S32_SysTick_CSR_ENABLE(1);
    if(S32_SysTick->CSR & S32_SysTick_CSR_ENABLE_MASK)
    {
        return ARM_DRIVER_OK;
    }else
    {
        return ARM_DRIVER_ERROR;
    }
}
/*FUNCTION**********************************************************************
 *
 * Function Name : TickInt_Enable
 * Description   : this function enable the systick interrupt request
 *
 *END**************************************************************************/
STATUS_ERROR_CODE TickInt_Enable()
{
    S32_SysTick->CSR |= S32_SysTick_CSR_TICKINT(1);
    if(S32_SysTick->CSR & S32_SysTick_CSR_TICKINT_MASK)
    {
        return ARM_DRIVER_OK;
    }else
    {
        return ARM_DRIVER_ERROR;
    }
}
/*FUNCTION**********************************************************************
 *
 * Function Name : ClockSource_Systick
 * Description   : this function set systick clock is processor clock
 *
 *END**************************************************************************/
STATUS_ERROR_CODE ClockSource_Systick()
{
    S32_SysTick->CSR |= S32_SysTick_CSR_CLKSOURCE(1);
    if(S32_SysTick->CSR & PROCESSOR_CLK)
    {
        return ARM_DRIVER_OK;
    }else
    {
        return ARM_DRIVER_ERROR;
    }
}
/*FUNCTION**********************************************************************
 *
 * Function Name : Reload_Systick
 * Description   : this function sets systick timing period
 *
 *END**************************************************************************/
STATUS_ERROR_CODE Reload_Systick(uint32_t tickms)
{
    uint32_t RELOAD = ((tickms * 0.001) * SystemCoreClock) - 1;
    S32_SysTick->RVR = RELOAD;
    if(S32_SysTick->RVR == RELOAD)
    {
        return ARM_DRIVER_OK;
    }else
    {
        return ARM_DRIVER_ERROR;
    }

}
