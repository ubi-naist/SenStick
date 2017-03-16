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

/** @file
 * @defgroup qspi_example_main main.c
 * @{
 * @ingroup qspi_example
 *
 * @brief QSPI Example Application main file.
 *
 * This file contains the source code for a sample application using QSPI.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "nrf_drv_qspi.h"
#include "nrf_delay.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "boards.h"
#define NRF_LOG_MODULE_NAME "APP"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "sdk_config.h"

#define QSPI_STD_CMD_WRSR   0x01
#define QSPI_STD_CMD_RSTEN  0x66
#define QSPI_STD_CMD_RST    0x99

#define QSPI_TEST_DATA_SIZE 256

#define WAIT_FOR_PERIPH() do { \
        while (!m_finished) {} \
        m_finished = false;    \
    } while(0)

static volatile bool m_finished = false;
static uint8_t m_buffer_tx[QSPI_TEST_DATA_SIZE];
static uint8_t m_buffer_rx[QSPI_TEST_DATA_SIZE];

static void qspi_handler(nrf_drv_qspi_evt_t event, void * p_context)
{
    UNUSED_PARAMETER(event);
    UNUSED_PARAMETER(p_context);
    m_finished = true;
}

static void configure_memory()
{
    uint8_t temporary = 0x40;
    uint32_t err_code;
    nrf_qspi_cinstr_conf_t cinstr_cfg = {
        .opcode    = QSPI_STD_CMD_RSTEN,
        .length    = NRF_QSPI_CINSTR_LEN_1B,
        .io2_level = true,
        .io3_level = true,
        .wipwait   = true,
        .wren      = true
    };

    // Send reset enable
    err_code = nrf_drv_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    // Send reset command
    cinstr_cfg.opcode = QSPI_STD_CMD_RST;
    err_code = nrf_drv_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    // Switch to qspi mode
    cinstr_cfg.opcode = QSPI_STD_CMD_WRSR;
    cinstr_cfg.length = NRF_QSPI_CINSTR_LEN_2B;
    err_code = nrf_drv_qspi_cinstr_xfer(&cinstr_cfg, &temporary, NULL);
    APP_ERROR_CHECK(err_code);
}


int main(void)
{
    uint32_t i;
    uint32_t err_code;

    err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_INFO("\r\n"
                 "QSPI write and read example using 24bit addressing mode\r\n");

    srand(0);
    for (i = 0; i < QSPI_TEST_DATA_SIZE; ++i)
    {
        m_buffer_tx[i] = (uint8_t)rand();
    }

    nrf_drv_qspi_config_t config = NRF_DRV_QSPI_DEFAULT_CONFIG;

    err_code = nrf_drv_qspi_init(&config, qspi_handler, NULL);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("QSPI Initialized\r\n");

    configure_memory();

    m_finished = false;
    err_code = nrf_drv_qspi_erase(NRF_QSPI_ERASE_LEN_64KB, 0);
    APP_ERROR_CHECK(err_code);
    WAIT_FOR_PERIPH();
    NRF_LOG_INFO("Process of erasing first block start\r\n");

    err_code = nrf_drv_qspi_write(m_buffer_tx, QSPI_TEST_DATA_SIZE, 0);
    APP_ERROR_CHECK(err_code);
    WAIT_FOR_PERIPH();
    NRF_LOG_INFO("Process of writing data start\r\n");

    err_code = nrf_drv_qspi_read(m_buffer_rx, QSPI_TEST_DATA_SIZE, 0);
    WAIT_FOR_PERIPH();
    NRF_LOG_INFO("Data read\r\n");

    NRF_LOG_INFO("Compare...\r\n");
    if (memcmp(m_buffer_tx, m_buffer_rx, QSPI_TEST_DATA_SIZE) == 0)
    {
        NRF_LOG_INFO("Data consistent\r\n");
    }
    else
    {
        NRF_LOG_INFO("Data inconsistent\r\n");
    }

    nrf_drv_qspi_uninit();

    for (;;)
    {
    }
}

/** @} */
