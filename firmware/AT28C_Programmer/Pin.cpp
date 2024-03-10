#include "Pin.hpp"


Pin::Pin(uint8_t number, bool inverted, Mode mode, bool enable) :
        number(number), inverted(inverted) {
    setMode(mode);
    setState(enable);
}

Pin::~Pin() {
    setMode(DEFAULT_MODE);
}

void Pin::setMode(Mode mode) const {
    uint8_t modeValue = INPUT;
    switch (mode) {
        case Mode::IN:
            modeValue = INPUT;
            break;
        case Mode::IN_PULLUP:
            modeValue = INPUT_PULLUP;
            break;
        case Mode::OUT:
            modeValue = OUTPUT;
            break;
    }
    pinMode(number, modeValue);
}

bool Pin::getState() const {
    return (digitalRead(number) == HIGH) ^ inverted;
}

void Pin::setState(bool enabled) const {
    digitalWrite(number, (enabled ^ inverted) ? HIGH : LOW);
}
