#pragma once

class Util {
public:    
    static uint8_t CRC8(const uint8_t* address, uint8_t length);
    static char HexC(uint8_t d);
    static String Hex(uint8_t b);
};

class HAL_Common : public Util {
public:
    HAL_Common(uint8_t pin);

protected:
    uint8_t _Pin;
};
