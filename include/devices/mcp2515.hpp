#pragma once
#include <cstdint>
#include <Arduino.h>
#include <SPI.h>

namespace mcp {

    struct can_frame {
        uint16_t standard_id;
        uint8_t data_length = 8;
        uint8_t data[8];
    };

    enum transmit_buffer {
        buffer0 = 0x30,
        buffer1 = 0x40,
        buffer2 = 0x50,
    };

struct mcp2515 {
    unsigned int cs = 0;
    SPIClass& spi;

    struct status {
        bool recieved_msg_in_buffer0 = false;
        bool recieved_msg_in_buffer1 = false;
        bool transmission_requested_buffer0 = false;
        bool transmission_empty_buffer0 = false;
        bool transmission_requested_buffer1 = false;
        bool transmission_empty_buffer1 = false;
        bool transmission_requested_buffer2 = false;
        bool transmission_empty_buffer2 = false;
    };

    mcp2515(unsigned _cs, SPIClass& _spi) : cs(_cs), spi(_spi) {
        pinMode(cs, OUTPUT);
        digitalWrite(cs, HIGH);
    }

    void write_register(uint8_t register_address, uint8_t * buffer, size_t length) {
        spi.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
        digitalWrite(cs, LOW);
        spi.transfer(0b00000010); // Write command
        spi.transfer(register_address);
        spi.transfer(buffer, length);
        digitalWrite(cs, HIGH);
        spi.endTransaction();
    }

    void read_register(uint8_t register_address, uint8_t * buffer, size_t length) {
        spi.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
        digitalWrite(cs, LOW);
        spi.transfer(0b00000011); // read command
        spi.transfer(register_address);
        spi.transfer(buffer, length);
        digitalWrite(cs, HIGH);
        spi.endTransaction();
    }
    void request_to_send(transmit_buffer buffer_select) {
        uint8_t command = 0;
        switch(buffer_select) {
        case buffer0:
            command = 0b10000001;
            break;
        case buffer1:
            command = 0b10000010;
            break;
        case buffer2:
            command = 0b10000100;
            break;
        }
        spi.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
        digitalWrite(cs, LOW);
        spi.transfer(command); // read command
        digitalWrite(cs, HIGH);
        spi.endTransaction();
    }
    // status get_status() {

    // }

    void send_can_frame(can_frame& frame, transmit_buffer buffer_select = transmit_buffer::buffer0) {
        uint8_t register_address = buffer_select + 0x01;
        uint8_t buffer[13];

        buffer[0] = frame.standard_id >> 3;
        buffer[1] = (frame.standard_id << 5);
        buffer[2] = 0;
        buffer[3] = 0;
        buffer[4] = 0 | frame.data_length; 
        for(size_t i = 0; i < frame.data_length; i ++) {
            buffer[5+i] = frame.data[i];
        }

        write_register(register_address, buffer, 5 + frame.data_length);

        request_to_send(buffer_select);
    }

    void reset() {
        spi.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
        digitalWrite(cs, LOW);
        spi.transfer(0b11000000); // Write command
        digitalWrite(cs, HIGH);
        spi.endTransaction();
    }

    void enter_normal_mode() {
        uint8_t value = 0b00000000;
        write_register(0x0F, &value, 1);
    }

};

}