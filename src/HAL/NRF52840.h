#pragma once

#if defined(ARDUINO_ARCH_NRF52840)

// some basic types
using uint8_t = unsigned char;
using int16_t = signed short;

static_assert(sizeof(int16_t) == 2);

#include "HAL/Common.h"

class HAL_NRF52840_EXT_PULLUP : public HAL_Common {
public:
    HAL_NRF52840_EXT_PULLUP(uint8_t pin);
    void BusDown();
    void BusUp();
    uint8_t BusRead();
    bool ResetPulse();
};

class HAL_NRF52840_INT_PULLUP : public HAL_Common {
public:
    HAL_NRF52840_INT_PULLUP(uint8_t pin);
    void BusDown();
    void BusUp();
    uint8_t BusRead();
    bool ResetPulse();
};

#if defined(MAXIMWIRE_EXTERNAL_PULLUP)
using HAL_NRF52840_Base = HAL_NRF52840_EXT_PULLUP;
#else
using HAL_NRF52840_Base = HAL_NRF52840_INT_PULLUP;
#endif

class HAL_NRF52840 : public HAL_NRF52840_Base {
public:
    HAL_NRF52840(uint8_t pin);

    uint8_t ReadSlot() {
        __disable_irq();
        BusDown();
        NRFX_DELAY_US(2);
        BusUp();
        NRFX_DELAY_US(5);
        uint8_t r = BusRead();
        __enable_irq();
        NRFX_DELAY_US(50);
        return r;
    }
    
    void WriteSlot0() {
        __disable_irq();
        BusDown();
        NRFX_DELAY_US(60);
        BusUp();
        __enable_irq();
        NRFX_DELAY_US(2);
    }
    
    void WriteSlot1() {
        __disable_irq();
        BusDown();
        NRFX_DELAY_US(2);
        BusUp();
        __enable_irq();
        NRFX_DELAY_US(60);
    }
};

using HAL = HAL_NRF52840;

#endif
