#include <stdint.h>
#include <string.h>
#include "S32K144.h"
#include "flash_driver.h"
#include "UART_driver.h"
#include "common_driver.h"
#include "s32_core_cm4.h"

/* -------------------------------------------------------------------------- */
/*  RAM / Flash address ranges (from linker script)                           */
/* -------------------------------------------------------------------------- */
#define SRAM_L_START  0x1FFF8000u
#define SRAM_L_END    0x20000000u   /* end-exclusive */
#define SRAM_U_START  0x20000000u
#define SRAM_U_END    0x20007000u   /* end-exclusive */
#define FLASH_END     0x00080000u   /* 512 KB */

#define SECTOR_SIZE   0x1000u       /* S32K144 P-Flash sector = 4 KB */
#define PHRASE_SIZE   8u            /* Program-phrase unit = 8 bytes  */

/** Function -----------------------------------------------------------------
*  Function name: irq_disable_save
*  Description: Must disable interrupts during flash erase/program because
*  the UART ISR code lives in flash. If an interrupt fires while flash is
*  busy, the CPU tries to fetch ISR from flash → read collision → HardFault.
* -------------------------------------------------------------------------- */
inline uint32_t irq_disable_save(void)
{
    uint32_t primask;
    __asm volatile ("mrs %0, primask" : "=r" (primask));
    __asm volatile ("cpsid i" ::: "memory");
    return primask;
}

/** Function -----------------------------------------------------------------
*  Function name: irq_restore
*  Description: restore the irq interrupt
* -------------------------------------------------------------------------- */
inline void irq_restore(uint32_t primask)
{
    __asm volatile ("msr primask, %0" :: "r" (primask) : "memory");
}

/** Function -----------------------------------------------------------------
*  Function name: is_in_ram
*  Description: check if x (Stack pointer) is in RAM address or not
* -------------------------------------------------------------------------- */
__attribute__((section(".code_ram")))
uint32_t is_in_ram(uint32_t x)
{
    if (x >= SRAM_L_START && x <= SRAM_L_END)
    {
    	return 1;
    }
    if (x >= SRAM_U_START && x <= SRAM_U_END)
    {
    	return 1;
    }
    return 0;
}

/** Function -----------------------------------------------------------------
*  Function name: app_valid
*  Description: Validate application vector table
* -------------------------------------------------------------------------- */
__attribute__((section(".code_ram")))
uint32_t is_in_flash(uint32_t x)
{
    return (x < FLASH_END);
}

/** Function -----------------------------------------------------------------
*  Function name: app_valid
*  Description: Validate application vector table
* -------------------------------------------------------------------------- */
__attribute__((section(".code_ram")))
uint32_t app_valid(uint32_t app_base)
{
    uint32_t MainStackPointer = *(uint32_t *)(app_base + 0u);
    uint32_t AppResetHandler = *(uint32_t *)(app_base + 4u);

    if (!is_in_ram(MainStackPointer)) /* check MSP in RAM_L or RAM_U */
    {
    	return 0;
    }
    if (AppResetHandler < app_base || AppResetHandler >= FLASH_END) /* Reset handler must in valid range */
    {
    	return 0;
    }
    if ((AppResetHandler & 1u) == 0u) /* check Thumb bit */
    {
    	return 0;
    }
    /* if all requirement are valid, return 1 */
    return 1;
}

/** Function -----------------------------------------------------------------
*  Function name: flash_erase_app
*  Description: erase the application section in flash
* -------------------------------------------------------------------------- */
__attribute__((section(".code_ram")))
uint32_t flash_erase_app(uint32_t app_base)
{
    for (uint32_t addr = app_base; addr < FLASH_END; addr += SECTOR_SIZE)
    {
        if (flash_erase_sector(addr) != 0u)
        {
        	return (uint32_t)(-1); /* 0xFFFFFFFF */
        }
    }
    return 0;	/* erase app completely */
}

/** Function -----------------------------------------------------------------
*  Function name: ftfc_wait_ccif
*  Description: wait for flash controller busy
* -------------------------------------------------------------------------- */
__attribute__((section(".code_ram")))
static inline void ftfc_wait_ccif(void)
{
    while ((IP_FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) == 0u) { }
}

/** Function -----------------------------------------------------------------
*  Function name: ftfc_clear_errors
*  Description: clear error of flash controller
* -------------------------------------------------------------------------- */
__attribute__((section(".code_ram")))
static inline void ftfc_clear_errors(void)
{
    IP_FTFC->FSTAT = FTFC_FSTAT_ACCERR_MASK |FTFC_FSTAT_FPVIOL_MASK | FTFC_FSTAT_RDCOLERR_MASK;
}

/** Function -----------------------------------------------------------------
*  Function name: ftfc_check_errors
*  Description: Check error of flash controller
* -------------------------------------------------------------------------- */
__attribute__((section(".code_ram")))
static inline uint32_t ftfc_check_errors(void)
{
    return IP_FTFC->FSTAT & (FTFC_FSTAT_ACCERR_MASK | FTFC_FSTAT_FPVIOL_MASK | FTFC_FSTAT_RDCOLERR_MASK);
}

/** Function -----------------------------------------------------------------
*  Function name: flash_erase_sector
*  Description:
*  Erase one 4 KB sector at the specific address
*  INTERRUPTS ARE DISABLED during the entire flash operation.
* -------------------------------------------------------------------------- */
__attribute__((section(".code_ram")))
uint32_t flash_erase_sector(uint32_t addr)
{
    uint32_t primask = irq_disable_save();

    /* wait for previous command execute */
    ftfc_wait_ccif();
    /* clear previous flash error */
    ftfc_clear_errors();

     IP_FTFC->FCCOB[3] = 0x09u; /* Erase Flash Sector command */
    /* flash 24 bit address */
    IP_FTFC->FCCOB[2] = (uint8_t)((addr >> 16) & 0xFFu);
    IP_FTFC->FCCOB[1] = (uint8_t)((addr >>  8) & 0xFFu);
    IP_FTFC->FCCOB[0] = (uint8_t)((addr >>  0) & 0xFFu);

    /* flash execute command */
    IP_FTFC->FSTAT = FTFC_FSTAT_CCIF_MASK;
    /* wait for previous command execute */
    ftfc_wait_ccif();

    /* read the flash error */
    uint32_t err = ftfc_check_errors();

    irq_restore(primask);

    if (err != 0)
    {
        return (uint32_t)(-1); /* 0xFFFFFFFF */
    }
    else
    {
        return 0; /* OK */
    }
}

/* Function -----------------------------------------------------------------
*  Function name: flash_program_phrase
*  Description:
*  Program one 8-byte phrase
*  S32K144 FCCOB byte ordering (big-endian within each 32-bit word):
*    Word 0: FCCOB[3]=command  FCCOB[2]=addr_H  FCCOB[1]=addr_M  FCCOB[0]=addr_L
*    Word 1: FCCOB[7]=d0   FCCOB[6]=d1      FCCOB[5]=d2      FCCOB[4]=d3
*    Word 2: FCCOB[11]=d4  FCCOB[10]=d5     FCCOB[9]=d6      FCCOB[8]=d7
* -------------------------------------------------------------------------- */
__attribute__((section(".code_ram")))
uint32_t flash_program_phrase(uint32_t addr, const uint8_t data8[8])
{
    if (addr & 7u)
	{
		return (uint32_t)(-2);
	}
    /* disable irq, save context to restore later */
    uint32_t primask = irq_disable_save();

    /* wait to flash bus */
    ftfc_wait_ccif();
    /* clear error from the previous flashing */
    ftfc_clear_errors();

    /* program phrase command */
    IP_FTFC->FCCOB[3] = 0x07u;
    /* Flash address 24 bits (3 bytes) */
    IP_FTFC->FCCOB[2] = (uint8_t)((addr >> 16) & 0xFFu);
    IP_FTFC->FCCOB[1] = (uint8_t)((addr >>  8) & 0xFFu);
    IP_FTFC->FCCOB[0] = (uint8_t)((addr >>  0) & 0xFFu);

    /* data mapping FCCOB[4+i] = data8[i] */
    IP_FTFC->FCCOB[4]  = data8[0];
    IP_FTFC->FCCOB[5]  = data8[1];
    IP_FTFC->FCCOB[6]  = data8[2];
    IP_FTFC->FCCOB[7]  = data8[3];
    IP_FTFC->FCCOB[8]  = data8[4];
    IP_FTFC->FCCOB[9]  = data8[5];
    IP_FTFC->FCCOB[10] = data8[6];
    IP_FTFC->FCCOB[11] = data8[7];

    /* Activate command */
    IP_FTFC->FSTAT = FTFC_FSTAT_CCIF_MASK;
    /* wait for flash execute command */
    ftfc_wait_ccif();

    /* check the flash error */
    uint32_t err = ftfc_check_errors();

    /* restore the irq context */
    irq_restore(primask);

    if (err)
    {
        return (uint32_t)(-1); /* 0xFFFFFFFF */
    }
    else
    {
        return 0; /* OK */
    }
}

/** Function -----------------------------------------------------------------
*  Function name: flash_program_buffer
*  Description:
*  Program arbitrary buffer to flash with auto-erase
*  `erased_map`: caller-owned bitmap. bit N set = sector N already erased.
*  Sector N starts at (app_base + N * 4 KB).
*  Returns 0 on success, negative on error.
* -------------------------------------------------------------------------- */
__attribute__((section(".code_ram")))
uint32_t flash_program_buffer(uint32_t addr, const uint8_t *data, uint32_t len)
{
    uint8_t phrase[8];
    uint32_t i = 0;
    while (i < len)
    {

        uint32_t paddr = (addr + i) & ~0x7u;
        /* read old flash */
        memcpy(phrase, (void *)paddr, 8);
        /* overlay new data */
        for (uint32_t j = 0; j < 8; j++)
        {
            uint32_t cur = paddr + j;
            if (cur >= addr && cur < addr + len)
            {
                phrase[j] = data[cur - addr];
            }
        }
        /* clear error flags */
        IP_FTFC->FSTAT = FTFC_FSTAT_ACCERR_MASK | FTFC_FSTAT_FPVIOL_MASK;
        /* program phrase */
        if (flash_program_phrase(paddr, phrase) != 0)
		{
			return 1;
		}
        i += 8;
    }
    return 0;
}

/** Function -----------------------------------------------------------------
*  Function name: hex_to_u8
*  Description:
*  SREC PARSER (srec_record_t defined in flash_driver.h)
* -------------------------------------------------------------------------- */
static uint8_t hex_to_u8(char c)
{
    if (c >= '0' && c <= '9') return (uint8_t)(c - '0');
    if (c >= 'A' && c <= 'F') return (uint8_t)(c - 'A' + 10);
    if (c >= 'a' && c <= 'f') return (uint8_t)(c - 'a' + 10);
    return 0;
}

/** Function -----------------------------------------------------------------
*  Function name: hex_byte
*  Description: create 1 byte from 2 hexadecimal
* -------------------------------------------------------------------------- */
static uint8_t hex_byte(const char *s)
{
    uint8_t high;
    uint8_t low;
    uint8_t result;

    high = hex_to_u8(s[0]);

    low  = hex_to_u8(s[1]);

    high = (uint8_t)(high << 4);

    result = (uint8_t)(high | low);

    return result;
}

/** Function -----------------------------------------------------------------
*  Function name: srec_parse_line
*  Description: Parse one SREC line.
*  Returns:
*   0  = data record (S1/S2/S3) parsed OK — rec->address/data/data_len filled
*   1  = end record (S7/S8/S9) — firmware transfer complete
*   2  = S0 header record — skip, no data to program
*  -1  = not an S-record
*  -2  = unsupported type
*  -3  = checksum error
* -------------------------------------------------------------------------- */
__attribute__((section(".code_ram")))
int32_t srec_parse_line(const char *line, srec_record_t *rec)
{
	/* check first character SREC line */
    if (line[0] != 'S')
    {
    	return -1;
    }
    /* decode the SREC type (0-9) */
    rec->type = (uint8_t)(line[1] - '0');

    uint8_t  count         = hex_byte(&line[2]);
    uint32_t idx           = 4;
    uint32_t checksum_calc = count;
    uint32_t addr_len      = 0;

    switch (rec->type)
    {
        case 0:  return 2;            /* S0 header — skip */
        case 1:  addr_len = 2; break; /* 16-bit address */
        case 2:  addr_len = 3; break; /* 24-bit address */
        case 3:  addr_len = 4; break; /* 32-bit address */
        case 5:  return 2;     break; /* S5 record count — skip */
        case 7:  return 1;	   break; /* end-of-file */
        case 8:  return 1;	   break; /* end-of-file */
        case 9:  return 1;     break; /* end-of-file */
        default: return -2; 		  /* unsupported type */
    }

    rec->address = 0;

    /* combine address */
    for (uint32_t i = 0; i < addr_len; i++)
    {
        uint8_t b = hex_byte(&line[idx]);
        idx += 2;
        checksum_calc += b;
        rec->address = (rec->address << 8) | b;
    }

    /* data length = count - address length - 1 checksum byte */
    rec->data_len = count - addr_len - 1u;

    /* combine two hexadecimal characters into one byte */
    for (uint32_t i = 0; i < rec->data_len; i++)
    {
        uint8_t b = hex_byte(&line[idx]);
        idx += 2;
        checksum_calc += b;
        rec->data[i] = b;
    }

    /* combine two hexadecimal checksum to 1 byte */
    uint8_t checksum = hex_byte(&line[idx]);

    /* calculate checksum = 0xFF - (~sum & 0xFF) */
    checksum_calc = (~checksum_calc) & 0xFFu;

    /* checksum check */
    if (checksum_calc != checksum)
    {
    	return -3; /* error checksum */
    }
    return 0; /* OK */
}

/** Function -----------------------------------------------------------------
*  Function name: srec_receive_line
*  Description: Receive one SREC line via UART (blocking, spin on ring buffer)
* -------------------------------------------------------------------------- */
__attribute__((section(".code_ram")))
int32_t srec_receive_line(char *line_buf, uint32_t buf_size)
{
    int32_t idx = 0;
    uint8_t receive_char;

    /* check buffer: NULL/size 0 => return error */
    if (!line_buf || buf_size == 0)
    {
    	return -1;
    }
    while (1)
    {
        if (ARM_USART_Receive(&receive_char, 1) != ARM_DRIVER_OK)
        {
            continue;   /* polling until byte available */
        }
        /* skip character \r */
        if (receive_char == '\r')
        {
        	continue;
        }
        /* replace character \n with C end string*/
        if (receive_char == '\n')
        {
            line_buf[idx] = '\0';
            return idx; /* return length */
        }
        if (idx >= (buf_size - 1u))
        {
        	return -2;  /* line too long */
        }
        line_buf[idx++] = (char)receive_char;
    }
}
