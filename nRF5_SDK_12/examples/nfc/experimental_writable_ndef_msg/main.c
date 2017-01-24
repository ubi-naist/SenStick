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
 *
 * @defgroup nfc_writable_ndef_msg_example_main main.c
 * @{
 * @ingroup nfc_writable_ndef_msg_example
 * @brief The application main file of NFC writable NDEF message example.
 *
 */
#include <stdint.h>
#include <stdbool.h>
#include "app_error.h"
#include "boards.h"
#include "nfc_t4t_lib.h"
#include "nfc_uri_msg.h"
#include "nrf_log_ctrl.h"

static const uint8_t m_url[] =
    {'n', 'o', 'r', 'd', 'i', 'c', 's', 'e', 'm', 'i', '.', 'c', 'o', 'm'}; // URL "nordicsemi.com"

uint8_t       m_ndef_msg_buf[NDEF_FILE_SIZE];                               // Buffer for NDEF file
volatile bool m_update_state;                                               // Flag indicating that Type 4 Tag performs NDEF message update procedure.

/**
 * @brief Callback function for handling NFC events.
 */
static void nfc_callback(void          * context,
                         nfc_t4t_event_t event,
                         const uint8_t * data,
                         size_t          dataLength,
                         uint32_t        flags)
{
    (void)context;

    switch (event)
    {
        case NFC_T4T_EVENT_FIELD_ON:
            bsp_board_led_on(BSP_BOARD_LED_0);
            break;

        case NFC_T4T_EVENT_FIELD_OFF:
            bsp_board_leds_off();
            m_update_state = false;
            break;

        case NFC_T4T_EVENT_NDEF_READ:
            bsp_board_led_on(BSP_BOARD_LED_3);
            break;

        case NFC_T4T_EVENT_NDEF_UPDATED:
            if (dataLength == 0)
            {
                m_update_state = true;
            }
            else if (m_update_state == true)
            {
                m_update_state = false;
                bsp_board_led_on(BSP_BOARD_LED_1);
            }
            break;

        default:
            break;
    }
}


/**
 * @brief Function for application main entry.
 */
int main(void)
{
    uint32_t err_code;

    err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    /* Configure LED-pins as outputs */
    bsp_board_leds_init();

    /* Set up NFC */
    err_code = nfc_t4t_setup(nfc_callback, NULL);
    APP_ERROR_CHECK(err_code);

    /* Provide information about available buffer size to encoding function */
    uint32_t len = sizeof(m_ndef_msg_buf);

    /* Encode URI message into buffer */
    err_code = nfc_uri_msg_encode(NFC_URI_HTTP_WWW,
                                  m_url,
                                  sizeof(m_url),
                                  m_ndef_msg_buf,
                                  &len);
    APP_ERROR_CHECK(err_code);

    /* Run Read-Write mode for Type 4 Tag platform */
    err_code = nfc_t4t_ndef_rwpayload_set(m_ndef_msg_buf, sizeof(m_ndef_msg_buf));
    APP_ERROR_CHECK(err_code);

    /* Start sensing NFC field */
    err_code = nfc_t4t_emulation_start();
    APP_ERROR_CHECK(err_code);

    while (1)
    {
        NRF_LOG_FLUSH();
    }
}


/** @} */
