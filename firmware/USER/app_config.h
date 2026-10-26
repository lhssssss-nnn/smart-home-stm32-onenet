#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/* Copy app_config.local.example.h to app_config.local.h before building. */
#include "app_config.local.h"

#define APP_STRINGIFY_IMPL(value) #value
#define APP_STRINGIFY(value) APP_STRINGIFY_IMPL(value)

#endif
