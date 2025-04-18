#ifndef __HOST_H__
#define __HOST_H__

#include <stdint.h>
#include "driver/i2c.h"

//wifi interfaces
typedef struct {
    const char* ssid;
    const char* password;
    void (*wifi_init)(void);
    void (*wifi_disconnect)(void);
}host_wifi_if_t;

//platform interfaces
typedef struct {
    void (*drivers_init)(void);
    void (*time_init)(void);
}host_plt_if_t;

//host interfaces
typedef struct {
    host_wifi_if_t wifi_if;
    host_plt_if_t plt_if;
}host_if_t;

//host
typedef struct _host{
    uint8_t mac[6];
    host_if_t host_if;
}host_t;

typedef struct {
    host_wifi_if_t wifi_if;
    host_plt_if_t plt_if;
}host_config_t;

typedef host_t* host_handle_t;

host_handle_t host_create(host_config_t *host_config);
void host_destory(host_handle_t host_handler);

void drivers_init();
void time_init();

#endif /* __HOST_H__ */
