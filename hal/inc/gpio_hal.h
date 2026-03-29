#ifndef GPIO_HAL_H
#define GPIO_HAL_H

#include "../../include/device_registers.h"
#include "../../include/S32K144.h"
#include "stdbool.h"
#include "../../driver/inc/common_driver.h"

#define RISING  0x09 /* Mode interrupt rising edge */
#define FALLING 0x0A /* Mode interrupt falling edge */
#define EITHER  0x0B /* Mode interrupt either edge */


/**
 * @enum PortID
 * use this enum to choose PORT index
 */
typedef enum
{
    PORT_A, /* 0 */
    PORT_B, /* 1 */
    PORT_C, /* 2 */
    PORT_D, /* 3 */
    PORT_E  /* 4 */
}PortID;

/**
 * @enum LEVEL
 * use this enum to choose logic level of output pin
 */
typedef enum
{
    LOW, /* 0 */
    HIGH  /* 1 */
}ARM_GPIO_LEVEL;

/**
 * + bit (0 - 4): Pin ID
 * + bit (5 - 7): Port ID
 */
/* define type ARM_GPIO_Pin_t */
typedef uint32_t ARM_GPIO_Pin_t;

/* encode type ARM_GPIO_Pin_t */
#define FIVELOWMASK 0x1F
#define GPIO_PIN(port, pin) (((uint32_t)(port)) << 5 | ((uint32_t)(pin) & FIVELOWMASK))

/* decode to take PORT ID from ARM_GPIO_Pin_t */
#define TAKE_PORT(gpio_pin) ((uint32_t)(gpio_pin) >> 5)
/* decode to take PIN ID from ARM_GPIO_Pin_t */
#define TAKE_PIN(gpio_pin) ((uint32_t)(gpio_pin) & 0x1F)

/**
 * @enum ARM_GPIO_DIRECTION
 * use this enum to set direction of GPIO pin
 */
typedef enum
{
    ARM_GPIO_INPUT,
    ARM_GPIO_OUTPUT
}ARM_GPIO_DIRECTION;

/**
 * @enum ARM_GPIO_PULL_RESISTOR
 * use this enum to set pull resistor of GPIO pin
 */
typedef enum
{
    ARM_GPIO_PULL_NONE,
    ARM_GPIO_PULLUP,
    ARM_GPIO_PULLDOWN
}ARM_GPIO_PULL_RESISTOR;

/**
 * @enum ARM_GPIO_EVENT_TRIGGER
 * use this enum to set type of interrupt signal GPIO pin
 */
typedef enum
{
    ARM_GPIO_TRIGGER_NONE,
    ARM_GPIO_TRIGGER_RISING_EDGE,
    ARM_GPIO_TRIGGER_FALLING_EDGE,
    ARM_GPIO_TRIGGER_EITHER_EDGE
}ARM_GPIO_EVENT_TRIGGER;

/**
 * @brief define the callback function pointer
 * @params ARM_GPIO_Pin_t input GPIO, PIN information
 * @params ARM_GPIO_EVENT_TRIGGER is the type of interrupt signal
 * @return void
 */
typedef void(*ARM_GPIO_SignalEvent_t)(ARM_GPIO_Pin_t, ARM_GPIO_EVENT_TRIGGER);

/**
 * @struct: ARM_DRIVER_GPIO
 * define the structure of GPIO DRIVER function
 */
typedef struct
{
    STATUS_ERROR_CODE (* Setup)(ARM_GPIO_Pin_t, ARM_GPIO_SignalEvent_t);
    STATUS_ERROR_CODE (* SetEventTrigger)(ARM_GPIO_Pin_t, ARM_GPIO_EVENT_TRIGGER);
    STATUS_ERROR_CODE (* SetDirection)(ARM_GPIO_Pin_t, ARM_GPIO_DIRECTION);
    STATUS_ERROR_CODE (* SetPullResistor)(ARM_GPIO_Pin_t, ARM_GPIO_PULL_RESISTOR);
    void (* SetOutput)(ARM_GPIO_Pin_t, uint32_t);
    uint32_t (* GetInput)(ARM_GPIO_Pin_t);
    void (* SetToggle)(ARM_GPIO_Pin_t);
    void (* SetHigh)(ARM_GPIO_Pin_t);
    void (* SetLow)(ARM_GPIO_Pin_t);
}ARM_DRIVER_GPIO;

/**
 * @brief assign a callback function (ISR) for specific pin
 * @param ARM_GPIO_Pin_t input GPIO, PIN information
 * @param CallbackEvent is ISR function for GPIO interrupt signal
 * @return STATUS_ERROR_CODE return the status of function
 */
STATUS_ERROR_CODE ARM_GPIO_Setup(ARM_GPIO_Pin_t, ARM_GPIO_SignalEvent_t);

/**
 * @brief Set the interrupt config of specific GPIO pin
 * @param ARM_GPIO_Pin_t input GPIO, PIN information
 * @param trigger define the type of interrupt signal on GPIO pin
 * @return STATUS_ERROR_CODE return the status of function
 */
STATUS_ERROR_CODE ARM_GPIO_SetEventTrigger(ARM_GPIO_Pin_t, ARM_GPIO_EVENT_TRIGGER);
/**
 * @brief is
 * @param ARM_GPIO_Pin_t input GPIO, PIN information
 * @param ARM_GPIO_DIRECTION is direction of data (INPUT/OUTPUT)
 * @return STATUS_ERROR_CODE
 */
STATUS_ERROR_CODE ARM_GPIO_SetDirection(ARM_GPIO_Pin_t, ARM_GPIO_DIRECTION);

/**
 * @brief set pull resistor config of specific GPIO pin
 * @param ARM_GPIO_Pin_t input GPIO, PIN information
 * @param ARM_GPIO_PULL_RESISTOR is resistor pulling config
 * @return STATUS_ERROR_CODE
 */
STATUS_ERROR_CODE ARM_GPIO_SetPullResistor(ARM_GPIO_Pin_t, ARM_GPIO_PULL_RESISTOR);
/**
 * @brief Set a specific GPIO pin is OUTPUT
 * @param ARM_GPIO_Pin_t input GPIO, PIN information
 * @param uint32_t is logic level data (0/1)
 * @return STATUS_ERROR_CODE
 */
void ARM_GPIO_SetOutput(ARM_GPIO_Pin_t, uint32_t);
/**
 * @brief get input data from a specific GPIO pin
 * @param ARM_GPIO_Pin_t input GPIO, PIN information
 * @return uint32_t is logic level of GPIO pin
 */
uint32_t ARM_GPIO_GetInput(ARM_GPIO_Pin_t);
/**
 * @brief export LOW logic to gpio pin
 * @param ARM_GPIO_Pin_t input GPIO, PIN information
 */
void ARM_GPIO_OutputLow(ARM_GPIO_Pin_t);
/**
 * @brief export HIGH logic to gpio pin
 * @param ARM_GPIO_Pin_t input GPIO, PIN information
 */
void ARM_GPIO_OutputHigh(ARM_GPIO_Pin_t);
/**
 * @brief: Set toggle pin GPIO
 * @params: ARM_GPIO_Pin_t input GPIO, PIN information
 * @return STATUS_ERROR_CODE
 */
void ARM_GPIO_SetToggle(ARM_GPIO_Pin_t);


#endif /* GPIO_HAL_H */
