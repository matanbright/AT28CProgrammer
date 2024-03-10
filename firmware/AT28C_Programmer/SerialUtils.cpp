#include "SerialUtils.hpp"


String SerialUtils::readLine() {
    String line;
    while (true) {
        while (!Serial.available())
            delay(1);
        char c = Serial.read();
        if (c == '\n')
            return line;
        if (c != '\r')
            line += c;
    }
}
