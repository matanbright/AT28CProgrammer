#include "HardwareTimer.hpp"


HardwareTimer* HardwareTimer::instance = nullptr;

HardwareTimer* HardwareTimer::getInstance() {
    if (HardwareTimer::instance == nullptr)
        HardwareTimer::instance = new HardwareTimer();
    return HardwareTimer::instance;
}

void HardwareTimer::tick() {
    if (HardwareTimer::instance != nullptr)
        if (HardwareTimer::instance->onTickEvent != nullptr)
            HardwareTimer::instance->onTickEvent();
}

HardwareTimer::HardwareTimer() :
        onTickEvent(nullptr) {}

HardwareTimer::~HardwareTimer() {
    stop();
    HardwareTimer::instance = nullptr;
}

void HardwareTimer::start(void (*onTickEvent)(), uint32_t intervalInMilliseconds) {
    stop();
    this->onTickEvent = onTickEvent;
    noInterrupts();
    uint16_t timerPrescaler = 0;
    uint8_t timerPrescalerRegisterBits;
    uint32_t compareMatchValue;
    if (intervalInMilliseconds == 0) {
        timerPrescaler = 0;
        timerPrescalerRegisterBits = (0 << CS12) | (0 << CS11) | (0 << CS10);
        compareMatchValue = 0;
    } else {
        do {
            if (timerPrescaler == 0) {
                timerPrescaler = 1;
                timerPrescalerRegisterBits = (0 << CS12) | (0 << CS11) | (1 << CS10);
            } else if (timerPrescaler == 1) {
                timerPrescaler = 8;
                timerPrescalerRegisterBits = (0 << CS12) | (1 << CS11) | (0 << CS10);
            } else if (timerPrescaler == 8) {
                timerPrescaler = 64;
                timerPrescalerRegisterBits = (0 << CS12) | (1 << CS11) | (1 << CS10);
            } else if (timerPrescaler == 64) {
                timerPrescaler = 256;
                timerPrescalerRegisterBits = (1 << CS12) | (0 << CS11) | (0 << CS10);
            } else if (timerPrescaler == 256) {
                timerPrescaler = 1024;
                timerPrescalerRegisterBits = (1 << CS12) | (0 << CS11) | (1 << CS10);
            } else {
                timerPrescaler = 0;
                timerPrescalerRegisterBits = (0 << CS12) | (0 << CS11) | (0 << CS10);
            }
            compareMatchValue = (F_CPU / (timerPrescaler * (1.0 / (intervalInMilliseconds / 1000.0)))) - 1.0;
        } while (compareMatchValue >= (static_cast<uint32_t>(1) << 16));
    }
    TCCR1B |= (1 << WGM12);
    TCCR1B |= timerPrescalerRegisterBits;
    OCR1A = compareMatchValue;
    TIMSK1 |= (1 << OCIE1A);
    interrupts();
}

void HardwareTimer::stop() {
    noInterrupts();
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    OCR1A = 0;
    TIMSK1 &= ~(1 << OCIE1A);
    interrupts();
    onTickEvent = nullptr;
}


ISR(TIMER1_COMPA_vect) {
    HardwareTimer::tick();
}
