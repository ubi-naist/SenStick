/**
 *
 * @defgroup adafruit_pn532_config Adafruit PN532 implementation for nRF5x configuration
 * @{
 * @ingroup adafruit_pn532
 */
/** @brief Enabling Adafruit PN532 module
 *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define ADAFRUIT_PN532_ENABLED

/** @brief Pin number
 *
 *  Minimum value: 0
 *  Maximum value: 31
 *
 * @note This is an NRF_CONFIG macro.
 */
#define PN532_IRQ


/** @brief Pin number
 *
 *  Minimum value: 0
 *  Maximum value: 31
 *
 * @note This is an NRF_CONFIG macro.
 */
#define PN532_RESET


/** @brief Pin number
 *
 *  Minimum value: 0
 *  Maximum value: 31
 *
 * @note This is an NRF_CONFIG macro.
 */
#define PN532_CONFIG_SCL


/** @brief Pin number
 *
 *  Minimum value: 0
 *  Maximum value: 31
 *
 * @note This is an NRF_CONFIG macro.
 */
#define PN532_CONFIG_SDA


/** @brief TWI instance to be used
 *
 *  Following options are available:
 * - 0
 * - 1
 * - 2
 *
 * @note This is an NRF_CONFIG macro.
 */
#define PN532_CONFIG_TWI_INSTANCE


/** @brief Enables logging in the module.
 *
 *  Set to 1 to activate.
 *
 * @note This is an NRF_CONFIG macro.
 */
#define ADAFRUIT_PN532_LOG_ENABLED

/** @brief Default Severity level
 *
 *  Following options are available:
 * - 0 - Off
 * - 1 - Error
 * - 2 - Warning
 * - 3 - Info
 * - 4 - Debug
 *
 * @note This is an NRF_CONFIG macro.
 */
#define ADAFRUIT_PN532_LOG_LEVEL


/** @brief ANSI escape code prefix.
 *
 *  Following options are available:
 * - 0 - Default
 * - 1 - Black
 * - 2 - Red
 * - 3 - Green
 * - 4 - Yellow
 * - 5 - Blue
 * - 6 - Magenta
 * - 7 - Cyan
 * - 8 - White
 *
 * @note This is an NRF_CONFIG macro.
 */
#define ADAFRUIT_PN532_INFO_COLOR



/** @brief Size of the buffer used for sending commands and storing responses.
 *
 *
 * @note This is an NRF_CONFIG macro.
 */
#define PN532_PACKBUFF_SIZE



/** @} */
