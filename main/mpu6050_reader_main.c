#include <inttypes.h>
#include <stdio.h>

#include "driver/i2c_master.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/clk_tree_defs.h"

#define I2C_PORT_NUM                     I2C_NUM_0
#define I2C_SDA_GPIO                     GPIO_NUM_8
#define I2C_SCL_GPIO                     GPIO_NUM_9
#define I2C_SCL_SPEED_HZ                 400000
#define I2C_XFER_TIMEOUT_MS              100

#define MPU6050_I2C_ADDRESS              0x68
#define MPU6050_EXPECTED_WHO_AM_I        0x68
#define MPU6050_REG_ACCEL_XOUT_H         0x3B
#define MPU6050_REG_GYRO_XOUT_H          0x43
#define MPU6050_REG_PWR_MGMT_1           0x6B
#define MPU6050_REG_WHO_AM_I             0x75
#define MPU6050_PWR_MGMT_1_WAKEUP        0x00

#define SENSOR_READ_PERIOD_MS            1000
#define MPU6050_ACCEL_LSB_PER_G          16384.0f
#define MPU6050_GYRO_LSB_PER_DPS         131.0f

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} axis3_t;

static const char *TAG = "mpu6050_reader";
static i2c_master_bus_handle_t s_i2c_bus;
static i2c_master_dev_handle_t s_mpu6050;

static esp_err_t mpu6050_write_register(uint8_t reg_addr, uint8_t value)
{
    uint8_t write_buffer[] = { reg_addr, value };
    return i2c_master_transmit(s_mpu6050, write_buffer, sizeof(write_buffer), I2C_XFER_TIMEOUT_MS);
}

static esp_err_t mpu6050_read_registers(uint8_t reg_addr, uint8_t *data, size_t data_len)
{
    return i2c_master_transmit_receive(s_mpu6050, &reg_addr, sizeof(reg_addr), data, data_len, I2C_XFER_TIMEOUT_MS);
}

static esp_err_t mpu6050_read_axis3(uint8_t start_reg, axis3_t *axis)
{
    uint8_t raw_data[6] = { 0 };
    ESP_RETURN_ON_ERROR(mpu6050_read_registers(start_reg, raw_data, sizeof(raw_data)), TAG, "Falha ao ler registrador 0x%02X", start_reg);

    axis->x = (int16_t)((raw_data[0] << 8) | raw_data[1]);
    axis->y = (int16_t)((raw_data[2] << 8) | raw_data[3]);
    axis->z = (int16_t)((raw_data[4] << 8) | raw_data[5]);

    return ESP_OK;
}

static esp_err_t init_i2c_bus(void)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_PORT_NUM,
        .sda_io_num = I2C_SDA_GPIO,
        .scl_io_num = I2C_SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&bus_config, &s_i2c_bus), TAG, "Falha ao iniciar barramento I2C");
    ESP_RETURN_ON_ERROR(i2c_master_probe(s_i2c_bus, MPU6050_I2C_ADDRESS, I2C_XFER_TIMEOUT_MS), TAG, "MPU6050 nao encontrado no endereco 0x%02X", MPU6050_I2C_ADDRESS);

    i2c_device_config_t device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = MPU6050_I2C_ADDRESS,
        .scl_speed_hz = I2C_SCL_SPEED_HZ,
    };

    return i2c_master_bus_add_device(s_i2c_bus, &device_config, &s_mpu6050);
}

static esp_err_t init_mpu6050(void)
{
    uint8_t who_am_i = 0;

    ESP_RETURN_ON_ERROR(mpu6050_read_registers(MPU6050_REG_WHO_AM_I, &who_am_i, sizeof(who_am_i)), TAG, "Falha ao ler WHO_AM_I");
    ESP_RETURN_ON_FALSE(who_am_i == MPU6050_EXPECTED_WHO_AM_I, ESP_ERR_INVALID_RESPONSE, TAG, "WHO_AM_I inesperado: 0x%02X", who_am_i);

    ESP_RETURN_ON_ERROR(mpu6050_write_register(MPU6050_REG_PWR_MGMT_1, MPU6050_PWR_MGMT_1_WAKEUP), TAG, "Falha ao acordar o MPU6050");
    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_LOGI(TAG, "MPU6050 inicializado com sucesso. WHO_AM_I=0x%02X", who_am_i);
    return ESP_OK;
}

static void log_sensor_data(const axis3_t *accel, const axis3_t *gyro)
{
    const float accel_x_g = accel->x / MPU6050_ACCEL_LSB_PER_G;
    const float accel_y_g = accel->y / MPU6050_ACCEL_LSB_PER_G;
    const float accel_z_g = accel->z / MPU6050_ACCEL_LSB_PER_G;

    const float gyro_x_dps = gyro->x / MPU6050_GYRO_LSB_PER_DPS;
    const float gyro_y_dps = gyro->y / MPU6050_GYRO_LSB_PER_DPS;
    const float gyro_z_dps = gyro->z / MPU6050_GYRO_LSB_PER_DPS;

    ESP_LOGI(
        TAG,
        "Accel[g] X=%.2f Y=%.2f Z=%.2f | Gyro[dps] X=%.2f Y=%.2f Z=%.2f",
        (double)accel_x_g,
        (double)accel_y_g,
        (double)accel_z_g,
        (double)gyro_x_dps,
        (double)gyro_y_dps,
        (double)gyro_z_dps
    );
}

void app_main(void)
{
    printf("Iniciando leitura do MPU6050...\n");

    axis3_t accel = { 0 };
    axis3_t gyro = { 0 };

    ESP_ERROR_CHECK(init_i2c_bus());
    ESP_ERROR_CHECK(init_mpu6050());

    while (true) {
        if (mpu6050_read_axis3(MPU6050_REG_ACCEL_XOUT_H, &accel) == ESP_OK &&
            mpu6050_read_axis3(MPU6050_REG_GYRO_XOUT_H, &gyro) == ESP_OK) {
            log_sensor_data(&accel, &gyro);
        } else {
            ESP_LOGE(TAG, "Falha ao ler os dados do MPU6050");
        }

        vTaskDelay(pdMS_TO_TICKS(SENSOR_READ_PERIOD_MS));
    }
}