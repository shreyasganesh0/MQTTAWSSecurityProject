
#include "freertos/FreeRTOS.h"

#define portTICK_PERIOD_MS			( ( TickType_t ) 1000 / configTICK_RATE_HZ )
#define MS2TICK(ms)      ((ms)/portTICK_PERIOD_MS)
#define TICK2MS(tick)    ((tick)*portTICK_PERIOD_MS)
