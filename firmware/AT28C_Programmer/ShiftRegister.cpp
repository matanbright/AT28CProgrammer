#include "ShiftRegister.hpp"


ShiftRegister::ShiftRegister(uint8_t serPinNumber,
                             uint8_t srclkPinNumber,
                             uint8_t rclkPinNumber) :
        serPin(serPinNumber, false, Pin::Mode::OUT),
        srclkPin(srclkPinNumber, false, Pin::Mode::OUT),
        rclkPin(rclkPinNumber, false, Pin::Mode::OUT) {}

void ShiftRegister::push16BitData(uint16_t data) const {
    srclkPin.setState(false);
    rclkPin.setState(false);
    for (int8_t i = 15; i >= 0; i--) {
        serPin.setState((data >> i) & 1);
        srclkPin.setState(true);
        delayMicroseconds(1);
        srclkPin.setState(false);
    }
    rclkPin.setState(true);
    delayMicroseconds(1);
    rclkPin.setState(false);
}
