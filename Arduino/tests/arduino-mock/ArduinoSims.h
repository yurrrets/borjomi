#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <ostream>
#include <variant>
#include <vector>

struct ArduinoSimsContext
{
    unsigned long micros = 0;
    std::array<int, 8> analogPins = {};
    std::array<int, 13> digitalPins = {};
    std::array<uint8_t, 13> digitalPinModes = {};
};

struct DigitalPinChangedEvent
{
    unsigned long timeMs;
    int pin;
    int value;
    int pinMode;
};
bool operator==(const DigitalPinChangedEvent &a, const DigitalPinChangedEvent &b);
std::ostream &operator<<(std::ostream &os, const DigitalPinChangedEvent &v);

struct AnalogPinChangedEvent
{
    unsigned long timeMs;
    int pin;
    int value;
    int pinMode;
};
bool operator==(const AnalogPinChangedEvent &a, const AnalogPinChangedEvent &b);
std::ostream &operator<<(std::ostream &os, const AnalogPinChangedEvent &v);

using ArduinoEvent = std::variant<DigitalPinChangedEvent, AnalogPinChangedEvent>;

void setArduinoEventHandler(std::function<void(const ArduinoEvent &)> cb);
void resetArduinoEventHandler();
void notifyArduinoEvent(const ArduinoEvent &event);

void setValueForAnalogRead(uint8_t pin, int value);

using setup_cb_t = std::function<void()>;
using loop_cb_t = std::function<void()>;

struct ArduinoRunContext
{
    ArduinoRunContext();
    void set_callbacks(setup_cb_t setup_cb, loop_cb_t loop_cb);
    void run_for(int ms);
    void stop();

  private:
    setup_cb_t setup_cb;
    loop_cb_t loop_cb;
    bool is_running = false;
};

ArduinoSimsContext &get_sims_context();
void reset_sims_context();
