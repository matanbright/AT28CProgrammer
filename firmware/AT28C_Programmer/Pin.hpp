#pragma once

#include <Arduino.h>


class Pin {

    public:
        enum class Mode {
            IN,
            IN_PULLUP,
            OUT
        };

    public:
        static constexpr Mode DEFAULT_MODE = Mode::IN;

    private:
        uint8_t number;
        bool inverted;

    public:
        Pin(uint8_t number, bool inverted = false, Mode mode = DEFAULT_MODE, bool enable = false);
        ~Pin();
        void setMode(Mode mode) const;
        bool getState() const;
        void setState(bool enabled) const;
};
