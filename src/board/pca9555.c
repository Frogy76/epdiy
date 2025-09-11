

#include "pca9555.h"
#include <driver/i2c_master.h>
#include <esp_err.h>
#include <esp_log.h>
#include <stdint.h>
#include <string.h>

#define REG_INPUT_PORT0 0
#define REG_INPUT_PORT1 1

#define REG_OUTPUT_PORT0 2
#define REG_OUTPUT_PORT1 3

#define REG_INVERT_PORT0 4
#define REG_INVERT_PORT1 5

#define REG_CONFIG_PORT0 6
#define REG_CONFIG_PORT1 7

static esp_err_t i2c_master_read_slave(i2c_master_bus_handle_t bus_handle, uint8_t* data_rd, size_t size, int reg) {
    if (size == 0) {
        return ESP_OK;
    }
    
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = EPDIY_PCA9555_ADDR,
        .scl_speed_hz = 100000,
    };
    
    i2c_master_dev_handle_t dev_handle;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));
    
    esp_err_t ret = i2c_master_transmit_receive(dev_handle, (uint8_t*)&reg, 1, data_rd, size, 1000);
    
    ESP_ERROR_CHECK(i2c_master_bus_rm_device(dev_handle));
    return ret;
}

static esp_err_t i2c_master_write_slave(
    i2c_master_bus_handle_t bus_handle, uint8_t ctrl, uint8_t* data_wr, size_t size
) {
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = EPDIY_PCA9555_ADDR,
        .scl_speed_hz = 100000,
    };
    
    i2c_master_dev_handle_t dev_handle;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));
    
    uint8_t write_buffer[size + 1];
    write_buffer[0] = ctrl;
    memcpy(&write_buffer[1], data_wr, size);
    
    esp_err_t ret = i2c_master_transmit(dev_handle, write_buffer, size + 1, 1000);
    
    ESP_ERROR_CHECK(i2c_master_bus_rm_device(dev_handle));
    return ret;
}

static esp_err_t pca9555_write_single(i2c_master_bus_handle_t bus_handle, int reg, uint8_t value) {
    uint8_t w_data[1] = { value };
    return i2c_master_write_slave(bus_handle, reg, w_data, sizeof(w_data));
}

esp_err_t pca9555_set_config(i2c_master_bus_handle_t bus_handle, uint8_t config_value, int high_port) {
    return pca9555_write_single(bus_handle, REG_CONFIG_PORT0 + high_port, config_value);
}

esp_err_t pca9555_set_inversion(i2c_master_bus_handle_t bus_handle, uint8_t config_value, int high_port) {
    return pca9555_write_single(bus_handle, REG_INVERT_PORT0 + high_port, config_value);
}

esp_err_t pca9555_set_value(i2c_master_bus_handle_t bus_handle, uint8_t config_value, int high_port) {
    return pca9555_write_single(bus_handle, REG_OUTPUT_PORT0 + high_port, config_value);
}

uint8_t pca9555_read_input(i2c_master_bus_handle_t bus_handle, int high_port) {
    esp_err_t err;
    uint8_t r_data[1];

    err = i2c_master_read_slave(bus_handle, r_data, 1, REG_INPUT_PORT0 + high_port);
    if (err != ESP_OK) {
        ESP_LOGE("PCA9555", "%s failed", __func__);
        return 0;
    }

    return r_data[0];
}
