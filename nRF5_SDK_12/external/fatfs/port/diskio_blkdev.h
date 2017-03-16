/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
 
#ifndef DISKIO_SDCARD_H_
#define DISKIO_SDCARD_H_

#include "diskio.h"
#include "nrf_block_dev.h"

#ifdef __cplusplus
extern "C" {
#endif

/**@file
 *
 * @defgroup diskio_blockdev FatFS disk I/O interface based on block device.
 * @{
 *
 * @brief This module implements the FatFs disk API. Internals of this module are based on block device.
 *
 */

/**
 * @brief FatFs disk I/O block device configuration structure.
 * */
typedef struct
{
    const nrf_block_dev_t * p_block_device; ///< Block device associated with a FatFs drive.

    /**
     * @brief FatFs disk interface synchronous wait function.
     *
     * The function to be called repeatedly until the disk I/O operation is completed.
     */
    void (*wait_func)(void);
} diskio_blkdev_config_t;


/**
 * @brief Disk I/O block device.
 * */
typedef struct
{
    diskio_blkdev_config_t config;       ///< Disk I/O configuration.
    nrf_block_dev_result_t last_result;  ///< Result of the last I/O operation.
    volatile DSTATUS state;              ///< Current disk state.
    volatile bool    busy;               ///< Disk busy flag.
} diskio_blkdev_t;

/**
 * @brief Initializer of @ref diskio_blkdev_t.
 *
 * @param blk_device    Block device handle.
 * @param wait_funcion  User wait function (NULL is allowed).
 * */
#define DISKIO_BLOCKDEV_CONFIG(blk_device, wait_funcion)    {   \
    .config = {                                                 \
            .p_block_device = (blk_device),                     \
            .wait_func = (wait_funcion),                        \
    },                                                          \
    .last_result = NRF_BLOCK_DEV_RESULT_SUCCESS,                \
    .state       = STA_NOINIT,                                  \
    .busy        = false                                        \
}

/**
 * @brief FatFs disk initialization.
 *
 * Initializes a block device assigned to a drive number.
 *
 * @param[in] drv Drive number.
 *
 * @return Disk status code.
 * */
DSTATUS disk_initialize(BYTE drv);

/**
 * @brief FatFs disk uninitialization.
 *
 * Uninitializes a block device assigned to a drive number.
 *
 * @param[in] drv Drive index.
 *
 * @return Disk status code.
 * */
DSTATUS disk_uninitialize(BYTE drv);

/**
 * @brief FatFs disk status get.
 *
 * @param[in] drv Drive index.
 *
 * @return Disk status code.
 * */
DSTATUS disk_status(BYTE drv);

/**
 * @brief FatFs disk sector read.
 *
 * @param[in] drv       Drive number.
 * @param[out] buff     Output buffer.
 * @param[in] sector    Sector start number.
 * @param[in] count     Sector count.
 *
 * @return FatFs standard error code.
 * */
DRESULT disk_read(BYTE drv, BYTE* buff, DWORD sector, UINT count);

/**
 * @brief FatFs disk sector write.
 *
 * @param[in] drv       Drive number.
 * @param[in] buff      Input buffer.
 * @param[in] sector    Sector start number.
 * @param[in] count     Sector count.
 *
 * @return FatFs standard error code.
 * */
DRESULT disk_write(BYTE drv, const BYTE* buff, DWORD sector, UINT count);

/**
 * @brief FatFs disk I/O control operation.
 *
 * @param[in] drv   Drive number.
 * @param[in] cmd   I/O control command.
 * @param buff      I/O control parameter (optional).
 *
 * @return FatFs standard error code.
 * */
DRESULT disk_ioctl(BYTE drv, BYTE cmd, void* buff);


/**
 * @brief Registers a block device array.
 *
 * @warning This function must be called before any other function from this header.
 *
 * @param[in] diskio_blkdevs    Disk I/O block device array.
 * @param[in] count             Number of elements in a block device array.
 * */
void diskio_blockdev_register(diskio_blkdev_t * diskio_blkdevs, size_t count);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /*DISKIO_SDCARD_H_*/
