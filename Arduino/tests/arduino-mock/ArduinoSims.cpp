#include "ArduinoSims.h"
#include "Arduino.h"

namespace
{
std::function<void(const ArduinoEvent &)> currentArduinoEventHandler;
ArduinoSimsContext sims_context;

std::string pinModeToString(int pinMode)
{
    switch (pinMode)
    {
    case INPUT:
        return "input";
    case OUTPUT:
        return "output";
    case INPUT_PULLUP:
        return "input_pullup";
    default:
        return "unknown(" + std::to_string(pinMode) + ")";
    }
}

} // namespace

bool operator==(const DigitalPinChangedEvent &a, const DigitalPinChangedEvent &b)
{
    return std::tie(a.timeMs, a.pin, a.value, a.pinMode) ==
           std::tie(b.timeMs, b.pin, b.value, b.pinMode);
}

std::ostream &operator<<(std::ostream &os, const DigitalPinChangedEvent &v)
{
    os << "DigitalPinChangedEvent(timeMs=" << v.timeMs << ", pin=" << v.pin << ", val=" << v.value
       << ", pinMode=" << pinModeToString(v.pinMode) << ")" << std::endl;
    return os;
}

bool operator==(const AnalogPinChangedEvent &a, const AnalogPinChangedEvent &b)
{
    return std::tie(a.timeMs, a.pin, a.value, a.pinMode) ==
           std::tie(b.timeMs, b.pin, b.value, b.pinMode);
}

std::ostream &operator<<(std::ostream &os, const AnalogPinChangedEvent &v)
{
    os << "AnalogPinChangedEvent(timeMs=" << v.timeMs << ", pin=" << v.pin << ", val=" << v.value
       << ", pinMode=" << pinModeToString(v.pinMode) << ")" << std::endl;
    return os;
}

void setArduinoEventHandler(std::function<void(const ArduinoEvent &)> cb)
{
    currentArduinoEventHandler = std::move(cb);
}

void resetArduinoEventHandler()
{
    currentArduinoEventHandler = std::function<void(const ArduinoEvent &)>();
}

void notifyArduinoEvent(const ArduinoEvent &event)
{
    if (currentArduinoEventHandler)
    {
        currentArduinoEventHandler(event);
    }
}

void setValueForAnalogRead(uint8_t pin, int value)
{
    get_sims_context().analogPins[pin] = value;
}

ArduinoSimsContext &get_sims_context()
{
    return sims_context;
}

void reset_sims_context()
{
    sims_context = ArduinoSimsContext();
}

ArduinoRunContext::ArduinoRunContext()
{
}

void ArduinoRunContext::set_callbacks(setup_cb_t setup_cb, loop_cb_t loop_cb)
{
    this->setup_cb = std::move(setup_cb);
    this->loop_cb = std::move(loop_cb);
}

void ArduinoRunContext::run_for(int ms)
{
    is_running = true;
    auto target_micros = get_sims_context().micros + ms * 1000;

    setup_cb();
    while (is_running && get_sims_context().micros < target_micros)
    {
        loop_cb();
        get_sims_context().micros += 500;
    }

    is_running = false;
}

void ArduinoRunContext::stop()
{
    is_running = false;
}
