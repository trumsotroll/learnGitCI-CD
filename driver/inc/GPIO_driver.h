/*
 * GPIO.h
 *
 *  Created on: Jan 7, 2026
 *      Author: khang
 */

#ifndef INC_GPIO_DRIVER_H_
#define INC_GPIO_DRIVER_H_

#include "S32K144_Package.h"
#include "gpio_hal.h"

/**
 * @enum PULL_t
 * use this enum to set type of pulling resistor
 */
typedef enum
{
    NONE,
    PULLUP,
    PULLDOWN
}PULL_t;

/**
 * @enum TRIGGER_t
 * use this enum to set type of interrupt signal GPIO pin
 */
typedef enum
{
    TRIGGER_NONE,
    RISING_EDGE,
    FALLING_EDGE,
    EITHER_EDGE
}TRIGGER_t;

/*
 * @brief set an GPIO pin as Input pin
 * @params ARM_GPIO_Pin_t is information about GPIO, PIN
 * @params ARM_GPIO_PULL_RESISTOR is PULL setting of specific GPIO pin
 * @return STATUS_ERROR_CODE
 */
STATUS_ERROR_CODE GPIO_SetInput(ARM_GPIO_Pin_t, PULL_t);
/*
 * @brief set an GPIO pin as Output pin
 * @params ARM_GPIO_Pin_t is information about GPIO, PIN
 * @params ARM_GPIO_LEVEL is output logic level
 * @return STATUS_ERROR_CODE
 */
STATUS_ERROR_CODE GPIO_SetOutput(ARM_GPIO_Pin_t, ARM_GPIO_LEVEL);
/*
 * @brief Toggle an specific GPIO pin
 * @params ARM_GPIO_Pin_t is information about GPIO, PIN
 * @return STATUS_ERROR_CODE
 */
void GPIO_SetToggle(ARM_GPIO_Pin_t);
/*
 * @brief export Low to an specific GPIO pin
 * @params ARM_GPIO_Pin_t is information about GPIO, PIN
 */
void GPIO_SetLow(ARM_GPIO_Pin_t pin);
/*
 * @brief export high to an specific GPIO pin
 * @params ARM_GPIO_Pin_t is information about GPIO, PIN
 */
void GPIO_SetHigh(ARM_GPIO_Pin_t);
/*
 * @brief get data from an GPIO Input pin
 * @params ARM_GPIO_Pin_t is information about GPIO, PIN
 * @return uint32_t is input logic level
 */
uint32_t GPIO_GetInput(ARM_GPIO_Pin_t);
/*
 * @brief Set a trigger and callback function for a specific input pin
 * @params ARM_GPIO_Pin_t is information about GPIO, PIN
 * @params ARM_GPIO_EVENT_TRIGGER is type of interrupt trigger
 * @params ARM_GPIO_SignalEvent_t is callback function
 * @return STATUS_ERROR_CODE
 */
STATUS_ERROR_CODE GPIO_SetInterrupt(ARM_GPIO_Pin_t, TRIGGER_t, ARM_GPIO_SignalEvent_t);

#endif /* INC_GPIO_DRIVER_H_ */
