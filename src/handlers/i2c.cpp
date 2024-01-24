#include <handlers/i2c.hpp>
#include <Wire.h>
#include "AsyncJson.h"
#include <ArduinoJson.h>

#define bus0 Wire

namespace i2c_handlers {




void probe(AsyncWebServerRequest *request) {
    std::uint8_t addresses[128];
    std::size_t i = 0;

    for(std::uint8_t address = 1; address < 127; address ++) {
        bus0.beginTransmission(address);
        std::uint8_t error = bus0.endTransmission();

        if (error == 0) {
            addresses[i] = address;
            i++;
        }
    }

    Serial.println("Recived request to probe i2c");
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->setContentType("application/json");
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->print("{\"success\":true,\"devices\":[");
    for(int j = 0; j < i; j ++) {
        response->printf("%d", addresses[j]);
        if(j+1 != i) {
            response->print(",");
        }
    }
    response->print("]}");
    response->setCode(200);
    request->send(response);
}

const char * i2c_error_msgs[] = {
    "SUCCESS"
    "DATA_TOO_LONG",
    "NACK_ON_ADDR",
    "NACK_ON_DATA",
    "OTHER",
    "TIMEOUT"
};

void register_handlers(AsyncWebServer& server) {
    
    AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/i2c/send", [](AsyncWebServerRequest *request, JsonVariant &json) {
        Serial.println("Recived request to send i2c");
        int device_address = json["device_address"];
        int response_size = json["response_size"];
        

        if(response_size > 127) {
            response_size = 127;
        }

        std::uint8_t buffer[127];
        std::uint8_t recieved = 0;

        Serial.print("dev_addr: ");
        Serial.println(device_address);

        Serial.print("res_size: ");
        Serial.println(response_size);
        
        Serial.println("Writing: ");
        Wire.beginTransmission(device_address);
        for (JsonVariant value : json["data"].as<JsonArray>()) {
            Serial.print(value.as<int>());
            Serial.print(", ");
            Wire.write(value.as<int>());
        }
        Serial.println();
        
        const char * i2c_error_msg = NULL;

        if(response_size != 0) {
            int error = Wire.endTransmission(false);
            if(error == 0) {
                recieved = Wire.requestFrom(device_address, response_size);
                Serial.println("Recieved: ");
                for(std::uint8_t i = 0; i < recieved; i ++) {
                    buffer[i] = Wire.read();
                    Serial.print(buffer[i]);
                    Serial.print(", ");
                }
                Serial.println();

            }else {
                // Set error message.
                Wire.requestFrom(device_address, 0);
                Serial.println(i2c_error_msgs[error]);
                i2c_error_msg = i2c_error_msgs[error];
            }
        }else {
            Wire.endTransmission();
        }

        AsyncResponseStream *response = request->beginResponseStream("text/html");
        response->setContentType("application/json");
        response->addHeader("Access-Control-Allow-Origin", "*");
        response->setCode(200);
        response->print("{\"success\":true,\"recieved_data\":[");
        for(std::size_t i = 0; i < recieved; i ++) {
            response->printf("%d", buffer[i]);
            if(i+1 != recieved) {
                response->print(",");
            }
        }
        response->print("]");
        if(i2c_error_msg != nullptr) {
            response->print(",\"i2c_error_message\":\"");
            response->print(i2c_error_msg);
            response->print("\"");
        }
        response->print("}");
        request->send(response);
    }, 256);

    server.addHandler(handler);

    server.on("/i2c/probe", HTTP_GET, probe);
}
}