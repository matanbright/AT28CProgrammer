#include "EEPROM.hpp"


EEPROM::EEPROM(uint8_t cePinNumber,
               uint8_t oePinNumber,
               uint8_t wePinNumber,
               const uint8_t (&dataPinNumbers)[8],
               uint8_t sr_serPinNumber,
               uint8_t sr_srclkPinNumber,
               uint8_t sr_rclkPinNumber,
               Mode mode) :
            cePin(cePinNumber, true, Pin::Mode::OUT),
            oePin(oePinNumber, true, Pin::Mode::OUT),
            wePin(wePinNumber, true, Pin::Mode::OUT),
            mode(mode) {
    for (uint8_t i = 0; i < sizeof(dataPins) / sizeof(dataPins[0]); i++)
        dataPins[i] = new Pin(dataPinNumbers[i]);
    addressShiftRegister = new ShiftRegister(sr_serPinNumber, sr_srclkPinNumber, sr_rclkPinNumber);
}

EEPROM::~EEPROM() {
    delete addressShiftRegister;
    addressShiftRegister = nullptr;
    for (uint8_t i = 0; i < sizeof(dataPins) / sizeof(dataPins[0]); i++) {
        delete dataPins[i];
        dataPins[i] = nullptr;
    }
}

EEPROM::Mode EEPROM::getMode() const {
    return mode;
}

void EEPROM::setMode(Mode mode) {
    setAddressPinsValue(0);
    this->mode = mode;
}

bool EEPROM::hasAddress(uint16_t address) const {
    switch (mode) {
        case Mode::AT28C64:
            if (address >= static_cast<uint16_t>(1) << 13)
                return false;
            break;
        case Mode::AT28C256:
            if (address >= static_cast<uint16_t>(1) << 15)
                return false;
            break;
    }
    return true;
}

void EEPROM::read(uint16_t address, uint8_t& data) const {
    setDataPinsModeAsOutput(false);
    enable(true);
    setAddressPinsValue(address);
    enableOutput(true);
    data = getDataPinsValue();
    enableOutput(false);
    enable(false);
    delay(10);
}

void EEPROM::write(uint16_t address, uint8_t data) const {
    setDataPinsModeAsOutput(true);
    enable(true);
    setAddressPinsValue(address);
    setDataPinsValue(data);
    enableWrite(true);
    delayMicroseconds(1);
    enableWrite(false);
    enable(false);
    delay(10);
}

void EEPROM::enable(bool enabled) const {
    cePin.setState(enabled);
}

void EEPROM::enableOutput(bool enabled) const {
    oePin.setState(enabled);
}

void EEPROM::enableWrite(bool enabled) const {
    wePin.setState(enabled);
}

void EEPROM::setAddressPinsValue(uint16_t value) const {
    addressShiftRegister->push16BitData(value);
}

void EEPROM::setDataPinsModeAsOutput(bool setAsOutput) const {
    for (uint8_t i = 0; i < sizeof(dataPins) / sizeof(dataPins[0]); i++)
        dataPins[i]->setMode(setAsOutput ? Pin::Mode::OUT : Pin::Mode::IN);
}

uint8_t EEPROM::getDataPinsValue() const {
    uint8_t value = 0;
    for (uint8_t i = 0; i < sizeof(dataPins) / sizeof(dataPins[0]); i++)
        value |= dataPins[i]->getState() << i;
    return value;
}

void EEPROM::setDataPinsValue(uint8_t value) const {
    for (uint8_t i = 0; i < sizeof(dataPins) / sizeof(dataPins[0]); i++)
        dataPins[i]->setState((value >> i) & 1);
}
