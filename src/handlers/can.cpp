#include <handlers/can.hpp>
#include "AsyncJson.h"
#include <ArduinoJson.h>
#include <SPI.h>
#include <devices/mcp2515.hpp>


namespace can_handlers {


    const uint HSPI_SCK = 14;
    const uint HSPI_MISO = 12;
    const uint HSPI_MOSI = 13;
    const uint CAN_CS = 27;
    const uint CAN_INT = 26;

void register_handlers(AsyncWebServer& server) {

    static SPIClass hspi(HSPI);
    hspi.begin(HSPI_SCK, HSPI_MISO, HSPI_MOSI, CAN_CS);
    pinMode(CAN_CS, OUTPUT);
    digitalWrite(CAN_CS, HIGH);

    Serial.println("WAKE UP CAN");
    static mcp::mcp2515 can_controller(CAN_CS, hspi);
    can_controller.reset();
    uint8_t result = 0b10101010;
    Serial.println(result, BIN);
    can_controller.read_register(0x0F, &result, 1);
    Serial.println(result, BIN);
    can_controller.read_register(0x0e, &result, 1);
    Serial.println(result, BIN);
    can_controller.enter_normal_mode();
    Serial.println("did it work?");
    can_controller.read_register(0x0e, &result, 1);
    Serial.println(result, BIN);

    AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/can/send", [&](AsyncWebServerRequest *request, JsonVariant &json) {
        Serial.println("Recived request to send can");
        mcp::can_frame frame;
        frame.data_length = 8;
        frame.standard_id = json["frame_id"];;
        
        std::uint8_t buffer[127];
        std::uint8_t recieved = 0;

        Serial.print("frame id: ");
        Serial.println(frame.standard_id);
        
        std::size_t i = 0;

        Serial.println("data: ");
        for (JsonVariant value : json["data"].as<JsonArray>()) {
            frame.data[i] = value.as<int>();
            Serial.print(frame.data[i]);
            Serial.print(", ");
            i++;
        }
        Serial.println();

        can_controller.send_can_frame(frame);  
        uint8_t result = 0b10101010;
        can_controller.read_register(0x1c, &result, 1);
        Serial.print("CAN Error Count: ");
        Serial.println(result, BIN);
        
        // const char * can_error_msg = NULL;


        AsyncResponseStream *response = request->beginResponseStream("text/html");
        response->setContentType("application/json");
        response->addHeader("Access-Control-Allow-Origin", "*");
        response->setCode(200);
        response->print("{\"success\":true");
        response->print("}");
        request->send(response);
    }, 256);

    server.addHandler(handler);
}
}