
#include "tps65185.h"
#include "pca9555.h"
#include "epd_board.h"
#include "esp_err.h"
#include "esp_log.h"

#include <driver/i2c_master.h>
#include <stdint.h>
#include <string.h>

static const int EPDIY_TPS_ADDR = 0x68;

static uint8_t i2c_master_read_slave(i2c_master_bus_handle_t bus_handle, int reg) {
    uint8_t r_data[1];

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = EPDIY_TPS_ADDR,
        .scl_speed_hz = 100000,
    };
    
    i2c_master_dev_handle_t dev_handle;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));
    
    esp_err_t ret = i2c_master_transmit_receive(dev_handle, (uint8_t*)&reg, 1, r_data, 1, 1000);
    ESP_ERROR_CHECK(ret);
    
    ESP_ERROR_CHECK(i2c_master_bus_rm_device(dev_handle));
    return r_data[0];
}

static esp_err_t i2c_master_write_slave(
    i2c_master_bus_handle_t bus_handle, uint8_t ctrl, uint8_t* data_wr, size_t size
) {
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = EPDIY_TPS_ADDR,
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

esp_err_t tps_write_register(i2c_master_bus_handle_t bus_handle, int reg, uint8_t value) {
    uint8_t w_data[1];
    esp_err_t err;

    w_data[0] = value;

    err = i2c_master_write_slave(bus_handle, reg, w_data, 1);
    return err;
}

uint8_t tps_read_register(i2c_master_bus_handle_t bus_handle, int reg) {
    return i2c_master_read_slave(bus_handle, reg);
}

void tps_set_vcom(i2c_master_bus_handle_t bus_handle, unsigned vcom_mV) {
    unsigned val = vcom_mV / 10;
    ESP_ERROR_CHECK(tps_write_register(bus_handle, 4, (val & 0x100) >> 8));
    ESP_ERROR_CHECK(tps_write_register(bus_handle, 3, val & 0xFF));
}

int8_t tps_read_thermistor(i2c_master_bus_handle_t bus_handle) {
    tps_write_register(bus_handle, TPS_REG_TMST1, 0x80);
    int tries = 0;
    while (true) {
        uint8_t val = tps_read_register(bus_handle, TPS_REG_TMST1);
        // temperature conversion done
        if (val & 0x20) {
            break;
        }
        tries++;

        if (tries >= 100) {
            ESP_LOGE("epdiy", "thermistor read timeout!");
            break;
        }
    }
    return (int8_t)tps_read_register(bus_handle, TPS_REG_TMST_VALUE);
}

void tps_vcom_kickback() {
    printf("VCOM Kickback test\n");
    // pull the WAKEUP pin and the PWRUP pin high to enable all output rails.
    epd_current_board()->measure_vcom(epd_ctrl_state());
    // set the HiZ bit in the VCOM2 register (BIT 5) 0x20
    // this puts the VCOM pin in a high-impedance state.
    // bit 3 & 4 Number of acquisitions that is averaged to a single kick-back V. measurement
    tps_write_register(I2C_NUM_0, 4, 0x38);
    vTaskDelay(1);

    uint8_t int1reg = tps_read_register(I2C_NUM_0, TPS_REG_INT1);
    uint8_t vcomreg = tps_read_register(I2C_NUM_0, TPS_REG_VCOM2);
}

void tps_vcom_kickback_start() {
    uint8_t int1reg = tps_read_register(I2C_NUM_0, TPS_REG_INT1);
    // set the ACQ bit in the VCOM2 register to 1 (BIT 7)
    tps_write_register(I2C_NUM_0, TPS_REG_VCOM2, 0xA0);
}

unsigned tps_vcom_kickback_rdy() {
    uint8_t int1reg = tps_read_register(I2C_NUM_0, TPS_REG_INT1);

    if (int1reg == 0x02) {
        uint8_t lsb = tps_read_register(I2C_NUM_0, 3);
        uint8_t msb = tps_read_register(I2C_NUM_0, 4);
        int u16Value = (lsb | (msb << 8)) & 0x1ff;
        ESP_LOGI("vcom", "raw value:%d temperature:%d C", u16Value, tps_read_thermistor(I2C_NUM_0));
        return u16Value * 10;
    } else {
        return 0;
    }
}