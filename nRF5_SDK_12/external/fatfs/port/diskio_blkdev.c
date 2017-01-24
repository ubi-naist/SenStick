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

#include "diskio_blkdev.h"


/**
 * @brief Registered drives array.
 * */
static diskio_blkdev_t * m_drives;

/**
 * @brief Number of registered drives.
 * */
static BYTE m_drives_count;

/**
 * @brief Block device handler.
 *
 * @ref nrf_block_dev_ev_handler
 * */
static void block_dev_handler(struct nrf_block_dev_s const * p_blk_dev,
                              nrf_block_dev_event_t const *  p_event)
{
    uint8_t drv = (uint8_t)(uint32_t) p_event->p_context;
    ASSERT(drv < m_drives_count);

    switch (p_event->ev_type)
    {
        case NRF_BLOCK_DEV_EVT_INIT:
        case NRF_BLOCK_DEV_EVT_UNINIT:
        case NRF_BLOCK_DEV_EVT_BLK_WRITE_DONE:
        case NRF_BLOCK_DEV_EVT_BLK_READ_DONE:
            m_drives[drv].last_result = p_event->result;
            m_drives[drv].busy = false;
            break;
        default:
            break;
    }
}

/**
 * @brief Default IO operation wait function.
 * */
static void default_wait_func(void)
{
    __WFE();
}

DSTATUS disk_initialize(BYTE drv)
{
    ASSERT(m_drives);

    if (drv >= m_drives_count)
    {
        return (STA_NODISK | STA_NOINIT);
    }

    if (!m_drives[drv].config.p_block_device)
    {
        return (STA_NODISK | STA_NOINIT);
    }

    if (!(m_drives[drv].state & STA_NOINIT))
    {
        // Disk already initialized.
        return m_drives[drv].state;
    }

    if (m_drives[drv].config.wait_func == NULL)
    {
        m_drives[drv].config.wait_func = default_wait_func;
    }

    m_drives[drv].busy = true;
    ret_code_t err_code = nrf_blk_dev_init(m_drives[drv].config.p_block_device,
                                           block_dev_handler,
                                           (void *) (uint32_t) drv);
    if (err_code == NRF_SUCCESS)
    {
        while (m_drives[drv].busy)
        {
            m_drives[drv].config.wait_func();
        }

        if (m_drives[drv].last_result == NRF_BLOCK_DEV_RESULT_SUCCESS)
        {
            m_drives[drv].state &= ~STA_NOINIT;
        }
    }

    return m_drives[drv].state;
}

DSTATUS disk_uninitialize(BYTE drv)
{
    ASSERT(m_drives);

    if (drv >= m_drives_count)
    {
        return (STA_NODISK | STA_NOINIT);
    }

    if (!m_drives[drv].config.p_block_device)
    {
        return (STA_NODISK | STA_NOINIT);
    }

    if (m_drives[drv].state & STA_NOINIT)
    {
        // Disk already uninitialized.
        return m_drives[drv].state;
    }

    (void)nrf_blk_dev_ioctl(m_drives[drv].config.p_block_device,
                            NRF_BLOCK_DEV_IOCTL_REQ_CACHE_FLUSH,
                            NULL);
    ret_code_t ret;
    do
    {
        /*Perform synchronous uninit.*/
        ret = nrf_blk_dev_uninit(m_drives[drv].config.p_block_device);
    } while (ret == NRF_ERROR_BUSY);

    if (ret == NRF_SUCCESS)
    {
        while (m_drives[drv].busy)
        {
            m_drives[drv].config.wait_func();
        }
    }
    if (m_drives[drv].last_result == NRF_BLOCK_DEV_RESULT_SUCCESS)
    {
        m_drives[drv].state |= STA_NOINIT;
    }

    return m_drives[drv].state;
}

DSTATUS disk_status(BYTE drv)
{
    ASSERT(m_drives);

    if (drv >= m_drives_count)
    {
        return STA_NOINIT;
    }
    return m_drives[drv].state;
}

DRESULT disk_read(BYTE drv, BYTE *buff, DWORD sector, UINT count)
{
    ASSERT(m_drives);

    if ((drv >= m_drives_count) || (!count))
    {
        return RES_PARERR;  // Invalid parameter(s).
    }

    if ((m_drives[drv].config.p_block_device == NULL)
        || (m_drives[drv].state & STA_NOINIT))
    {
        return RES_NOTRDY;    // Disk not initialized.
    }

    const nrf_block_req_t req = {
        .p_buff = buff,
        .blk_id = sector,
        .blk_count = count
    };

    m_drives[drv].busy = true;
    ret_code_t err_code = nrf_blk_dev_read_req(m_drives[drv].config.p_block_device, &req);

    if (err_code == NRF_SUCCESS)
    {
        while (m_drives[drv].busy)
        {
            m_drives[drv].config.wait_func();
        }

        if (m_drives[drv].last_result == NRF_BLOCK_DEV_RESULT_SUCCESS)
        {
            return RES_OK;
        }
    }
    return RES_ERROR;
}

DRESULT disk_write(BYTE drv, const BYTE *buff, DWORD sector, UINT count)
{
    ASSERT(m_drives);

    if ((drv >= m_drives_count) || (!count))
    {
        return RES_PARERR;  // Invalid parameter(s).
    }

    if ((m_drives[drv].config.p_block_device == NULL)
        || (m_drives[drv].state & STA_NOINIT))
    {
        return RES_NOTRDY;    // Disk not initialized.
    }

    if (m_drives[drv].state & STA_PROTECT)
    {
        return RES_WRPRT;    // Disk protection is enabled.
    }

    const nrf_block_req_t req = {
        .p_buff = (void *)buff,
        .blk_id = sector,
        .blk_count = count
    };

    m_drives[drv].busy = true;
    ret_code_t err_code = nrf_blk_dev_write_req(m_drives[drv].config.p_block_device, &req);
    if (err_code == NRF_SUCCESS)
    {
        while (m_drives[drv].busy)
        {
            m_drives[drv].config.wait_func();
        }

        if (m_drives[drv].last_result == NRF_BLOCK_DEV_RESULT_SUCCESS)
        {
            return RES_OK;
        }
    }
    return RES_ERROR;
}

DRESULT disk_ioctl(BYTE drv, BYTE cmd, void *buff)
{
    ASSERT(m_drives);

    if (drv >= m_drives_count)
    {
        return RES_PARERR;
    }
    
    switch (cmd)
    {
        case CTRL_SYNC:
        {
            bool flush_in_progress = true;
            do {
                /*Perform synchronous FLUSH operation on block device*/
                ret_code_t ret = nrf_blk_dev_ioctl(m_drives[drv].config.p_block_device,
                                                   NRF_BLOCK_DEV_IOCTL_REQ_CACHE_FLUSH,
                                                   &flush_in_progress);
                if (ret != NRF_SUCCESS && ret != NRF_ERROR_BUSY)
                {
                    break;
                }

            } while (flush_in_progress);

            return RES_OK;
        }
        case GET_SECTOR_COUNT:
        {
            if (m_drives[drv].config.p_block_device == NULL)
            {
                return RES_NOTRDY;
            }

            DWORD * val = buff;
            *val = nrf_blk_dev_geometry(m_drives[drv].config.p_block_device)->blk_count;
            return RES_OK;
        }
        case GET_SECTOR_SIZE:
        {
            if (m_drives[drv].config.p_block_device == NULL)
            {
                return RES_NOTRDY;
            }

            WORD * val = buff;
            *val = nrf_blk_dev_geometry(m_drives[drv].config.p_block_device)->blk_size;
            return RES_OK;
        }
        default:
            break;
    }
    return RES_ERROR;
}

void diskio_blockdev_register(diskio_blkdev_t * diskio_blkdevs, size_t count)
{
    ASSERT(diskio_blkdevs);

    m_drives = diskio_blkdevs;
    m_drives_count = count;
}

