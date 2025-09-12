#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "epd_board.h"
#include "esp_log.h"

static const adc_channel_t channel = ADC_CHANNEL_7;
static adc_oneshot_unit_handle_t adc1_handle;
static adc_cali_handle_t adc1_cali_handle;

#define NUMBER_OF_SAMPLES 100

void epd_board_temperature_init_v2() {
    // ADC1 Init
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    // ADC1 Config
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_6,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, channel, &config));

    // ADC1 Calibration Init
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_6,
        .bitwidth = ADC_BITWIDTH_12,
    };
    esp_err_t ret = adc_cali_create_scheme_curve_fitting(&cali_config, &adc1_cali_handle);
    if (ret == ESP_OK) {
        ESP_LOGI("epd_temperature", "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !adc1_cali_handle) {
        ESP_LOGW("epd_temperature", "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE("epd_temperature", "Invalid arg or no memory");
    }
}

float epd_board_ambient_temperature_v2() {
    uint32_t value = 0;
    int adc_raw;
    
    for (int i = 0; i < NUMBER_OF_SAMPLES; i++) {
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, channel, &adc_raw));
        value += adc_raw;
    }
    value /= NUMBER_OF_SAMPLES;
    
    // Convert adc_raw to voltage in mV
    int voltage;
    if (adc1_cali_handle) {
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, value, &voltage));
    } else {
        // Fallback: simple linear conversion if calibration not available
        voltage = value * 3300 / 4095; // Assuming 3.3V reference and 12-bit ADC
    }
    
    return (voltage - 500.0) / 10.0;
}
