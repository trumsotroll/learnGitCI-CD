#ifndef JUMP_TO_APP_H
#define JUMP_TO_APP_H

/** @brief Data Synchronization Barrier */
void __DSB(void);

/** @brief Instruction Synchronization Barrier */
void __ISB(void);

/**
 * @brief Set Main Stack Pointer (MSP)
 * @param[in] topOfMainStack New MSP value
 */
void __set_MSP(uint32_t topOfMainStack);

/**
 * @brief Jump to application firmware
 * @param[in] app_addr Application vector table address
 * @warning This function does not return
 */
void jump_to_app(uint32_t app_addr);

/**
 * @brief Erase application flash area
 * @param[in] start_addr Application start address
 * @return 0u on success, 0xFFFFFFFF on failure
 */
uint32_t flash_erase_app(uint32_t start_addr);

#endif /* JUMP_TO_APP_H */
