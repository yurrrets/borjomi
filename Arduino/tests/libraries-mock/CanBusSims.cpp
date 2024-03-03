#include "CanBusSims.h"
#include "can_message.h"
#include "nodes.h"
#include <cstring>
#include <experimental/source_location>

namespace
{

std::string get_not_implemented_msg(const std::string &funcname,
                                    const std::experimental::source_location location =
                                        std::experimental::source_location::current())
{
    return std::string("function NOT implemented at ") + location.file_name() + ":" +
           std::to_string(location.line()) + ". " + funcname;
}

#define raise_not_implemented_error()                                                              \
    throw std::runtime_error(get_not_implemented_msg(__PRETTY_FUNCTION__));

} // namespace

bool operator==(const PingEvent &a, const PingEvent &b)
{
    return std::tie(a.timeMs, a.nodeId) == std::tie(b.timeMs, b.nodeId);
}

std::ostream &operator<<(std::ostream &os, const PingEvent &v)
{
    os << "PingEvent(timeMs=" << v.timeMs << ", nodeId=" << v.nodeId << ")" << std::endl;
    return os;
}

bool operator==(const SetWaterSwitchEvent &a, const SetWaterSwitchEvent &b)
{
    return std::tie(a.timeMs, a.nodeId, a.devNo, a.value) ==
           std::tie(b.timeMs, b.nodeId, b.devNo, b.value);
}

std::ostream &operator<<(std::ostream &os, const SetWaterSwitchEvent &v)
{
    os << "SetWaterSwitchEvent(timeMs=" << v.timeMs << ", nodeId=" << v.nodeId
       << ", devNo=" << v.devNo << ", val=" << v.value << ")" << std::endl;
    return os;
}

CanDevice::CanDevice(CanBusSims &bus) : bus(bus)
{
}

void CanDevice::sendData(int32_t nodeId, int8_t ext, std::vector<int8_t> data)
{
    bus.receiveData(nodeId, ext, data);
}

void CanDevice::notifyEvent(const NodeEvent &event)
{
    bus.notifyEvent(event);
}

CanBusSims::CanBusSims(int gpioIntPin) : gpioIntPin(gpioIntPin)
{
}

void CanBusSims::loop()
{
    foreachDevice([](std::shared_ptr<CanDevice> device) { device->loop(); });
}

void CanBusSims::onEvent(std::function<void(const NodeEvent &)> cb)
{
    eventCb = std::move(cb);
}

void CanBusSims::attachDevice(std::weak_ptr<CanDevice> device)
{
    devices.push_back(device);
}

void CanBusSims::receiveData(int32_t nodeId, int8_t ext, std::vector<int8_t> data)
{
    pendingInputData.emplace_back(CanDeviceData{nodeId, ext, data});
    digitalWrite(gpioIntPin, LOW);
}

void CanBusSims::notifyEvent(const NodeEvent &event)
{
    if (eventCb)
    {
        eventCb(event);
    }
}

INT8U CanBusSims::begin(INT8U idmodeset, INT8U speedset, INT8U clockset)
{
    pinMode(gpioIntPin, OUTPUT);
    digitalWrite(gpioIntPin, HIGH);
    foreachDevice([](std::shared_ptr<CanDevice> device) { device->setup(); });
    return CAN_OK;
}

INT8U CanBusSims::init_Mask(INT8U num, INT8U ext, INT32U ulData)
{
    raise_not_implemented_error();
}

INT8U CanBusSims::init_Mask(INT8U num, INT32U ulData)
{
    raise_not_implemented_error();
}

INT8U CanBusSims::init_Filt(INT8U num, INT8U ext, INT32U ulData)
{
    raise_not_implemented_error();
}

INT8U CanBusSims::init_Filt(INT8U num, INT32U ulData)
{
    raise_not_implemented_error();
}

INT8U CanBusSims::setMode(INT8U opMode)
{
    return MCP2515_OK;
}

INT8U CanBusSims::sendMsgBuf(INT32U id, INT8U ext, INT8U len, INT8U *buf)
{
    std::vector<int8_t> data(buf, buf + len);
    foreachDevice(
        [&](std::shared_ptr<CanDevice> device) { device->onDataReceived(id, ext, data); });
    return len;
}

INT8U CanBusSims::sendMsgBuf(INT32U id, INT8U len, INT8U *buf)
{
    return sendMsgBuf(id, len, buf);
}

INT8U CanBusSims::readMsgBuf(INT32U *id, INT8U *ext, INT8U *len, INT8U *buf)
{
    if (pendingInputData.empty())
    {
        assert(false);
    }

    CanDeviceData &record = pendingInputData.front();
    *id = record.nodeId;
    *ext = record.ext;
    *len = record.data.size();
    std::memcpy(buf, record.data.data(), record.data.size());
    pendingInputData.pop_front();

    if (pendingInputData.empty())
    {
        digitalWrite(gpioIntPin, HIGH);
    }

    return *len;
}

INT8U CanBusSims::readMsgBuf(INT32U *id, INT8U *len, INT8U *buf)
{
    INT8U ext;
    return readMsgBuf(id, &ext, len, buf);
}

INT8U CanBusSims::checkReceive(void)
{
    raise_not_implemented_error();
}

INT8U CanBusSims::checkError(void)
{
    raise_not_implemented_error();
}

INT8U CanBusSims::getError(void)
{
    raise_not_implemented_error();
}

INT8U CanBusSims::errorCountRX(void)
{
    raise_not_implemented_error();
}

INT8U CanBusSims::errorCountTX(void)
{
    raise_not_implemented_error();
}

INT8U CanBusSims::enOneShotTX(void)
{
    raise_not_implemented_error();
}

INT8U CanBusSims::disOneShotTX(void)
{
    raise_not_implemented_error();
}

INT8U CanBusSims::abortTX(void)
{
    raise_not_implemented_error();
}

INT8U CanBusSims::setGPO(INT8U data)
{
    raise_not_implemented_error();
}

INT8U CanBusSims::getGPI(void)
{
    raise_not_implemented_error();
}

void CanBusSims::foreachDevice(std::function<void(std::shared_ptr<CanDevice>)> cb)
{
    for (auto weak_device : devices)
    {
        auto device = weak_device.lock();
        if (device)
        {
            cb(device);
        }
    }
}

SlaveNodeSims::SlaveNodeSims(CanBusSims &bus, int nodeId, Behavior behavior)
    : CanDevice(bus), thisNodeId(nodeId), behavior(behavior)
{
}

void SlaveNodeSims::setup()
{
}

void SlaveNodeSims::loop()
{
    while (true)
    {
        auto it = std::find_if(std::begin(pendingPackets), std::end(pendingPackets),
                               [](const auto &item) { return millis() >= item.timeMs; });
        if (it == std::end(pendingPackets))
        {
            break;
        }

        switch (behavior)
        {
        case Behavior::Success: {
            auto &item = *it;
            std::vector<int8_t> data(sizeof(item.data));
            std::memcpy(data.data(), &item.data, sizeof(item.data));
            sendData(MASTER_NODE, 0, data);
            break;
        }
        case Behavior::NoAnswer: {
            // do nothing
            break;
        }
        default: {
            assert(false);
        }
        }
        pendingPackets.erase(it);
    }
}

void SlaveNodeSims::onDataReceived(int32_t nodeId, int8_t ext, std::vector<int8_t> data)
{
    CanMessage inp;
    std::memcpy(&inp, data.data(), data.size());

    CanMessage outp = inp;
    outp.code = CMD_OK;
    outp.value = CVAL_ACCEPTED;

    if (nodeId == MULTICAST_NODE && inp.code == CMD_PING)
    {
        outp.code = CMD_PONG;
        pendingPackets.push_back(Task{.timeMs = millis() + 2, .data = outp});
        notifyEvent(PingEvent{.timeMs = millis(), .nodeId = nodeId});
        return;
    }

    if (nodeId != thisNodeId)
    {
        return;
    }

    int exectimeMs = 5;

    switch (inp.code)
    {
    case CMD_PING:
        outp.code = CMD_PONG;
        exectimeMs = 2;
        notifyEvent(PingEvent{.timeMs = millis(), .nodeId = nodeId});
        break;
    case CMD_SET_WATER_SWITCH:
        // accept command
        notifyEvent(SetWaterSwitchEvent{
            .timeMs = millis(), .nodeId = nodeId, .devNo = inp.devno, .value = inp.value});
        break;
    default:
        outp = CanMessage(); // reset to invalid
        break;
    }
    outp.updateCrc();

    pendingPackets.push_back(Task{.timeMs = millis() + exectimeMs, .data = outp});
}
