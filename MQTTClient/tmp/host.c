#include "host.h"

host_handle_t host_create(host_config_t *host_config)
{
    if(!host_config)
    {
        return NULL;
    }
    host_config->plt_if.drivers_init();
    return NULL;
}

void drivers_init()
{
    i2c_config_t i2c_config = {
      .mode = I2C_MODE_MASTER,
      .sda_io_num = GPIO_NUM_21,
      .scl_io_num = GPIO_NUM_22,
      .scl_pullup_en = false,
      .sda_pullup_en = false,
      .master = {
          .clk_speed = 100e3,
      },
    };
    if(ESP_OK != i2c_param_config(I2C_NUM_0, &i2c_config))
    {
        // LOG_ERROR("Failed to configure i2c driver.");
        // return -1;
    }
    if(ESP_OK != i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0))
    {
        // LOG_ERROR("Failed to install i2c driver.");
        // return -1;
    }
}