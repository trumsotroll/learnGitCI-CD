#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#include "stdint.h"
#include "stdio.h"

/** define function pointer for callback funtion of UART */
typedef void (*ARM_UART_SignalEvent_t) (uint32_t event);

/** BIT FIELD MAP
* --|  1 | 1  |     2     | 2  |   2  | 3  |  8    |
* --|CPHA|CPOL|FLOWCONTROL|STOP|PARITY|DATA|CONTROL|
*/
#define ARM_USART_CONTROL_Pos           0
#define ARM_USART_CONTROL_Pos_Msk       (0xFFUL << ARM_USART_CONTROL_Pos)

#define ARM_USART_DATA_BITS_Pos         8
#define ARM_USART_DATA_BITS_Pos_Msk     (7UL << ARM_USART_DATA_BITS_Pos)

#define ARM_USART_PARITY_Pos            12
#define ARM_USART_PARITY_Pos_Msk        (3UL << ARM_USART_PARITY_Pos)

#define ARM_USART_STOP_BITS_Pos         14
#define ARM_USART_STOP_BITS_Pos_Msk     (3UL << ARM_USART_STOP_BITS_Pos)

#define ARM_USART_FLOW_CONTROL_Pos      16
#define ARM_USART_FLOW_CONTROL_Pos_Msk  (3UL << ARM_USART_FLOW_CONTROL_Pos)

#define ARM_USART_CPOL_Pos              18
#define ARM_USART_CPOL_Pos_Msk          (1UL << ARM_USART_CPOL_Pos)

#define ARM_USART_CPHA_Pos              19
#define ARM_USART_CPHA_Pos_Msk          (1UL << ARM_USART_CPHA_Pos)

#define ARM_USART_MODE_ASYNCHRONOUS         (0x01UL << ARM_USART_CONTROL_Pos)
#define ARM_USART_MODE_SYNCHRONOUS_MASTER   (0x02UL << ARM_USART_CONTROL_Pos)
#define ARM_USART_MODE_SYNCHRONOUS_SLAVE    (0x03UL << ARM_USART_CONTROL_Pos)
#define ARM_USART_CONTROL_TX                (0x04UL << ARM_USART_CONTROL_Pos)
#define ARM_USART_CONTROL_RX                (0x05UL << ARM_USART_CONTROL_Pos)

#define ARM_USART_DATA_BITS_5               (5UL << ARM_USART_DATA_BITS_Pos)
#define ARM_USART_DATA_BITS_6               (6UL << ARM_USART_DATA_BITS_Pos)
#define ARM_USART_DATA_BITS_7               (7UL << ARM_USART_DATA_BITS_Pos)
#define ARM_USART_DATA_BITS_8               (0UL << ARM_USART_DATA_BITS_Pos)
#define ARM_USART_DATA_BITS_9               (1UL << ARM_USART_DATA_BITS_Pos)

#define ARM_USART_PARITY_NONE               (0UL << ARM_USART_PARITY_Pos)
#define ARM_USART_PARITY_EVEN               (1UL << ARM_USART_PARITY_Pos)
#define ARM_USART_PARITY_ODD                (2UL << ARM_USART_PARITY_Pos)

#define ARM_USART_STOP_BITS_1               (0UL << ARM_USART_STOP_BITS_Pos)
#define ARM_USART_STOP_BITS_2               (1UL << ARM_USART_STOP_BITS_Pos)

/**
 * @enum UART_STATUS_ERROR_CODE
 * define status error code
 */
typedef enum
{
    ARM_USART_OK                    =  0,
    ARM_USART_ERROR_MODE            = -1,
    ARM_USART_ERROR_BAUDRATE        = -2,
    ARM_USART_ERROR_DATA_BITS       = -3,
    ARM_USART_ERROR_PARITY          = -4,
    ARM_USART_ERROR_STOP_BITS       = -5,
    ARM_USART_ERROR_FLOW_CONTROL    = -6,
    ARM_USART_ERROR_CPOL            = -7,
    ARM_USART_ERROR_CPHA            = -8
}UART_STATUS_ERROR_CODE;


#define ARM_USART_EVENT_SEND_COMPLETE       (1UL << 0)
#define ARM_USART_EVENT_RECEIVE_COMPLETE    (1UL << 1)
#define ARM_USART_EVENT_TRANSFER_COMPLETE   (1UL << 2)
#define ARM_USART_EVENT_TX_COMPLETE         (1UL << 3)
#define ARM_USART_EVENT_TX_UNDERFLOW        (1UL << 4)
#define ARM_USART_EVENT_RX_OVERFLOW         (1UL << 5)
#define ARM_USART_EVENT_RX_TIMEOUT          (1UL << 6)
#define ARM_USART_EVENT_RX_BREAK            (1UL << 7)
#define ARM_USART_EVENT_RX_FRAMING_ERROR    (1UL << 8)
#define ARM_USART_EVENT_RX_PARITY_ERROR     (1UL << 9)
#define ARM_USART_EVENT_CTS                 (1UL << 10)
#define ARM_USART_EVENT_DSR                 (1UL << 11)
#define ARM_USART_EVENT_DCD                 (1UL << 12)
#define ARM_USART_EVENT_RI                  (1UL << 13)
/**
 * @enum ARM_POWER_STATE
 * define 3 type of power mode
 */
typedef enum
{
    ARM_POWER_OFF   = 0,
    ARM_POWER_LOW   = 1,
    ARM_POWER_FULL  = 2
}ARM_POWER_STATE;

/**
 * @struct ARM_DRIVER_USART
 * define function pointer struct
 */
typedef struct
{
    int32_t (*PowerControl)(ARM_POWER_STATE);
    int32_t (*Initialize)(ARM_UART_SignalEvent_t);
    int32_t (*Uninitialize)(void);
    int32_t (*Control)(uint32_t, uint32_t);
    int32_t (*Send)(const void *, uint32_t);
    int32_t (*Receive)(void*, uint32_t);
}ARM_DRIVER_USART;

/*
 * @brief control powermode of UART
 * @param ARM_POWER_STATE is enum of power mode
 * @return int32_t is status error code
 */
int32_t ARM_USART_PowerControl(ARM_POWER_STATE);
/*
 * @brief this function initialize UART config, assign a callback function
 * @param ARM_UART_SignalEvent_t is the function callback for interrupt
 * @return int32_t is status error code
 */
int32_t ARM_USART_Initialize(ARM_UART_SignalEvent_t);
/*
 * @brief this function Uninitialize UART
 * @return int32_t is status error code
 */
int32_t ARM_USART_Uninitialize(void);
/*
 * @brief this function setup UART configuration
 * @param uint32_t is bit field of config setting
 * @param uint32_t is config value
 * @return int32_t is status error code
 */
int32_t ARM_USART_Control(uint32_t, uint32_t);
/*
 * @brief this function send a data packet
 * @param const void* data is a void pointer to any type of data
 * @param uint32_t is the number of data word
 * @return int32_t is status error code
 */
int32_t ARM_USART_Send(const void *data, uint32_t num);
/*
 * @brief this function reveive a data packet
 * @param void* is a pointer to buffer data receive
 * @param uint32_t is the number of data word to receive
 * @return int32_t is status error code
 */
int32_t ARM_USART_Receive(void *data, uint32_t num);

/*
 * @brief this function to wait the UART transmit complete
 */
void ARM_UART_Wait_Tc(void);

#endif /* UART_DRIVER_H */
