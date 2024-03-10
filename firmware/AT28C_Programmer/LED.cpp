#include "LED.hpp"


LED::LED(uint8_t pinNumber, bool turnOn) :
        pin(pinNumber, false, Pin::Mode::OUT, turnOn) {}

void LED::turnOn() const {
    pin.setState(true);
}

void LED::turnOff() const {
    pin.setState(false);
}

void LED::toggle() const {
    pin.setState(!pin.getState());
}
