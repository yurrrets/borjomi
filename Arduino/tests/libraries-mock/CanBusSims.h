#include "ArduinoSims.h"
#include "can_message.h"
#include "mcp_can.h"
#include <deque>
#include <memory>
#include <variant>
#include <vector>

class CanBusSims;

struct CanDeviceData
{
    int32_t nodeId;
    int8_t ext;
    std::vector<int8_t> data;
};

struct PingEvent
{
    unsigned long timeMs;
    int nodeId;
};
bool operator==(const PingEvent &a, const PingEvent &b);
std::ostream &operator<<(std::ostream &os, const PingEvent &v);

struct SetWaterSwitchEvent
{
    unsigned long timeMs;
    int nodeId;
    int devNo;
    unsigned int value;
};
bool operator==(const SetWaterSwitchEvent &a, const SetWaterSwitchEvent &b);
std::ostream &operator<<(std::ostream &os, const SetWaterSwitchEvent &v);

using NodeEvent = std::variant<PingEvent, SetWaterSwitchEvent>;

class CanDevice : std::enable_shared_from_this<CanDevice>
{
    friend class CanBusSims;

  protected:
    CanDevice(CanBusSims &bus);
    virtual void setup() = 0;
    virtual void loop() = 0;
    virtual void onDataReceived(int32_t nodeId, int8_t ext, std::vector<int8_t> data) = 0;
    void sendData(int32_t nodeId, int8_t ext, std::vector<int8_t> data);
    void notifyEvent(const NodeEvent &event);

  protected:
    CanBusSims &bus;
};

class CanBusSims : public MCP_CAN
{
    friend class CanDevice;

  public:
    CanBusSims(int gpioIntPin);
    void attachDevice(std::weak_ptr<CanDevice> device);
    void loop();
    void onEvent(std::function<void(const NodeEvent &)> cb);

  protected:
    void receiveData(int32_t nodeId, int8_t ext, std::vector<int8_t> data);
    void notifyEvent(const NodeEvent &event);

  public:
    INT8U begin(INT8U idmodeset, INT8U speedset, INT8U clockset) override;
    INT8U init_Mask(INT8U num, INT8U ext, INT32U ulData) override;
    INT8U init_Mask(INT8U num, INT32U ulData) override;
    INT8U init_Filt(INT8U num, INT8U ext, INT32U ulData) override;
    INT8U init_Filt(INT8U num, INT32U ulData) override;
    INT8U setMode(INT8U opMode) override;
    INT8U sendMsgBuf(INT32U id, INT8U ext, INT8U len, INT8U *buf) override;
    INT8U sendMsgBuf(INT32U id, INT8U len, INT8U *buf) override;
    INT8U readMsgBuf(INT32U *id, INT8U *ext, INT8U *len, INT8U *buf) override;
    INT8U readMsgBuf(INT32U *id, INT8U *len, INT8U *buf) override;
    INT8U checkReceive(void) override;
    INT8U checkError(void) override;
    INT8U getError(void) override;
    INT8U errorCountRX(void) override;
    INT8U errorCountTX(void) override;
    INT8U enOneShotTX(void) override;
    INT8U disOneShotTX(void) override;
    INT8U abortTX(void) override;
    INT8U setGPO(INT8U data) override;
    INT8U getGPI(void) override;

  private:
    void foreachDevice(std::function<void(std::shared_ptr<CanDevice>)> cb);

  private:
    int gpioIntPin;
    std::vector<std::weak_ptr<CanDevice>> devices;
    std::deque<CanDeviceData> pendingInputData;
    std::function<void(const NodeEvent &)> eventCb;
};

class SlaveNodeSims : public CanDevice
{
  public:
    enum class Behavior
    {
        Success,
        NoAnswer
    };
    SlaveNodeSims(CanBusSims &bus, int nodeId, Behavior behavior = Behavior::Success);

  protected:
    void setup() override;
    void loop() override;
    void onDataReceived(int32_t nodeId, int8_t ext, std::vector<int8_t> data) override;

  private:
    int thisNodeId;

    struct Task
    {
        unsigned long timeMs;
        CanMessage data;
    };
    std::vector<Task> pendingPackets;
    Behavior behavior;
};
