#pragma once
#include <ESPAsyncWebServer.h>

namespace i2c_handlers {
    void register_handlers(AsyncWebServer& server);
};