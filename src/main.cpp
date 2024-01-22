#include <secrets.h>



#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <Wire.h>

#include <handlers/can.hpp>
#include <handlers/i2c.hpp>


// Server Static IP address. THIS SHOULD BE A FREE ADDRESS ON THE NETWORK!
IPAddress local_IP(192, 168, 1, 184);


// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional

AsyncWebServer server(80);

const char* HOSTNAME = "esp_rest_server";

const char* PARAM_MESSAGE = "message";

void not_found(AsyncWebServerRequest *request) {
    Serial.print(request->url());
    Serial.println(" : 404 not found");
    request->send(404, "text/plain", "Not a valid endpoint");
}

void setup() {
    Serial.begin(115200);
    Wire.begin();
    // if(!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    //     Serial.println("Failed to configure WiFi controller\n");
    // }

    if(!WiFi.hostname(HOSTNAME)) {
        Serial.print("Failed to set hostname to \"");Serial.print(HOSTNAME);Serial.println("\"\n");
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASSWORD);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.printf("WiFi Failed to connect! Perhaps invalid credentials?\nCheck include/secrets.h");
        return;
    }

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    if(WiFi.localIP() != local_IP) {
        Serial.println("WARNING: IP RECIEVED IS NOT THE SAME AS THE REQUESTED IP.");
        Serial.print("Recieved: ");
        Serial.println(WiFi.localIP());
        Serial.print("Requested: ");
        Serial.println(local_IP);
    }

    server.on("*", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
        Serial.println("Serviced Preflight Request");
        AsyncResponseStream *response = request->beginResponseStream("text/html");
        response->addHeader("Access-Control-Allow-Origin", "*");
        response->addHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
        response->addHeader("Access-Control-Allow-Headers", "Content-Type");
        response->setCode(200);
        request->send(response);
    });

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Hello, world");
    });



    // Send a GET request to <IP>/get?message=<message>
    server.on("/id", HTTP_GET, [] (AsyncWebServerRequest *request) {
        request->send(200, "text/plain", HOSTNAME);
    });

    can_handlers::register_handlers(server);
    i2c_handlers::register_handlers(server);

    server.onNotFound(not_found);

    server.begin();
}

void loop() {
}