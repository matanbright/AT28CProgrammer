#pragma once

#include <Arduino.h>


namespace IntegerUtils {
    bool isInteger(const String& str);
    bool isMaxXBitsInteger(uint64_t integer, uint8_t x);
};
