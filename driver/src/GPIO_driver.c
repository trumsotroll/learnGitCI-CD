/*
 * GPIO.c
 *
 *  Created on: Jan 7, 2026
 *      Author: khang
 */
#include "stdio.h"
#include "../inc/GPIO_driver.h"
#include "../../hal/inc/GPIO_hal.h"
#include "../inc/common_driver.h"

extern ARM_DRIVER_GPIO DriverGPIO;
ARM_DRIVER_GPIO* GPIOPtr = &DriverGPIO;

extern ARM_GPIO_SignalEvent_t callback[5][32];
extern ARM_GPIO_EVENT_TRIGGER callbackEvent[5][32];
/*FUNCTION**********************************************************************
 *
 * Function Name : GPIO_SetInput
 * Description   : this function set an GPIO pin is input pin
 *
 *END**************************************************************************/
STATUS_ERROR_CODE GPIO_SetInput(ARM_GPIO_Pin_t pin, PULL_t pull)
{
    STATUS_ERROR_CODE status = GPIOPtr->Setup(pin, NULL);
    if(status == ARM_DRIVER_OK)
    {
        status = GPIOPtr->SetDirection(pin, ARM_GPIO_INPUT);
    }
    if(status == ARM_DRIVER_OK)
    {
        status = GPIOPtr->SetPullResistor(pin, pull);
    }
    if(status == ARM_DRIVER_OK)
    {
        return ARM_DRIVER_OK;
    }else
    {
        return ARM_DRIVER_ERROR;
    }
}
/*FUNCTION**********************************************************************
 *
 * Function Name : GPIO_SetOutput
 * Description   : this function set an GPIO pin is output pin
 *
 *END**************************************************************************/
STATUS_ERROR_CODE GPIO_SetOutput(ARM_GPIO_Pin_t pin, ARM_GPIO_LEVEL level)
{
    STATUS_ERROR_CODE status = GPIOPtr->Setup(pin, NULL);
    if(status == ARM_DRIVER_OK)
    {
        status = GPIOPtr->SetDirection(pin, ARM_GPIO_OUTPUT);
    }
    if(status == ARM_DRIVER_OK)
    {
        GPIOPtr->SetOutput(pin, level);
    }
    if(status == ARM_DRIVER_OK)
    {
        return ARM_DRIVER_OK;
    }else
    {
        return ARM_DRIVER_ERROR;
    }
}
/*FUNCTION**********************************************************************
 *
 * Function Name : GPIO_SetInterrupt
 * Description   : this function set an GPIO pin is input interrupt, assigns callback function to a pin
 *
 *END**************************************************************************/
STATUS_ERROR_CODE GPIO_SetInterrupt(ARM_GPIO_Pin_t pin, TRIGGER_t trigger, ARM_GPIO_SignalEvent_t callback)
{
    STATUS_ERROR_CODE status = GPIOPtr->Setup(pin, callback);
    if(status == ARM_DRIVER_OK)
    {
        status = GPIOPtr->SetEventTrigger(pin, trigger);
    }
    if(status == ARM_DRIVER_OK)
    {
        return ARM_DRIVER_OK;
    }else
    {
        return ARM_DRIVER_ERROR;
    }
}
/*FUNCTION**********************************************************************
 *
 * Function Name : GPIO_SetToggle
 * Description   : this function toggle an GPI output pin
 *
 *END**************************************************************************/
void GPIO_SetToggle(ARM_GPIO_Pin_t pin)
{
    GPIOPtr->SetToggle(pin);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : GPIO_SetLow
 * Description   : this function export Low to GPIO pin
 *
 *END**************************************************************************/
void GPIO_SetLow(ARM_GPIO_Pin_t pin)
{
    GPIOPtr->SetLow(pin);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : GPIO_SetHigh
 * Description   : this function export High to GPIO pin
 *
 *END**************************************************************************/
void GPIO_SetHigh(ARM_GPIO_Pin_t pin)
{
    GPIOPtr->SetHigh(pin);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : GPIO_GetInput
 * Description   : this function get input from an input pin
 *
 *END**************************************************************************/
uint32_t GPIO_GetInput(ARM_GPIO_Pin_t pin)
{
    return GPIOPtr->GetInput(pin);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : PORTC_IRQHandler
 * Description   : portC IRQHandler overriding function
 *
 *END**************************************************************************/
void PORTC_IRQHandler(void) /* IRQn port C */
{
    uint32_t pendingISFR = IP_PORTC->ISFR;
    IP_PORTC->ISFR = pendingISFR; /**/

    while(pendingISFR)
    {
        uint32_t pinPosition = __builtin_ctz(pendingISFR); /* 1hot c-ount */

        if (callback[PORT_C][pinPosition] != NULL)
        {
            ARM_GPIO_EVENT_TRIGGER triggerType =callbackEvent[PORT_C][pinPosition];
            callback[PORT_C][pinPosition](GPIO_PIN(PORT_C, pinPosition), triggerType);
        }
        pendingISFR &= ~(1u << pinPosition); /** kep ant 1s */
    }
}
