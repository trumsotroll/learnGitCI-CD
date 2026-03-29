#ifndef COMMON_DRIVER_H
#define COMMON_DRIVER_H
#include "stdint.h"
#include "../../include/S32K144.h"
#include "../../include/S32K144_features.h"

/**
 * @enum STATUS_ERROR_CODE
 * use this enum to return status of function
 */
typedef enum
{
    ARM_DRIVER_OK                   =  0,
    ARM_DRIVER_ERROR                = -1,
    ARM_DRIVER_ERROR_BUSY           = -2,
    ARM_DRIVER_ERROR_TIMEOUT        = -3,
    ARM_DRIVER_ERROR_UNSUPPORTED    = -4,
    ARM_DRIVER_ERROR_PARAMETER      = -5,
    ARM_DRIVER_ERROR_SPECIFIC       = -6
}STATUS_ERROR_CODE;


#endif /* COMMON_DRIVER_H */
