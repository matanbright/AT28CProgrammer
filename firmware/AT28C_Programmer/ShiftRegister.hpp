#pragma once

#include <Arduino.h>
#include "Pin.hpp"


class ShiftRegister {

    private:
        Pin serPin;
        Pin srclkPin;
        Pin rclkPin;

    public:
        ShiftRegister(uint8_t serPinNumber, uint8_t srclkPinNumber, uint8_t rclkPinNumber);
        void push16BitData(uint16_t data) const;
};
