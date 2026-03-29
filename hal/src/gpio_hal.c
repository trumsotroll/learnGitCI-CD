#include "../inc/gpio_hal.h"
#include "../../include/S32K144.h"
#include "../inc/S32K144_Package.h"
#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"
#include "../../driver/inc/common_driver.h"
/*--------------------------------------- Common use---------------------------------------------------------------*/
/* declare a callback function pointer array for 5 GPIO PORT */
ARM_GPIO_SignalEvent_t callback[5][32] = {NULL};
/* declare a callback envent for each callback function */
ARM_GPIO_EVENT_TRIGGER callbackEvent[5][32] = {ARM_GPIO_TRIGGER_NONE};

static PORT_Type* const PORTx[5] = IP_PORT_BASE_PTRS;
static GPIO_Type* const GPIOx[5] = IP_GPIO_BASE_PTRS;
static IRQn_Type PORT_IRQn[5] = { PORTA_IRQn, PORTB_IRQn, PORTC_IRQn, PORTD_IRQn, PORTE_IRQn };
/* clock gate index for PORT A,B,C,D,E */
static const uint32_t PORT_CLOCK_GATE[5] = { PCC_PORTA_INDEX, PCC_PORTB_INDEX, PCC_PORTC_INDEX, PCC_PORTD_INDEX, PCC_PORTE_INDEX };
/* ----------------------------------------------------------------------------------------------------------- */

/* --------------------------------------- API Setup GPIO driver ----------------------------------------------- */

/*FUNCTION**********************************************************************
 *
 * Function Name : ARM_GPIO_Setup
 * Description   : this function assigns an GPIO pin with a callback function
 *
 *END**************************************************************************/
STATUS_ERROR_CODE ARM_GPIO_Setup(ARM_GPIO_Pin_t pin, ARM_GPIO_SignalEvent_t CallbackEvent)
{
    uint32_t Port = TAKE_PORT(pin);
    uint32_t Pin = TAKE_PIN(pin);

    bool error = false;

    if(Port >= 5 || Pin >= 32)
    {
        error = true;
    }
    callback[Port][Pin] = CallbackEvent;
    IP_PCC->PCCn[PORT_CLOCK_GATE[Port]] |= PCC_PCCn_CGC_MASK;
    PORTx[Port]->PCR[Pin] |= PORT_PCR_MUX(1);
    if(error == true)
        return ARM_DRIVER_ERROR;
    else
        return ARM_DRIVER_OK;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : ARM_GPIO_SetDirection
 * Description   : this function set data direction of an GPIO pin
 *
 *END**************************************************************************/
STATUS_ERROR_CODE ARM_GPIO_SetDirection(ARM_GPIO_Pin_t pin, ARM_GPIO_DIRECTION direction)
{
    uint32_t Port = TAKE_PORT(pin);
    uint32_t Pin = TAKE_PIN(pin);
    bool error = false;
    if(Port >= 5 || Pin >= 32)
    {
        error = true;
    }
    switch (direction)
    {
    case ARM_GPIO_INPUT:
        GPIOx[Port]->PDDR &= ~(1 << Pin);
        break;
    case ARM_GPIO_OUTPUT:
        GPIOx[Port]->PDDR |= (1 << Pin);
        break;
    default:
        error = true;
    }
    if(error == true)
        return ARM_DRIVER_ERROR;
    else
        return ARM_DRIVER_OK;
}


/*FUNCTION**********************************************************************
 *
 * Function Name : ARM_GPIO_SetPullResistor
 * Description   : this function set pull resistor for GPIO Pin
 *
 *END**************************************************************************/
STATUS_ERROR_CODE ARM_GPIO_SetPullResistor(ARM_GPIO_Pin_t encode_pin, ARM_GPIO_PULL_RESISTOR resistor)
{
    uint32_t Port = TAKE_PORT(encode_pin);
    uint32_t Pin = TAKE_PIN(encode_pin);

    bool error = false;

    if(Port >= 5 || Pin >= 32)
    {
        error = true;
    }
    switch (resistor)
    {
    case ARM_GPIO_PULL_NONE:
        PORTx[Port]->PCR[Pin] &= ~(1 << PORT_PCR_PE_SHIFT); /* unable pull */
        break;
    case ARM_GPIO_PULLUP:
        PORTx[Port]->PCR[Pin] |= (1 << PORT_PCR_PE_SHIFT); /* enable pull */
        PORTx[Port]->PCR[Pin] |= (1 << PORT_PCR_PS_SHIFT); /* pull up */
        break;
    case ARM_GPIO_PULLDOWN:
        PORTx[Port]->PCR[Pin] |= (1 << PORT_PCR_PE_SHIFT); /* enable pull */
        PORTx[Port]->PCR[Pin] &= ~(1 << PORT_PCR_PS_SHIFT); /* pull down */
        break;
    default:
        error = true;
    }
    if(error == true)
        return ARM_DRIVER_ERROR;
    else
        return ARM_DRIVER_OK;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : ARM_GPIO_SetOutput
 * Description   : this function logical level for GPIO ouput Pin
 *
 *END**************************************************************************/
void ARM_GPIO_SetOutput(ARM_GPIO_Pin_t encode_pin, uint32_t output)
{
    uint32_t Port = TAKE_PORT(encode_pin);
    uint32_t Pin = TAKE_PIN(encode_pin);
    if(output == 0)
        GPIOx[Port]->PCOR = (1u << Pin);        /* clear bit */
    else
        GPIOx[Port]->PSOR = (1u << Pin);        /* Set bit */
}

void ARM_GPIO_SetToggle(ARM_GPIO_Pin_t encode_pin)
{
    uint32_t Port = TAKE_PORT(encode_pin);
    uint32_t Pin = TAKE_PIN(encode_pin);
    GPIOx[Port]->PTOR = (1u << Pin);        /* clear bit */
}

void ARM_GPIO_OutputHigh(ARM_GPIO_Pin_t encode_pin)
{
    uint32_t Port = TAKE_PORT(encode_pin);
    uint32_t Pin = TAKE_PIN(encode_pin);
    GPIOx[Port]->PSOR = (1u << Pin);        /* clear bit */
}
void ARM_GPIO_OutputLow(ARM_GPIO_Pin_t encode_pin)
{
    uint32_t Port = TAKE_PORT(encode_pin);
    uint32_t Pin = TAKE_PIN(encode_pin);
    GPIOx[Port]->PCOR = (1u << Pin);        /* clear bit */
}

/*FUNCTION**********************************************************************
 *
 * Function Name : ARM_GPIO_GetInput
 * Description   : this function gets data input from GPIO Pin
 *
 *END**************************************************************************/
uint32_t ARM_GPIO_GetInput(ARM_GPIO_Pin_t encode_pin)
{
    uint32_t Port = TAKE_PORT(encode_pin);
    uint32_t Pin = TAKE_PIN(encode_pin);

    return (GPIOx[Port]->PDIR >> Pin) & 1u;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : ARM_GPIO_SetEventTrigger
 * Description   : this function set interrupt config for a specific GPIO pin
 *
 *END**************************************************************************/
STATUS_ERROR_CODE ARM_GPIO_SetEventTrigger(ARM_GPIO_Pin_t encode_pin, ARM_GPIO_EVENT_TRIGGER trigger)
{
    uint32_t Port = TAKE_PORT(encode_pin);
    uint32_t Pin = TAKE_PIN(encode_pin);

    bool error = false;

    if(Port >= 5 || Pin >= 32)
    {
        error = true;
    }

    S32_NVIC->ISER[PORT_IRQn[Port] >> 5] = 1u << (PORT_IRQn[Port] & 31u);

    if(trigger == ARM_GPIO_TRIGGER_NONE)
    {
        PORTx[Port]->PCR[Pin] &= ~PORT_PCR_IRQC_MASK; /* set IRQC = 0000 */
    }else if(trigger == ARM_GPIO_TRIGGER_RISING_EDGE)
    {
        PORTx[Port]->ISFR = (1u << Pin);
        PORTx[Port]->PCR[Pin] = (PORTx[Port]->PCR[Pin] & ~PORT_PCR_IRQC_MASK) | PORT_PCR_IRQC(RISING);
        callbackEvent[Port][Pin] = ARM_GPIO_TRIGGER_RISING_EDGE;
    }else if(trigger == ARM_GPIO_TRIGGER_FALLING_EDGE)
    {
        PORTx[Port]->ISFR = (1u << Pin);
        PORTx[Port]->PCR[Pin] = (PORTx[Port]->PCR[Pin] & ~PORT_PCR_IRQC_MASK) | PORT_PCR_IRQC(FALLING);
        callbackEvent[Port][Pin] = ARM_GPIO_TRIGGER_FALLING_EDGE;
    }else if(trigger == ARM_GPIO_TRIGGER_EITHER_EDGE)
    {
        PORTx[Port]->ISFR = (1u << Pin);
        PORTx[Port]->PCR[Pin] = (PORTx[Port]->PCR[Pin] & ~PORT_PCR_IRQC_MASK) | PORT_PCR_IRQC(EITHER);
        callbackEvent[Port][Pin] = ARM_GPIO_TRIGGER_EITHER_EDGE;
    }else
    {
        error = true;
    }

    if(error == true)
        return ARM_DRIVER_ERROR;
    else
        return ARM_DRIVER_OK;
}

/*--------------------------------- Struct DriverGPIO ------------------- -----------------------------------*/
ARM_DRIVER_GPIO DriverGPIO = {
    .Setup = ARM_GPIO_Setup,
    .SetEventTrigger = ARM_GPIO_SetEventTrigger,
    .SetDirection = ARM_GPIO_SetDirection,
    .SetPullResistor = ARM_GPIO_SetPullResistor,
    .SetOutput = ARM_GPIO_SetOutput,
    .GetInput = ARM_GPIO_GetInput,
	.SetHigh = ARM_GPIO_OutputHigh,
	.SetLow = ARM_GPIO_OutputLow,
    .SetToggle = ARM_GPIO_SetToggle
};
/* ------------------------------------------------------------------------------------------------------------*/
