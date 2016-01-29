#include "ble_logger_service.h"

/*
 init_spi_slaves(p_context);

 // SPIのnCS
 //    out_config = GPIOTE_CONFIG_OUT_SIMPLE(false); // 引数は init_high。初期値をlowにする。
 out_config.init_state = NRF_GPIOTE_INITIAL_VALUE_LOW;
 out_config.task_pin = false;
 err_code = nrf_drv_gpiote_out_init(PIN_NUMBER_SPI_nCS, &out_config);
 APP_ERROR_CHECK(err_code);

 
 static void init_spi_slaves(senstick_sensor_manager_t *p_context)
 {
 ret_code_t err_code;
 // SPIインタフェース SPI0を使用。
 
 /*
 #define NRF_DRV_SPI_INSTANCE(id)                        \
 {                                                       \
 .p_registers  = NRF_DRV_SPI_PERIPHERAL(id),         \     #define NRF_DRV_SPI_PERIPHERAL(id)  (void *)CONCAT_2(NRF_SPI, id)
 .irq          = CONCAT_3(SPI, id, _IRQ),            \
 .drv_inst_idx = CONCAT_3(SPI, id, _INSTANCE_INDEX), \
 .use_easy_dma = CONCAT_3(SPI, id, _USE_EASY_DMA)    \
 }     *
p_context->spi.p_registers  = NRF_SPI0;
p_context->spi.irq          = SPI0_IRQ;
p_context->spi.drv_inst_idx = SPI0_INSTANCE_INDEX;
p_context->spi.use_easy_dma = SPI0_USE_EASY_DMA;

//    nrf_drv_spi_config_t config = NRF_DRV_SPI_DEFAULT_CONFIG(0);
/*
 #define NRF_DRV_SPI_DEFAULT_CONFIG(id)                       \
 {                                                            \
 .sck_pin      = CONCAT_3(SPI, id, _CONFIG_SCK_PIN),      \
 .mosi_pin     = CONCAT_3(SPI, id, _CONFIG_MOSI_PIN),     \
 .miso_pin     = CONCAT_3(SPI, id, _CONFIG_MISO_PIN),     \
 .ss_pin       = NRF_DRV_SPI_PIN_NOT_USED,                \
 .irq_priority = CONCAT_3(SPI, id, _CONFIG_IRQ_PRIORITY), \
 .orc          = 0xFF,                                    \
 .frequency    = NRF_DRV_SPI_FREQ_4M,                     \
 .mode         = NRF_DRV_SPI_MODE_0,                      \
 .bit_order    = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST,         \
 }
 *
nrf_drv_spi_config_t config;
config.sck_pin      = SPI0_CONFIG_SCK_PIN;
config.mosi_pin     = SPI0_CONFIG_MOSI_PIN;
config.miso_pin     = SPI0_CONFIG_MISO_PIN;
config.ss_pin       = NRF_DRV_SPI_PIN_NOT_USED;
config.irq_priority = SPI0_CONFIG_IRQ_PRIORITY;
config.orc          = 0xff;
config.frequency    = NRF_DRV_SPI_FREQ_4M;
//    config.frequency    = NRF_DRV_SPI_FREQ_250K;
config.mode         = NRF_DRV_SPI_MODE_0;
config.bit_order    = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;

err_code = nrf_drv_spi_init(&(p_context->spi), &config, NULL);
APP_ERROR_CHECK(err_code);

initFlashMemory(&(p_context->flash_memory_context), &(p_context->spi));
}

 */