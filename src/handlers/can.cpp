#include <handlers/can.hpp>



namespace can_handlers {
void send(AsyncWebServerRequest *request) {

}

void probe(AsyncWebServerRequest *request) {

}


void register_handlers(AsyncWebServer& server) {
    server.on("/can/send", HTTP_POST, send);
    server.on("/can/probe", HTTP_GET, probe);
}
}