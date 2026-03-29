#include "../inc/UART_driver.h"
#include "../inc/common_driver.h"
#include "../../include/S32K144.h"

/* define physical UART1 pinout */
#define UART1_RX 6
#define UART1_TX 7

/* define callback function for UART1 ISR */
ARM_UART_SignalEvent_t UART1_callback = NULL;

/* receive buffer length */
#define RX_BUF_SZ 1024
volatile uint8_t  rx_buf[RX_BUF_SZ];

/* global count */
volatile uint16_t rx_w = 0;
volatile uint16_t rx_r = 0;
volatile uint32_t g_tx_cnt = 0;
volatile uint32_t g_rx_cnt = 0;

/*FUNCTION**********************************************************************
 *
 * Function Name : rx_push
 * Description   : push data to rx buffer
 *
 *END**************************************************************************/
void rx_push(uint8_t b)
{
    uint16_t next = (uint16_t)((rx_w + 1u) % RX_BUF_SZ);
    if (next != rx_r)
    {
        rx_buf[rx_w] = b;
        rx_w = next;
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : rx_pop
 * Description   : pop data out of rx buffer
 *
 *END**************************************************************************/
uint8_t rx_pop(uint8_t *out)
{
    if (rx_r == rx_w)
    {
        return 0;  /* empty */
    }
    *out = rx_buf[rx_r];
    rx_r = (uint16_t)((rx_r + 1u) % RX_BUF_SZ);
    return 1;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : ARM_USART_PowerControl
 * Description   : Choose PowerMode
 *
 *END**************************************************************************/
int32_t ARM_USART_PowerControl(ARM_POWER_STATE state)
{
    STATUS_ERROR_CODE status = ARM_DRIVER_OK;
    switch (state)
    {
    case ARM_POWER_OFF:
        break;
    case ARM_POWER_LOW:
        break;
    case ARM_POWER_FULL:
        break;
    default:
        status = ARM_DRIVER_ERROR_UNSUPPORTED;
        break;
    }
    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : ARM_USART_Initialize
 * Description   : Initialize UART (clock, NVIC, Pin Mux)
 *
 *END**************************************************************************/
int32_t ARM_USART_Initialize(ARM_UART_SignalEvent_t callback)
{
    UART1_callback = callback;

    if ((IP_SCG->FIRCCSR & SCG_FIRCCSR_FIRCEN_MASK) == 0u) {
        IP_SCG->FIRCCSR |= SCG_FIRCCSR_FIRCEN_MASK;
        while ((IP_SCG->FIRCCSR & SCG_FIRCCSR_FIRCVLD_MASK) == 0u) { }
    }
    /* DIV2 = /1 */
    IP_SCG->FIRCDIV = (IP_SCG->FIRCDIV & ~SCG_FIRCDIV_FIRCDIV2_MASK) | SCG_FIRCDIV_FIRCDIV2(1);

    /* PORTC CGC ENABLE */
    IP_PCC->PCCn[PCC_PORTC_INDEX] |= PCC_PCCn_CGC_MASK;

    /* Select FIRC_DIV2 for LPUART1 (PCS=3) + enable CGC */
    IP_PCC->PCCn[PCC_LPUART1_INDEX] &= ~PCC_PCCn_CGC_MASK;
    IP_PCC->PCCn[PCC_LPUART1_INDEX] = (IP_PCC->PCCn[PCC_LPUART1_INDEX] & ~PCC_PCCn_PCS_MASK) |
        PCC_PCCn_PCS(3) | PCC_PCCn_CGC_MASK;

    /* reset UART1 */
    IP_LPUART1->GLOBAL |= LPUART_GLOBAL_RST_MASK;
    IP_LPUART1->GLOBAL &= ~LPUART_GLOBAL_RST_MASK;

    /* Pin mux PTC6/PTC7 ALT2 */
    IP_PORTC->PCR[UART1_RX] = (IP_PORTC->PCR[UART1_RX] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(2);
    IP_PORTC->PCR[UART1_TX] = (IP_PORTC->PCR[UART1_TX] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(2);

    /* enable drive strength */
    IP_PORTC->PCR[UART1_TX] =
        (IP_PORTC->PCR[UART1_TX] & ~PORT_PCR_MUX_MASK) |
        PORT_PCR_MUX(2) |
        PORT_PCR_DSE_MASK;

    /* enable UART1 NVIC */
    S32_NVIC->ICPR[LPUART1_RxTx_IRQn >> 5] = 1UL << (LPUART1_RxTx_IRQn & 31);
    S32_NVIC->ISER[LPUART1_RxTx_IRQn >> 5] = 1UL << (LPUART1_RxTx_IRQn & 31);
    rx_w = 0;
    rx_r = 0;
    return ARM_DRIVER_OK;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : ARM_USART_Uninitialize
 * Description   : Uninitalize UART1
 *
 *END**************************************************************************/
int32_t ARM_USART_Uninitialize(void)
{
    UART1_callback = NULL;
    IP_PCC->PCCn[PCC_LPUART1_INDEX] &= ~PCC_PCCn_CGC_MASK;
    S32_NVIC->ICER[LPUART1_RxTx_IRQn >> 5u] = 1u << (LPUART1_RxTx_IRQn & 31u);
    IP_LPUART1->GLOBAL |= LPUART_GLOBAL_RST_MASK;
    IP_LPUART1->FIFO = (LPUART_FIFO_TXFLUSH_MASK | LPUART_FIFO_RXFLUSH_MASK);
    return ARM_DRIVER_OK;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : ARM_USART_Control
 * Description   : setup UART (baudrate, stop bit, start bit, parity...)
 *
 *END**************************************************************************/
int32_t ARM_USART_Control(uint32_t control, uint32_t arg)
{
    STATUS_ERROR_CODE status = ARM_DRIVER_OK;
    uint32_t cmd = (control & ARM_USART_CONTROL_Pos_Msk);

    if (cmd == ARM_USART_CONTROL_TX)
    {
        if (arg) IP_LPUART1->CTRL |=  LPUART_CTRL_TE_MASK;
        else     IP_LPUART1->CTRL &= ~LPUART_CTRL_TE_MASK;
        return ARM_DRIVER_OK;
    }

    if (cmd == ARM_USART_CONTROL_RX)
    {
        if (arg)
        {
            IP_LPUART1->CTRL |= LPUART_CTRL_RE_MASK;
            IP_LPUART1->CTRL |= LPUART_CTRL_RIE_MASK;
        } else
        {
            IP_LPUART1->CTRL &= ~LPUART_CTRL_RIE_MASK;
            IP_LPUART1->CTRL &= ~LPUART_CTRL_RE_MASK;
        }
        return ARM_DRIVER_OK;
    }

    IP_LPUART1->CTRL &= ~(LPUART_CTRL_RE_MASK | LPUART_CTRL_TE_MASK);

    if (cmd == ARM_USART_MODE_ASYNCHRONOUS)
    {
        IP_LPUART1->BAUD &= ~(LPUART_BAUD_SBR_MASK |
                          LPUART_BAUD_OSR_MASK |
                          LPUART_BAUD_BOTHEDGE_MASK);

        if (arg == 9600)
        {
            IP_LPUART1->BAUD |= LPUART_BAUD_OSR(15) | LPUART_BAUD_SBR(312);
        } else if (arg == 115200)
        {
            IP_LPUART1->BAUD |= LPUART_BAUD_OSR(15) | LPUART_BAUD_SBR(26);
        } else
        {
            return ARM_USART_ERROR_BAUDRATE;
        }
    } else
    {
        return ARM_USART_ERROR_MODE;
    }

    switch (control & ARM_USART_PARITY_Pos_Msk)
    {
        case ARM_USART_PARITY_NONE:
            IP_LPUART1->CTRL &= ~LPUART_CTRL_PE_MASK;
            break;
        case ARM_USART_PARITY_EVEN:
            IP_LPUART1->CTRL |= LPUART_CTRL_PE_MASK;
            IP_LPUART1->CTRL = (IP_LPUART1->CTRL & ~LPUART_CTRL_PT_MASK) | LPUART_CTRL_PT(0);
            break;
        case ARM_USART_PARITY_ODD:
            IP_LPUART1->CTRL |= LPUART_CTRL_PE_MASK;
            IP_LPUART1->CTRL = (IP_LPUART1->CTRL & ~LPUART_CTRL_PT_MASK) | LPUART_CTRL_PT(1);
            break;
        default:
            return ARM_USART_ERROR_PARITY;
    }

    switch (control & ARM_USART_DATA_BITS_Pos_Msk)
    {
        case ARM_USART_DATA_BITS_8:
            IP_LPUART1->CTRL = (IP_LPUART1->CTRL & ~LPUART_CTRL_M7_MASK) | LPUART_CTRL_M7(0);
            IP_LPUART1->CTRL = (IP_LPUART1->CTRL & ~LPUART_CTRL_M_MASK)  | LPUART_CTRL_M(0);
            break;
        default:
            return ARM_USART_ERROR_DATA_BITS;
    }

    switch (control & ARM_USART_STOP_BITS_Pos_Msk)
    {
        case ARM_USART_STOP_BITS_1:
            IP_LPUART1->BAUD = (IP_LPUART1->BAUD & ~LPUART_BAUD_SBNS_MASK) | LPUART_BAUD_SBNS(0);
            break;
        case ARM_USART_STOP_BITS_2:
            IP_LPUART1->BAUD = (IP_LPUART1->BAUD & ~LPUART_BAUD_SBNS_MASK) | LPUART_BAUD_SBNS(1);
            break;
        default:
            return ARM_USART_ERROR_STOP_BITS;
    }
    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : ARM_USART_Send
 * Description   : send a number of data bytes (blocking)
 *
 *END**************************************************************************/
int32_t ARM_USART_Send(const void *data, uint32_t num)
{
    if (!data || num == 0u)
    {
        return ARM_DRIVER_ERROR_PARAMETER;  /* FIX: return immediately */
    }

    const uint8_t *p = (const uint8_t *)data;
    g_tx_cnt = 0;

    /* TX enable */
    IP_LPUART1->CTRL |= LPUART_CTRL_TE_MASK;

    while (g_tx_cnt < num)
    {
        while ((IP_LPUART1->STAT & LPUART_STAT_TDRE_MASK) == 0u) { }
        IP_LPUART1->DATA = p[g_tx_cnt++];
    }

    /* wait for transmit complete */
    while ((IP_LPUART1->STAT & LPUART_STAT_TC_MASK) == 0u) { }

    return ARM_DRIVER_OK;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : ARM_USART_Receive
 * Description   : Receive 1 byte from ring buffer (non-blocking)
 *
 *END**************************************************************************/
int32_t ARM_USART_Receive(void *data, uint32_t num)
{
    if (!data || num != 1u)
    {
        return ARM_DRIVER_ERROR_PARAMETER;  /* FIX: return immediately */
    }

    uint8_t b;
    if (!rx_pop(&b))
    {
        return ARM_DRIVER_ERROR_BUSY;       /* FIX: return immediately */
    }

    *(uint8_t *)data = b;
    return ARM_DRIVER_OK;
}

/** Function***********************************************************************************
 * Function name: ARM_UART_WAIT_TC
 * Description : wait for UART transmit complete
********************************************************************************************/
void ARM_UART_Wait_Tc(void)
{
    while (!(IP_LPUART1->STAT & LPUART_STAT_TC_MASK)) {}
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART1_RxTx_IRQHandler
 * Description   : override UART1_RXTX_IRQHandler
 *
 *END**************************************************************************/
void LPUART1_RxTx_IRQHandler(void)
{
    uint32_t stat = IP_LPUART1->STAT;

    if (stat & LPUART_STAT_RDRF_MASK)
    {
        uint8_t b = (uint8_t)(IP_LPUART1->DATA & 0xFFu);
        rx_push(b);
    }
    /* clear all error flags */
    if (stat & (LPUART_STAT_OR_MASK | LPUART_STAT_NF_MASK |
                LPUART_STAT_FE_MASK | LPUART_STAT_PF_MASK))
    {
        IP_LPUART1->STAT |= (LPUART_STAT_OR_MASK | LPUART_STAT_NF_MASK |
                              LPUART_STAT_FE_MASK | LPUART_STAT_PF_MASK);
    }
}

/* Struct UART driver */
ARM_DRIVER_USART Driver_USART1 =
{
    .Initialize   = ARM_USART_Initialize,
    .Uninitialize = ARM_USART_Uninitialize,
    .PowerControl = ARM_USART_PowerControl,
    .Send         = ARM_USART_Send,
    .Receive      = ARM_USART_Receive,
    .Control      = ARM_USART_Control
};
