#include "CommandLine.hpp"


CommandLine::CommandLine(const String& command, const String& arg1, const String& arg2) :
        command(command), arg1(arg1), arg2(arg2) {}

CommandLine::CommandLine(const String& commandLineString) {
    String lineParts[3];
    uint8_t currentLinePartIndex = 0;
    for (char c : commandLineString) {
        if (c == ' ') {
            currentLinePartIndex++;
            if (currentLinePartIndex >= sizeof(lineParts) / sizeof(lineParts[0]))
                break;
            continue;
        }
        lineParts[currentLinePartIndex] += c;
    }
    command = lineParts[0];
    arg1 = lineParts[1];
    arg2 = lineParts[2];
}

const String& CommandLine::getCommand() const {
    return command;
}

const String& CommandLine::getArg1() const {
    return arg1;
}

const String& CommandLine::getArg2() const {
    return arg2;
}
