#pragma once

#include <Arduino.h>
#include "Pin.hpp"
#include "ShiftRegister.hpp"


class EEPROM {

    public:
        enum class Mode {
            AT28C64,
            AT28C256
        };

    private:
        Pin cePin;
        Pin oePin;
        Pin wePin;
        Pin* dataPins[8];
        ShiftRegister* addressShiftRegister;
        Mode mode;

    public:
        EEPROM(uint8_t cePinNumber,
               uint8_t oePinNumber,
               uint8_t wePinNumber,
               const uint8_t (&dataPinNumbers)[8],
               uint8_t sr_serPinNumber,
               uint8_t sr_srclkPinNumber,
               uint8_t sr_rclkPinNumber,
               Mode mode = Mode::AT28C64);
        ~EEPROM();
        Mode getMode() const;
        void setMode(Mode mode);
        bool hasAddress(uint16_t address) const;
        void read(uint16_t address, uint8_t& data) const;
        void write(uint16_t address, uint8_t data) const;
    private:
        void enable(bool enabled) const;
        void enableOutput(bool enabled) const;
        void enableWrite(bool enabled) const;
        void setAddressPinsValue(uint16_t value) const;
        void setDataPinsModeAsOutput(bool setAsOutput) const;
        uint8_t getDataPinsValue() const;
        void setDataPinsValue(uint8_t value) const;
};
