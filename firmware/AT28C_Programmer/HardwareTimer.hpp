#pragma once

#include <Arduino.h>


class HardwareTimer {

    private:
        static HardwareTimer* instance;
    
    public:
        static HardwareTimer* getInstance();
        static void tick();
    
    private:
        void (*onTickEvent)();

    private:
        HardwareTimer();
        HardwareTimer(const HardwareTimer& other) = delete;
        HardwareTimer& operator=(const HardwareTimer& other) = delete;
    public:
        ~HardwareTimer();
        void start(void (*onTickEvent)(), uint32_t intervalInMilliseconds);
        void stop();
};
