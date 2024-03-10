#pragma once

#include <Arduino.h>


class CommandLine {

    private:
        String command;
        String arg1;
        String arg2;

    public:
        CommandLine(const String& command, const String& arg1, const String& arg2);
        CommandLine(const String& commandLineString);
        const String& getCommand() const;
        const String& getArg1() const;
        const String& getArg2() const;
};
