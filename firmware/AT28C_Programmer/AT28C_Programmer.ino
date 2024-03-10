#include "EEPROM.hpp"
#include "LED.hpp"
#include "HardwareTimer.hpp"
#include "CommandLine.hpp"
#include "SerialUtils.hpp"
#include "IntegerUtils.hpp"


constexpr const char* FIRMWARE_INFO = "AT28C Programmer | v1.0";
constexpr uint32_t SERIAL_BAUD_RATE = 115200;
constexpr uint32_t LED_BLINKING_INTERVAL_IN_MILLISECONDS = 125;

namespace GpioPinNumber {
    namespace EEPROM {
        constexpr uint8_t CE = 11;
        constexpr uint8_t OE = 12;
        constexpr uint8_t WE = 13;
        constexpr uint8_t DATA[] = { A0, A1, A2, A3, A4, A5, 2, 3 };
    }
    namespace ShiftRegister {
        constexpr uint8_t SER = 10;
        constexpr uint8_t SRCLK = 8;
        constexpr uint8_t RCLK = 9;
    }
    constexpr uint8_t LED = 4;
}

namespace EepromMode {
    constexpr const char* AT28C64 = "AT28C64";
    constexpr const char* AT28C256 = "AT28C256";
}

namespace Command {
    constexpr const char* MODE = "m";
    constexpr const char* READ = "r";
    constexpr const char* WRITE = "w";
}

namespace StatusMessage {
    constexpr const char* OK = "OK";
    constexpr const char* ERROR_INVALID_COMMAND = "Error: Invalid Command!";
    constexpr const char* ERROR_MISSING_PARAMETERS = "Error: Missing Parameter(s)!";
    constexpr const char* ERROR_INVALID_PARAMETERS = "Error: Invalid Parameter(s)!";
}


EEPROM eeprom(
    GpioPinNumber::EEPROM::CE,
    GpioPinNumber::EEPROM::OE,
    GpioPinNumber::EEPROM::WE,
    GpioPinNumber::EEPROM::DATA,
    GpioPinNumber::ShiftRegister::SER,
    GpioPinNumber::ShiftRegister::SRCLK,
    GpioPinNumber::ShiftRegister::RCLK
);
LED led(GpioPinNumber::LED, true);
HardwareTimer* ledBlinkingTimer = HardwareTimer::getInstance();
bool blinkLed = false;


void printHelp() {
    Serial.println(FIRMWARE_INFO);
    Serial.println("-----------------------");
    Serial.println("Commands:");
    Serial.println("    " + String(Command::MODE) + "                     - Get mode of operation.");
    Serial.println("    " + String(Command::MODE) + " <mode>              - Set mode of operation. ('mode' can be \"" + String(EepromMode::AT28C64) + "\" or \"" + String(EepromMode::AT28C256) + "\")");
    Serial.println("                            [Default is: \"" + String(EepromMode::AT28C64) + "\"]");
    Serial.println("    " + String(Command::READ) + " <address>           - Read data from address 'address'.");
    Serial.println("    " + String(Command::WRITE) + " <address> <data>    - Write data 'data' at address 'address'.");
    Serial.println("");
}

void onLedBlinkingTimerTickEvent() {
    if (blinkLed) {
        led.toggle();
        blinkLed = false;
    } else
        led.turnOn();
}


void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    ledBlinkingTimer->start(onLedBlinkingTimerTickEvent, LED_BLINKING_INTERVAL_IN_MILLISECONDS);
    printHelp();
}

void loop() {
    CommandLine commandLine(SerialUtils::readLine());
    if (commandLine.getCommand().equals(Command::MODE)) {
        if (commandLine.getArg1().equals("")) {
            switch (eeprom.getMode()) {
                case (EEPROM::Mode::AT28C64):
                    Serial.println(EepromMode::AT28C64);
                    break;
                case (EEPROM::Mode::AT28C256):
                    Serial.println(EepromMode::AT28C256);
                    break;
            }
        } else if (!commandLine.getArg1().equals(EepromMode::AT28C64) && !commandLine.getArg1().equals(EepromMode::AT28C256))
            Serial.println(StatusMessage::ERROR_INVALID_PARAMETERS);
        else {
            if (commandLine.getArg1().equals(EepromMode::AT28C64))
                eeprom.setMode(EEPROM::Mode::AT28C64);
            else
                eeprom.setMode(EEPROM::Mode::AT28C256);
            Serial.println(StatusMessage::OK);
        }
    } else if (commandLine.getCommand().equals(Command::READ)) {
        if (commandLine.getArg1().equals(""))
            Serial.println(StatusMessage::ERROR_MISSING_PARAMETERS);
        else if (!IntegerUtils::isInteger(commandLine.getArg1()))
            Serial.println(StatusMessage::ERROR_INVALID_PARAMETERS);
        else {
            uint64_t arg1Integer = commandLine.getArg1().toInt();
            if (!IntegerUtils::isMaxXBitsInteger(arg1Integer, 16))
                Serial.println(StatusMessage::ERROR_INVALID_PARAMETERS);
            else {
                uint16_t address = arg1Integer;
                if (!eeprom.hasAddress(address))
                    Serial.println(StatusMessage::ERROR_INVALID_PARAMETERS);
                else {
                    blinkLed = true;
                    uint8_t data;
                    eeprom.read(address, data);
                    Serial.println(data);
                }
            }
        }
    } else if (commandLine.getCommand().equals(Command::WRITE)) {
        if (commandLine.getArg1().equals("") || commandLine.getArg2().equals(""))
            Serial.println(StatusMessage::ERROR_MISSING_PARAMETERS);
        else if (!IntegerUtils::isInteger(commandLine.getArg1()) || !IntegerUtils::isInteger(commandLine.getArg2()))
            Serial.println(StatusMessage::ERROR_INVALID_PARAMETERS);
        else {
            uint64_t arg1Integer = commandLine.getArg1().toInt();
            uint64_t arg2Integer = commandLine.getArg2().toInt();
            if (!IntegerUtils::isMaxXBitsInteger(arg1Integer, 16) || !IntegerUtils::isMaxXBitsInteger(arg2Integer, 8))
                Serial.println(StatusMessage::ERROR_INVALID_PARAMETERS);
            else {
                uint16_t address = arg1Integer;
                if (!eeprom.hasAddress(address))
                    Serial.println(StatusMessage::ERROR_INVALID_PARAMETERS);
                else {
                    blinkLed = true;
                    uint8_t data = arg2Integer;
                    eeprom.write(address, data);
                    Serial.println(StatusMessage::OK);
                }
            }
        }
    } else
        Serial.println(StatusMessage::ERROR_INVALID_COMMAND);
}
