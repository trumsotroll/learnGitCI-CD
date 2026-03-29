#ifndef FLASH_DRIVER_H
#define FLASH_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

#define SRAM_L_START  0x1FFF8000u
#define SRAM_L_END    0x1FFFFFFFu   /* 0x20000000 - 1 */
#define SRAM_U_START  0x20000000u
#define SRAM_U_END    0x20006FFFu   /* 0x20007000 - 1 */
#define FLASH_START   0x00000000u
#define FLASH_END     0x0007FFFFu   /* 0x00080000 - 1 (512 KB flash memory) */

#define SECTOR_SIZE   0x1000u       /* S32K144 P-Flash sector = 4 KB */
#define PHRASE_SIZE   8u            /* Program-phrase unit = 8 bytes  */


/**
 * @brief Parsed SREC record container.
 *
 * Holds address, data payload and record type
 * extracted from one S-record line.
 */
typedef struct
{
    uint32_t address;      /**< Target address */
    uint8_t  data[256];    /**< Data payload */
    uint32_t data_len;     /**< Number of valid data bytes */
    uint8_t  type;         /**< SREC type (S0–S9) */
} srec_record_t;

/**
 * @brief Check if a value is a valid Main Stack Pointer.
 *
 * @param[in] MSP value to validate.
 * @return true if valid, false otherwise.
 */
bool is_valid_msp(uint32_t);

/**
 * @brief Check whether an address lies in RAM memory.
 *
 * @param[in] Address to check.
 * @return Non-zero if address is in RAM, 0 otherwise.
 */
uint32_t is_in_ram(uint32_t);

/**
 * @brief Check whether an address lies in flash memory.
 *
 * @param[in] Address to check.
 * @return Non-zero if address is in flash, 0 otherwise.
 */
uint32_t is_in_flash(uint32_t);

/**
 * @brief Validate application image at given base address.
 *
 * Typically checks MSP and reset handler validity.
 *
 * @param[in] app_base Application base address.
 * @return Non-zero if valid, 0 otherwise.
 */
uint32_t app_valid(uint32_t app_base);

/**
 * @brief Erase flash sector containing given address.
 *
 * @param[in] addr Address inside the target sector.
 * @return 0 on success, non-zero on error.
 */
uint32_t flash_erase_sector(uint32_t addr);

/**
 * @brief Program one flash phrase (8 bytes).
 *
 * @param[in] addr  Destination flash address (aligned).
 * @param[in] data8 Pointer to 8 bytes of data.
 * @return 0 on success, non-zero on error.
 */
//uint32_t flash_program_phrase(uint32_t addr, const uint8_t data8[8]);

/**
 * @brief Program arbitrary-length buffer to flash.
 *
 * Automatically erases sectors on first write access.
 *
 * @param[in] dest_addr Target flash address.
 * @param[in] data      Source data buffer.
 * @param[in] len       Number of bytes to program.
 *
 * @return 0 on success, non-zero on error.
 */
uint32_t flash_program_buffer(uint32_t dest_addr, const uint8_t *data, uint32_t len);

/**
 * @brief Parse one SREC text line.
 *
 * @param[in]  line Input SREC line.
 * @param[out] rec  Parsed record output.
 *
 * @return
 *  - 0  : data record (S1/S2/S3)
 *  - 1  : end record (S7/S8/S9)
 *  - 2  : ignored record (S0/S5)
 *  - <0 : error
 */
int32_t srec_parse_line(const char *line, srec_record_t *rec);

/**
 * @brief Receive one SREC line via UART (blocking).
 *
 * @param[out] line_buf Buffer to store received line.
 * @param[in]  buf_size Buffer size.
 *
 * @return Line length on success, negative on error.
 */
int32_t srec_receive_line(char *line_buf, uint32_t buf_size);

/**
 * @brief disable irq and save context for restore later
 *
 * @return primask context before disable to save
 */
uint32_t irq_disable_save(void);

/**
 * @brief restore the primask context
 *
 * @param[in] irq context to restore
 */
void irq_restore(uint32_t);

#endif /* FLASH_DRIVER_H */
