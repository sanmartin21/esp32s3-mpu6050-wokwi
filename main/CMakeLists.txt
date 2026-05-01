idf_component_register(
    SRCS "mpu6050_reader_main.c"
    INCLUDE_DIRS "."
    REQUIRES esp_driver_i2c
)