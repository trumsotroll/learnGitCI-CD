#include <stdint.h>
#include "S32K144_features.h"   // hoặc header CMSIS tương đương
#include "S32K144.h"
#include "s32_core_cm4.h"
#include "jump_to_app.h"
#include "UART_driver.h"

/**
 * function to run at the entry 1 (reset handler)
 */
typedef void (*GO_RESET)(void);

/** Function***********************************************************************************
 * Function name: __DSB
 * Description : this function ensure the data bus processes have been taken before is done-
 * after the next action is taken
********************************************************************************************/
inline void __DSB(void)
{
    __asm volatile ("dsb 0xF" : : : "memory");
}

/** Function***********************************************************************************
 * Function name: __ISB
 * Description : this function complete the process of all the instruction before the ISB-
 * It also flush the pipeline, refetch instruction at new state after VTOR/MPU/CONTROL change
********************************************************************************************/
inline void __ISB(void)
{
    __asm volatile ("isb 0xF" : : : "memory");
}

/** Function***********************************************************************************
 * Function name: __set_MSP
 * Description : this function set the initTopOfStack address in word 0 of vector table-
 * into the SP register (r13)
********************************************************************************************/
inline void __set_MSP(uint32_t topOfMainStack)
{
    __asm volatile ("msr msp, %0" : : "r" (topOfMainStack));
}

/** Function***********************************************************************************
 * Function name: jump_to_app
 * Description : this function check all the routine to ensure the right condition to jump to
 * the application:
 * read entry 0 (stack pointer)
 * read entry 1 (reset handler)
 * disable global interrupt
 * clear all pending ISR + pending exception
 * set VTOR to application
 * set MSP
 * jump to application
********************************************************************************************/
__attribute__((section(".code_ram")))
void jump_to_app(uint32_t app_base)
{
	/* read the value at word 0 (TopOfStack) & word 1(reset_handler) of vector table */
    uint32_t app_msp   = *(volatile uint32_t *)(app_base + 0u);
    uint32_t app_resethandler = *(volatile uint32_t *)(app_base + 4u);

    /* disable global interrupt */
    DISABLE_INTERRUPTS();

    /* Clear pending system exceptions */
    S32_SCB->ICSR = S32_SCB_ICSR_PENDSVCLR_MASK | S32_SCB_ICSR_PENDSTCLR_MASK;

    /* Disable + clear all pending IRQ */
    for (int i = 0; i < 8; i++)
    {
        S32_NVIC->ICER[i] = 0xFFFFFFFFu;
        S32_NVIC->ICPR[i] = 0xFFFFFFFFu;
    }

    /* set CONTROL = 0 to ensure priviledged mode and MSP */
    __asm volatile ("mov r0, #0");
    __asm volatile ("msr CONTROL, r0");
    __ISB();

    /* Set vector table to APPLICATION */
    S32_SCB->VTOR = app_base;
    /* data synchronization barrier + instruction synchronization barrier */
    __DSB(); __ISB();

    /* Set MSP to application */
    __set_MSP(app_msp);
    __DSB(); __ISB();

    /* Jump to APPLICATION */
    ((GO_RESET)app_resethandler)();

    /* if main return, while(1) keep the function freeze */
    while(1) {}
}
