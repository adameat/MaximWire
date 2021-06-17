#include <Arduino.h>
#include <nrf_gpio.h>

namespace MaximWire {

#include "NRF52840.h"

HAL_NRF52840::HAL_NRF52840(uint8_t pin)
    : HAL_NRF52840_Base(digitalPinToPinName(pin))
{
}

HAL_NRF52840_EXT_PULLUP::HAL_NRF52840_EXT_PULLUP(uint8_t pin)
    : HAL_Common(pin)
{
    nrf_gpio_cfg_input(_Pin, NRF_GPIO_PIN_NOPULL);
}

void HAL_NRF52840_EXT_PULLUP::BusDown() {
    nrf_gpio_cfg_output(_Pin);
    nrf_gpio_pin_clear(_Pin);
}

void HAL_NRF52840_EXT_PULLUP::BusUp() {
    nrf_gpio_cfg_input(_Pin, NRF_GPIO_PIN_NOPULL);
}

uint8_t HAL_NRF52840_EXT_PULLUP::BusRead() {
    return nrf_gpio_pin_read(_Pin);
}

bool HAL_NRF52840_EXT_PULLUP::ResetPulse() {
    unsigned long now = micros();
    unsigned long deadline = now + 200;
    while (BusRead() == 0 && (now = micros()) < deadline) {}
    if (now < deadline) {
        BusDown();
        NRFX_DELAY_US(480);
        __disable_irq();
        BusUp();
        now = micros();
        deadline = now + 480;
        while (BusRead() != 0 && (now = micros()) < deadline) {}
        while (BusRead() == 0 && (now = micros()) < deadline) {}
        __enable_irq();
        if (now < deadline) {
            NRFX_DELAY_US(deadline - now);
            return true;
        }
    }
    return false;
}

HAL_NRF52840_INT_PULLUP::HAL_NRF52840_INT_PULLUP(uint8_t pin)
    : HAL_Common(pin)
{
    nrf_gpio_cfg_input(_Pin, NRF_GPIO_PIN_PULLUP);
}

void HAL_NRF52840_INT_PULLUP::BusDown() {
    nrf_gpio_cfg_input(_Pin, NRF_GPIO_PIN_PULLDOWN);
}

void HAL_NRF52840_INT_PULLUP::BusUp() {
    nrf_gpio_cfg_input(_Pin, NRF_GPIO_PIN_PULLUP);
}

uint8_t HAL_NRF52840_INT_PULLUP::BusRead() {
    return nrf_gpio_pin_read(_Pin);
}

bool HAL_NRF52840_INT_PULLUP::ResetPulse() {
    BusDown();
    NRFX_DELAY_US(480);
    BusUp();
    __disable_irq();
    unsigned long now = micros();
    unsigned long deadline = now + 480;
    while (BusRead() != 0 && (now = micros()) < deadline) {}
    while (BusRead() == 0 && (now = micros()) < deadline) {}
    __enable_irq();
    if (now < deadline) {
        NRFX_DELAY_US(deadline - now);
        return true;
    }
    return false;
}

}
