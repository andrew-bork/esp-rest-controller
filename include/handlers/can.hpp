#pragma once
#include <ESPAsyncWebServer.h>

namespace can_handlers {
    void register_handlers(AsyncWebServer& server);
};