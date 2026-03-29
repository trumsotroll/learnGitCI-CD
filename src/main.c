#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "S32K144.h"
#include "system_S32K144.h"
#include "S32K144_features.h"
#include "s32_core_cm4.h"

#include "UART_driver.h"
#include "GPIO_driver.h"

#include "jump_to_app.h"
#include "flash_driver.h"

/* -------------------------------------------------------------------------- */
/*  Configuration Flash sector                                                           */
/* -------------------------------------------------------------------------- */
#define APP_START        0x0000A000u
#define SECTOR_SIZE      0x1000u
#define APP_MAX_SIZE     (FLASH_END - APP_START)
#define ERASED_MAP_WORDS ((APP_MAX_SIZE / SECTOR_SIZE + 31u) / 32u)
#define LINE_BUF_SIZE    256u
#define FLASH_START 0x00000000u
#define FLASH_END  0x00080000u

/* -------------------------------------------------------------------------- */
/*  Extern drivers                                                            */
/* -------------------------------------------------------------------------- */
extern ARM_DRIVER_USART Driver_USART1;
static ARM_DRIVER_USART *UART1 = &Driver_USART1;

/** Function***********************************************************************************
 * Function name: uart_puts
 * Description : uart put a string to hercules terminal
********************************************************************************************/
static void uart_puts(const char *s)
{
    UART1->Send(s, (uint32_t)strlen(s));
}

/** Function***********************************************************************************
 * Function name: uart_put_hex32
 * Description : Print a 32-bit value as 8 hex digits
********************************************************************************************/
static void uart_put_hex32(uint32_t val)
{
    static const char hex[] = "0123456789ABCDEF";
    char buf[9];
    for (int i = 7; i >= 0; i--)
    {
        buf[i] = hex[val & 0xFu];
        val >>= 4;
    }
    buf[8] = '\0';
    uart_puts(buf);
}

/** Function***********************************************************************************
 * Function name: delay_loop
 * Description : delay blocking for led toggling
********************************************************************************************/
static void delay_loop(volatile uint32_t count)
{
    while (count > 0u) count--;
}

/** Function***********************************************************************************
 * Function name: GPIO_Init
 * Description : Init GPIO for LED indicator
********************************************************************************************/
static void GPIO_Init(void)
{
    GPIO_SetOutput(PORTD_PIN15, HIGH);
    GPIO_SetInput(PORTC_PIN12, PULLDOWN);
}

/** Function***********************************************************************************
 * Function name: UART_Init
 * Description : Init UART to communicate and transmit firmware
********************************************************************************************/
static void UART_Init(void)
{
    UART1->Initialize(NULL);
    UART1->Control(ARM_USART_MODE_ASYNCHRONOUS |
                   ARM_USART_PARITY_NONE       |
                   ARM_USART_STOP_BITS_1       |
                   ARM_USART_DATA_BITS_8, 115200);

    UART1->Control(ARM_USART_CONTROL_TX, 1);
    UART1->Control(ARM_USART_CONTROL_RX, 1);
}

/** Function***********************************************************************************
 * Function name: bootloader_run
 * Description : run bootloader logic (allow erase flash or not)
********************************************************************************************/
static void bootloader_run(bool allow_erase)
{
	char line_buf[LINE_BUF_SIZE];
	srec_record_t rec;
	if(allow_erase == false)
	{
		uart_puts("Ready to receive firmware\r\n");
		uart_puts("--------------------------------------------------------\r\n");
	}else
	{
		uart_puts("Ready to erase Application section\r\n");
		uart_puts("Erasing Application section...\r\n");
		if (flash_erase_app(APP_START) != 0u)
		{
			uart_puts("Erase error\r\n");
			return;
		}
		uart_puts("Erase done: Go to Listen mode\r\n");
		uart_puts("--------------------------------------------------------\r\n");
	}
    /* receive + program */
    while (1)
    {
        int len = srec_receive_line(line_buf, LINE_BUF_SIZE);
        if (len < 0)
        {
        	uart_puts("UART receive error.\r\n");
        	return;
        }
        if(len == 0)
		{
			continue; /* empty srec line -> skip line */
		}

        int r = srec_parse_line(line_buf, &rec);
        if (r == 2)
		{
			continue; /* S0/S5 skip */
		}

        if (r == 1)
        {
        	break; /* S7/S8/S9 end of srec file */
        }
        if (r < 0)
        {
        	if (r == -3)
			{
				uart_puts("Checksum error.\r\n");
			}else if(r == -1)
			{
				uart_puts("Not SREC format.\r\n");
			}else /* r == -2 */
			{
				uart_puts("Unsupported type.");
			}
			return;  /* stop */
        }
        if (rec.address < APP_START || rec.address + rec.data_len > FLASH_END)
        {
            uart_puts("Address error\r\n");
            continue;
        }
        if (flash_program_buffer(rec.address, rec.data, rec.data_len) != 0u)
        {
            uart_puts("Flash error\r\n");
            return;
        }
    }
    uart_puts("Receive done.\r\n");
    if (app_valid(APP_START) == 1)
    {
        uart_puts("APP_VALID\r\n");
        GPIO_SetLow(PORTD_PIN15);
        uart_puts("Press RESET BUTTON to start\r\n");
        uart_puts("--------------------------------------------------------\r\n");
        ARM_UART_Wait_Tc();
        while(1);
    }
    else
    {
        uart_puts("ERROR: Application invalid\r\n");
    }
}

/** Function***********************************************************************************
 * Function name: error_blink_loop
 * Description : this function blink red led to indicate an error
********************************************************************************************/
static void error_blink(void)
{
    while (1)
    {
    	GPIO_SetToggle(PORTD_PIN15);
        delay_loop(200000);
    }
}

/** Function***********************************************************************************
 * Function name: main
 * Description : main function of bootloader
********************************************************************************************/
int main(void)
{
    GPIO_Init();
    UART_Init();

    uart_puts("\r\n------------------ S32K144 BOOTLOADER ------------------\r\n");

    /* Print current app status */
    uint32_t app_msp = *(volatile uint32_t *)(APP_START + 0u);
    uint32_t app_rst = *(volatile uint32_t *)(APP_START + 4u);

    uart_puts("Bootloader from 0x"); /* app address */
	uart_put_hex32(FLASH_START);
    uart_puts("\r\nApplication from 0x"); /* app address */
    uart_put_hex32(APP_START);
    uart_puts("\r\nMain Stack pointer = 0x"); /* MSP address */
    uart_put_hex32(app_msp);
    uart_puts("\r\nReset handler = 0x"); /* reset handler address */
    uart_put_hex32(app_rst);
    uart_puts("\r\n");

    /* Read boot button */
    uint32_t Boot_button = GPIO_GetInput(PORTC_PIN12);
    uart_puts("BootMode = ");
    if(Boot_button == 0)
    {
    	uart_puts("OFF");
    }else
    {
    	uart_puts("ON");
    }
    uart_puts("\r\n");

    /* ---- Button pressed -> bootloader mode ---- */
    if (Boot_button == 0)
    {
        uart_puts("Button pressed: go to Boot\r\n");
        bootloader_run(true); /* bootloader erase APP */
        while(1);
    }
    /* Check APP valid */
    if (app_valid(APP_START))
    {
        uart_puts("App valid: jump into application\r\n");
        uart_puts("--------------------------------------------------------\r\n");
        jump_to_app(APP_START);
    }
    /* auto bootloader: not erase flash */
    uart_puts("No valid application: goto Listen mode\r\n");
    bootloader_run(false); /* Bootloader not erase APP */
    error_blink();

    return 0;
}

