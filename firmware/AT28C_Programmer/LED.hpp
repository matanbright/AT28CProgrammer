#pragma once

#include <Arduino.h>
#include "Pin.hpp"


class LED {

    private:
        Pin pin;

    public:
        LED(uint8_t pinNumber, bool turnOn = false);
        void turnOn() const;
        void turnOff() const;
        void toggle() const;
};
