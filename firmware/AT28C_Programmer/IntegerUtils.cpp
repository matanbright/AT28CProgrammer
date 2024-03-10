#include "IntegerUtils.hpp"


bool IntegerUtils::isInteger(const String& str) {
    for (char c : str)
        if (c < '0' || c > '9')
            return false;
    return true;
}

bool IntegerUtils::isMaxXBitsInteger(uint64_t integer, uint8_t x) {
    if (x == 0)
        return false;
    if (x >= 64)
        return true;
    return (integer < (static_cast<uint64_t>(1) << x));
}
