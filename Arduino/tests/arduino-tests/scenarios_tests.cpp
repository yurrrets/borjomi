#include "common.h"
#include "scenarios.h"

#include "Arduino.h"
#include "ArduinoSims.h"
#include "CanBusSims.h"
#include "log.h"

#include "GmockMatchers.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "TemplateTools.h"
#include <variant>

using namespace testing;

MasterNodeID &getNodeConfig()
{
    static MasterNodeID nodeId;
    return nodeId;
}

struct CurrectStepChangedEvent
{
    unsigned long timeMs;
    int step;
};
bool operator==(const CurrectStepChangedEvent &a, const CurrectStepChangedEvent &b);
std::ostream &operator<<(std::ostream &os, const CurrectStepChangedEvent &v);

struct ErrorFlagChanged
{
    unsigned long timeMs;
    bool value;
};
bool operator==(const ErrorFlagChanged &a, const ErrorFlagChanged &b);
std::ostream &operator<<(std::ostream &os, const ErrorFlagChanged &v);

struct RunnerStateChanged
{
    unsigned long timeMs;
    ScenarioRunnerState state;
};
bool operator==(const RunnerStateChanged &a, const RunnerStateChanged &b);
std::ostream &operator<<(std::ostream &os, const ScenarioRunnerState &v);
std::ostream &operator<<(std::ostream &os, const RunnerStateChanged &v);

bool operator==(const ScenarioStepNode &a, const ScenarioStepNode &b);
bool operator==(const ScenarioStep &a, const ScenarioStep &b);
bool operator==(const Scenario &a, const Scenario &b);

using OtherEvent = std::variant<CurrectStepChangedEvent, ErrorFlagChanged, RunnerStateChanged>;
using Event = template_concat_t<ArduinoEvent, NodeEvent, OtherEvent>;

class ScenariosTest : public testing::Test
{
  protected:
    void SetUp() override
    {
        reset_sims_context();
        runnerCurrentStep = runner.currentStep();

        auto eventHandler = [&](const auto &event) {
            std::visit([&](const auto &e) { events.push_back(e); }, event);
        };
        setArduinoEventHandler(eventHandler);
        can_bus.onEvent(eventHandler);

        ctx.set_callbacks(
            [&] {
                pinMode(PINS_DC_ADAPTER_SWITCH[0], OUTPUT);
                digitalWrite(PINS_DC_ADAPTER_SWITCH[0], LOW);
                pinMode(PINS_PUMP_SWITCH[0], OUTPUT);
                digitalWrite(PINS_PUMP_SWITCH[0], LOW);

                // can_bus.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ);
                canCommands.setup();
                checkCurrentStateChanged();

                if (additional_setup_step)
                {
                    additional_setup_step();
                }
            },
            [&] {
                can_bus.loop();
                canCommands.loop();
                runner.loop();
                checkCurrentStateChanged();

                if (additional_loop_step)
                {
                    additional_loop_step();
                }
            });
    }

    void TearDown() override
    {
        resetArduinoEventHandler();
    }

    void checkCurrentStateChanged()
    {
        if (runner.currentStep() != runnerCurrentStep)
        {
            runnerCurrentStep = runner.currentStep();
            events.push_back(
                CurrectStepChangedEvent{.timeMs = millis(), .step = runnerCurrentStep});
        }
        if (runner.hasError() != runnerHasError)
        {
            runnerHasError = runner.hasError();
            events.push_back(ErrorFlagChanged{.timeMs = millis(), .value = runnerHasError});
        }
        if (runner.currentState() != runnerState)
        {
            runnerState = runner.currentState();
            events.push_back(RunnerStateChanged{.timeMs = millis(), .state = runnerState});
        }
    }

    static constexpr auto can_int_pin = 1;
    CanBusSims can_bus{can_int_pin};
    CanCommands canCommands{can_bus, can_int_pin};
    ScenarioRunner runner{canCommands};
    ArduinoRunContext ctx;

    int8_t runnerCurrentStep = 0;
    bool runnerHasError = false;
    ScenarioRunnerState runnerState = ScenarioRunnerState::Ready;
    std::vector<Event> events;

    std::function<void()> additional_setup_step;
    std::function<void()> additional_loop_step;
};

bool operator==(const CurrectStepChangedEvent &a, const CurrectStepChangedEvent &b)
{
    return std::tie(a.timeMs, a.step) == std::tie(b.timeMs, b.step);
}

std::ostream &operator<<(std::ostream &os, const CurrectStepChangedEvent &v)
{
    os << "CurrectStepChangedEvent(timeMs=" << v.timeMs << ", step=" << v.step << ")" << std::endl;
    return os;
}

bool operator==(const ErrorFlagChanged &a, const ErrorFlagChanged &b)
{
    return std::tie(a.timeMs, a.value) == std::tie(b.timeMs, b.value);
}

std::ostream &operator<<(std::ostream &os, const ErrorFlagChanged &v)
{
    os << "ErrorFlagChanged(timeMs=" << v.timeMs << ", value=" << v.value << ")" << std::endl;
    return os;
}

bool operator==(const RunnerStateChanged &a, const RunnerStateChanged &b)
{
    return std::tie(a.timeMs, a.state) == std::tie(b.timeMs, b.state);
}

std::ostream &operator<<(std::ostream &os, const ScenarioRunnerState &v)
{
    switch (v)
    {
    case ScenarioRunnerState::Ready:
        os << "Ready";
        break;
    case ScenarioRunnerState::Running:
        os << "Running";
        break;
    case ScenarioRunnerState::Finished:
        os << "Finished";
        break;
    default:
        os << "<Unknown state>";
        break;
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const RunnerStateChanged &v)
{
    os << "RunnerStateChanged(timeMs=" << v.timeMs << ", state=" << v.state << ")" << std::endl;
    return os;
}

bool operator==(const ScenarioStepNode &a, const ScenarioStepNode &b)
{
    return a.address == b.address && (a.address == NO_NODE || a.devNo == b.devNo);
}

bool operator==(const ScenarioStep &a, const ScenarioStep &b)
{
    if (a.runTimeSec != b.runTimeSec)
    {
        return false;
    }
    for (int i = 0; i < ScenarioStep::MaxNodeCount; i++)
    {
        if (!(a.nodes[i] == b.nodes[i]))
        {
            return false;
        }
        if (a.nodes[i].address == NO_NODE)
        {
            break;
        }
    }
    return true;
}

bool operator==(const Scenario &a, const Scenario &b)
{
    if (a.stepCount != b.stepCount)
    {
        return false;
    }
    for (int i = 0; i < a.stepCount; i++)
    {
        if (!(a.steps[i] == b.steps[i]))
        {
            return false;
        }
    }
    return true;
}

TEST_F(ScenariosTest, no_scenario)
{
    EXPECT_EQ(runner.totalSteps(), 0);
    EXPECT_EQ(runner.currentState(), ScenarioRunnerState::Ready);
    ctx.run_for(1 * 1000);
    EXPECT_EQ(runner.currentState(), ScenarioRunnerState::Ready);
}

TEST_F(ScenariosTest, empty_scenario)
{
    Scenario scenario{.stepCount = 0, .steps = {}};

    runner.start(&scenario);
    EXPECT_EQ(runner.totalSteps(), 2);
    EXPECT_EQ(runner.currentState(), ScenarioRunnerState::Running);

    ctx.run_for(13 * 1000);
    EXPECT_EQ(runner.currentState(), ScenarioRunnerState::Finished);
    EXPECT_FALSE(runner.hasError());

    std::vector<Event> expected_events{
        DigitalPinChangedEvent{
            .timeMs = 0, .pin = PINS_PUMP_SWITCH[0], .value = true, .pinMode = OUTPUT},
        DigitalPinChangedEvent{
            .timeMs = 0, .pin = PINS_DC_ADAPTER_SWITCH[0], .value = true, .pinMode = OUTPUT},

        DigitalPinChangedEvent{
            .timeMs = 10001, .pin = PINS_PUMP_SWITCH[0], .value = false, .pinMode = OUTPUT},
        DigitalPinChangedEvent{
            .timeMs = 10001, .pin = PINS_DC_ADAPTER_SWITCH[0], .value = false, .pinMode = OUTPUT},
    };
    EXPECT_THAT(events, IsSupersequenceOf(expected_events));
}

TEST_F(ScenariosTest, singlestep_singlenode)
{
    Scenario scenario{
        .stepCount = 1,
        .steps = {ScenarioStep{.nodes = {ScenarioStepNode{20, 1}, ScenarioStepNode{NO_NODE, 0}},
                               .runTimeSec = 15}}};
    auto slave_node = std::make_shared<SlaveNodeSims>(can_bus, 20);
    can_bus.attachDevice(slave_node);

    runner.start(&scenario);
    EXPECT_EQ(runner.totalSteps(), 3);
    EXPECT_EQ(runner.currentState(), ScenarioRunnerState::Running);

    ctx.run_for(28 * 1000);
    EXPECT_EQ(runner.currentState(), ScenarioRunnerState::Finished);
    EXPECT_FALSE(runner.hasError());

    // time may flow a little due to simulated delays
    // and relatively high time increment between two loop() calls
    std::vector<Event> expected_events{
        DigitalPinChangedEvent{
            .timeMs = 0, .pin = PINS_PUMP_SWITCH[0], .value = true, .pinMode = OUTPUT},
        DigitalPinChangedEvent{
            .timeMs = 0, .pin = PINS_DC_ADAPTER_SWITCH[0], .value = true, .pinMode = OUTPUT},

        SetWaterSwitchEvent{.timeMs = 10001, .nodeId = 20, .devNo = 1, .value = true},
        SetWaterSwitchEvent{.timeMs = 25000, .nodeId = 20, .devNo = 1, .value = false},

        DigitalPinChangedEvent{
            .timeMs = 25006, .pin = PINS_PUMP_SWITCH[0], .value = false, .pinMode = OUTPUT},
        DigitalPinChangedEvent{
            .timeMs = 25006, .pin = PINS_DC_ADAPTER_SWITCH[0], .value = false, .pinMode = OUTPUT},
    };
    EXPECT_THAT(events, IsSupersequenceOf(expected_events));
}

TEST_F(ScenariosTest, singlestep_multinode)
{
    Scenario scenario{
        .stepCount = 1,
        .steps = {ScenarioStep{.nodes = {ScenarioStepNode{30, 0}, ScenarioStepNode{40, 1},
                                         ScenarioStepNode{60, 2}, ScenarioStepNode{50, 3}},
                               .runTimeSec = 5 * 60}}};
    std::vector<std::shared_ptr<SlaveNodeSims>> slave_nodes(
        {std::make_shared<SlaveNodeSims>(can_bus, 30), std::make_shared<SlaveNodeSims>(can_bus, 40),
         std::make_shared<SlaveNodeSims>(can_bus, 50),
         std::make_shared<SlaveNodeSims>(can_bus, 60)});
    for (auto node : slave_nodes)
    {
        can_bus.attachDevice(node);
    }

    runner.start(&scenario);
    EXPECT_EQ(runner.totalSteps(), 3);
    EXPECT_EQ(runner.currentState(), ScenarioRunnerState::Running);

    ctx.run_for((5 * 60 + 13) * 1000);
    EXPECT_EQ(runner.currentState(), ScenarioRunnerState::Finished);
    EXPECT_FALSE(runner.hasError());

    // time may flow a little due to simulated delays
    // and relatively high time increment between two loop() calls
    std::vector<Event> expected_events{
        DigitalPinChangedEvent{
            .timeMs = 0, .pin = PINS_PUMP_SWITCH[0], .value = true, .pinMode = OUTPUT},
        DigitalPinChangedEvent{
            .timeMs = 0, .pin = PINS_DC_ADAPTER_SWITCH[0], .value = true, .pinMode = OUTPUT},

        SetWaterSwitchEvent{.timeMs = 10001, .nodeId = 30, .devNo = 0, .value = true},
        SetWaterSwitchEvent{.timeMs = 10006, .nodeId = 40, .devNo = 1, .value = true},
        SetWaterSwitchEvent{.timeMs = 10011, .nodeId = 60, .devNo = 2, .value = true},
        SetWaterSwitchEvent{.timeMs = 10016, .nodeId = 50, .devNo = 3, .value = true},

        SetWaterSwitchEvent{
            .timeMs = 5 * 60 * 1000 + 10000, .nodeId = 30, .devNo = 0, .value = false},
        SetWaterSwitchEvent{
            .timeMs = 5 * 60 * 1000 + 10005, .nodeId = 40, .devNo = 1, .value = false},
        SetWaterSwitchEvent{
            .timeMs = 5 * 60 * 1000 + 10010, .nodeId = 60, .devNo = 2, .value = false},
        SetWaterSwitchEvent{
            .timeMs = 5 * 60 * 1000 + 10015, .nodeId = 50, .devNo = 3, .value = false},

        DigitalPinChangedEvent{.timeMs = 5 * 60 * 1000 + 10021,
                               .pin = PINS_PUMP_SWITCH[0],
                               .value = false,
                               .pinMode = OUTPUT},
        DigitalPinChangedEvent{.timeMs = 5 * 60 * 1000 + 10021,
                               .pin = PINS_DC_ADAPTER_SWITCH[0],
                               .value = false,
                               .pinMode = OUTPUT},
    };
    EXPECT_THAT(events, IsSupersequenceOf(expected_events));
}

TEST_F(ScenariosTest, multistep_singlenode)
{
    Scenario scenario{
        .stepCount = 4,
        .steps = {
            ScenarioStep{.nodes = {ScenarioStepNode{30, 0}, ScenarioStepNode{NO_NODE, 0}},
                         .runTimeSec = 20},
            ScenarioStep{.nodes = {ScenarioStepNode{40, 0}, ScenarioStepNode{NO_NODE, 0}},
                         .runTimeSec = 30},
            ScenarioStep{.nodes = {ScenarioStepNode{60, 0}, ScenarioStepNode{NO_NODE, 0}},
                         .runTimeSec = 50},
            ScenarioStep{.nodes = {ScenarioStepNode{50, 0}, ScenarioStepNode{NO_NODE, 0}},
                         .runTimeSec = 40},
        }};
    std::vector<std::shared_ptr<SlaveNodeSims>> slave_nodes(
        {std::make_shared<SlaveNodeSims>(can_bus, 30), std::make_shared<SlaveNodeSims>(can_bus, 40),
         std::make_shared<SlaveNodeSims>(can_bus, 50),
         std::make_shared<SlaveNodeSims>(can_bus, 60)});
    for (auto node : slave_nodes)
    {
        can_bus.attachDevice(node);
    }

    runner.start(&scenario);
    EXPECT_EQ(runner.totalSteps(), 6);
    EXPECT_EQ(runner.currentState(), ScenarioRunnerState::Running);

    ctx.run_for(160 * 1000);
    EXPECT_EQ(runner.currentState(), ScenarioRunnerState::Finished);
    EXPECT_FALSE(runner.hasError());

    // time may flow a little due to simulated delays
    // and relatively high time increment between two loop() calls
    std::vector<Event> expected_events{
        DigitalPinChangedEvent{
            .timeMs = 0, .pin = PINS_PUMP_SWITCH[0], .value = true, .pinMode = OUTPUT},
        DigitalPinChangedEvent{
            .timeMs = 0, .pin = PINS_DC_ADAPTER_SWITCH[0], .value = true, .pinMode = OUTPUT},

        SetWaterSwitchEvent{.timeMs = 10001, .nodeId = 30, .devNo = 0, .value = true},
        SetWaterSwitchEvent{.timeMs = 30000, .nodeId = 30, .devNo = 0, .value = false},

        SetWaterSwitchEvent{.timeMs = 30006, .nodeId = 40, .devNo = 0, .value = true},
        SetWaterSwitchEvent{.timeMs = 60005, .nodeId = 40, .devNo = 0, .value = false},

        SetWaterSwitchEvent{.timeMs = 60011, .nodeId = 60, .devNo = 0, .value = true},
        SetWaterSwitchEvent{.timeMs = 110010, .nodeId = 60, .devNo = 0, .value = false},

        SetWaterSwitchEvent{.timeMs = 110016, .nodeId = 50, .devNo = 0, .value = true},
        SetWaterSwitchEvent{.timeMs = 150015, .nodeId = 50, .devNo = 0, .value = false},

        DigitalPinChangedEvent{
            .timeMs = 150021, .pin = PINS_PUMP_SWITCH[0], .value = false, .pinMode = OUTPUT},
        DigitalPinChangedEvent{
            .timeMs = 150021, .pin = PINS_DC_ADAPTER_SWITCH[0], .value = false, .pinMode = OUTPUT},
    };
    EXPECT_THAT(events, IsSupersequenceOf(expected_events));
}

TEST_F(ScenariosTest, multistep_multinode)
{
    Scenario scenario{.stepCount = 2,
                      .steps = {
                          ScenarioStep{.nodes = {ScenarioStepNode{30, 0}, ScenarioStepNode{60, 0},
                                                 ScenarioStepNode{NO_NODE, 0}},
                                       .runTimeSec = 20},
                          ScenarioStep{.nodes = {ScenarioStepNode{40, 0}, ScenarioStepNode{50, 0},
                                                 ScenarioStepNode{NO_NODE, 0}},
                                       .runTimeSec = 30},
                      }};
    std::vector<std::shared_ptr<SlaveNodeSims>> slave_nodes(
        {std::make_shared<SlaveNodeSims>(can_bus, 30), std::make_shared<SlaveNodeSims>(can_bus, 40),
         std::make_shared<SlaveNodeSims>(can_bus, 50),
         std::make_shared<SlaveNodeSims>(can_bus, 60)});
    for (auto node : slave_nodes)
    {
        can_bus.attachDevice(node);
    }

    runner.start(&scenario);
    EXPECT_EQ(runner.currentState(), ScenarioRunnerState::Running);

    ctx.run_for(160 * 1000);
    EXPECT_EQ(runner.currentState(), ScenarioRunnerState::Finished);
    EXPECT_FALSE(runner.hasError());

    // time may flow a little due to simulated delays
    // and relatively high time increment between two loop() calls
    std::vector<Event> expected_events{
        DigitalPinChangedEvent{
            .timeMs = 0, .pin = PINS_PUMP_SWITCH[0], .value = true, .pinMode = OUTPUT},
        DigitalPinChangedEvent{
            .timeMs = 0, .pin = PINS_DC_ADAPTER_SWITCH[0], .value = true, .pinMode = OUTPUT},

        SetWaterSwitchEvent{.timeMs = 10001, .nodeId = 30, .devNo = 0, .value = true},
        SetWaterSwitchEvent{.timeMs = 10006, .nodeId = 60, .devNo = 0, .value = true},
        SetWaterSwitchEvent{.timeMs = 30000, .nodeId = 30, .devNo = 0, .value = false},
        SetWaterSwitchEvent{.timeMs = 30005, .nodeId = 60, .devNo = 0, .value = false},

        SetWaterSwitchEvent{.timeMs = 30011, .nodeId = 40, .devNo = 0, .value = true},
        SetWaterSwitchEvent{.timeMs = 30016, .nodeId = 50, .devNo = 0, .value = true},
        SetWaterSwitchEvent{.timeMs = 60010, .nodeId = 40, .devNo = 0, .value = false},
        SetWaterSwitchEvent{.timeMs = 60015, .nodeId = 50, .devNo = 0, .value = false},

        DigitalPinChangedEvent{
            .timeMs = 60021, .pin = PINS_PUMP_SWITCH[0], .value = false, .pinMode = OUTPUT},
        DigitalPinChangedEvent{
            .timeMs = 60021, .pin = PINS_DC_ADAPTER_SWITCH[0], .value = false, .pinMode = OUTPUT},
    };
    EXPECT_THAT(events, IsSupersequenceOf(expected_events));
}

TEST_F(ScenariosTest, step_partially_succeded__no_node_answer)
{
    // In this test we expect to see error, but other nodes which are involved on the same step
    // should work.
    // Also we're checking that scenario is trying to turn the problematic node on and off.

    Scenario scenario{.stepCount = 2,
                      .steps = {
                          ScenarioStep{.nodes = {ScenarioStepNode{30, 0}, ScenarioStepNode{60, 0},
                                                 ScenarioStepNode{NO_NODE, 0}},
                                       .runTimeSec = 20},
                          ScenarioStep{.nodes = {ScenarioStepNode{40, 0}, ScenarioStepNode{50, 0},
                                                 ScenarioStepNode{NO_NODE, 0}},
                                       .runTimeSec = 30},
                      }};

    // node 40 does not answer (e.g. because it's not connected)
    std::vector<std::shared_ptr<SlaveNodeSims>> slave_nodes(
        {std::make_shared<SlaveNodeSims>(can_bus, 30),
         std::make_shared<SlaveNodeSims>(can_bus, 40, SlaveNodeSims::Behavior::NoAnswer),
         std::make_shared<SlaveNodeSims>(can_bus, 50),
         std::make_shared<SlaveNodeSims>(can_bus, 60)});
    for (auto node : slave_nodes)
    {
        can_bus.attachDevice(node);
    }

    runner.start(&scenario);
    EXPECT_EQ(runner.currentState(), ScenarioRunnerState::Running);

    ctx.run_for(160 * 1000);
    EXPECT_EQ(runner.currentState(), ScenarioRunnerState::Finished);
    EXPECT_TRUE(runner.hasError());

    // time may flow a little due to simulated delays
    // and relatively high time increment between two loop() calls
    std::vector<Event> expected_events{
        DigitalPinChangedEvent{
            .timeMs = 0, .pin = PINS_PUMP_SWITCH[0], .value = true, .pinMode = OUTPUT},
        DigitalPinChangedEvent{
            .timeMs = 0, .pin = PINS_DC_ADAPTER_SWITCH[0], .value = true, .pinMode = OUTPUT},

        SetWaterSwitchEvent{.timeMs = 10001, .nodeId = 30, .devNo = 0, .value = true},
        SetWaterSwitchEvent{.timeMs = 10006, .nodeId = 60, .devNo = 0, .value = true},
        SetWaterSwitchEvent{.timeMs = 30000, .nodeId = 30, .devNo = 0, .value = false},
        SetWaterSwitchEvent{.timeMs = 30005, .nodeId = 60, .devNo = 0, .value = false},

        SetWaterSwitchEvent{.timeMs = 30011, .nodeId = 40, .devNo = 0, .value = true},
        ErrorFlagChanged{.timeMs = 31012, .value = true},
        SetWaterSwitchEvent{.timeMs = 31012, .nodeId = 50, .devNo = 0, .value = true},
        SetWaterSwitchEvent{.timeMs = 60010, .nodeId = 40, .devNo = 0, .value = false},
        SetWaterSwitchEvent{.timeMs = 61011, .nodeId = 50, .devNo = 0, .value = false},

        DigitalPinChangedEvent{
            .timeMs = 61017, .pin = PINS_PUMP_SWITCH[0], .value = false, .pinMode = OUTPUT},
        DigitalPinChangedEvent{
            .timeMs = 61017, .pin = PINS_DC_ADAPTER_SWITCH[0], .value = false, .pinMode = OUTPUT},
    };
    EXPECT_THAT(events, IsSupersequenceOf(expected_events));
}

TEST_F(ScenariosTest, whole_step_failed__no_node_answer)
{
    // In this test whole step fails. There's no sense in waiting while step finishes.
    // We're still checking that scenario is trying to turn the problematic nodes on and off.

    Scenario scenario{.stepCount = 2,
                      .steps = {
                          ScenarioStep{.nodes = {ScenarioStepNode{30, 0}, ScenarioStepNode{60, 0},
                                                 ScenarioStepNode{NO_NODE, 0}},
                                       .runTimeSec = 20},
                          ScenarioStep{.nodes = {ScenarioStepNode{40, 0}, ScenarioStepNode{50, 0},
                                                 ScenarioStepNode{NO_NODE, 0}},
                                       .runTimeSec = 30},
                      }};

    // nodes 30 and 60 do not answer (e.g. because they're not connected)
    std::vector<std::shared_ptr<SlaveNodeSims>> slave_nodes(
        {std::make_shared<SlaveNodeSims>(can_bus, 30, SlaveNodeSims::Behavior::NoAnswer),
         std::make_shared<SlaveNodeSims>(can_bus, 40), std::make_shared<SlaveNodeSims>(can_bus, 50),
         std::make_shared<SlaveNodeSims>(can_bus, 60, SlaveNodeSims::Behavior::NoAnswer)});
    for (auto node : slave_nodes)
    {
        can_bus.attachDevice(node);
    }

    runner.start(&scenario);
    EXPECT_EQ(runner.currentState(), ScenarioRunnerState::Running);

    ctx.run_for(160 * 1000);
    EXPECT_EQ(runner.currentState(), ScenarioRunnerState::Finished);
    EXPECT_TRUE(runner.hasError());

    // time may flow a little due to simulated delays
    // and relatively high time increment between two loop() calls
    std::vector<Event> expected_events{
        DigitalPinChangedEvent{
            .timeMs = 0, .pin = PINS_PUMP_SWITCH[0], .value = true, .pinMode = OUTPUT},
        DigitalPinChangedEvent{
            .timeMs = 0, .pin = PINS_DC_ADAPTER_SWITCH[0], .value = true, .pinMode = OUTPUT},

        SetWaterSwitchEvent{.timeMs = 10001, .nodeId = 30, .devNo = 0, .value = true},
        ErrorFlagChanged{.timeMs = 11002, .value = true},
        SetWaterSwitchEvent{.timeMs = 11002, .nodeId = 60, .devNo = 0, .value = true},
        SetWaterSwitchEvent{.timeMs = 12003, .nodeId = 30, .devNo = 0, .value = false},
        SetWaterSwitchEvent{.timeMs = 13004, .nodeId = 60, .devNo = 0, .value = false},

        SetWaterSwitchEvent{.timeMs = 14006, .nodeId = 40, .devNo = 0, .value = true},
        SetWaterSwitchEvent{.timeMs = 14011, .nodeId = 50, .devNo = 0, .value = true},
        SetWaterSwitchEvent{.timeMs = 44005, .nodeId = 40, .devNo = 0, .value = false},
        SetWaterSwitchEvent{.timeMs = 44010, .nodeId = 50, .devNo = 0, .value = false},

        DigitalPinChangedEvent{
            .timeMs = 44016, .pin = PINS_PUMP_SWITCH[0], .value = false, .pinMode = OUTPUT},
        DigitalPinChangedEvent{
            .timeMs = 44016, .pin = PINS_DC_ADAPTER_SWITCH[0], .value = false, .pinMode = OUTPUT},
    };
    EXPECT_THAT(events, IsSupersequenceOf(expected_events));
}

TEST_F(ScenariosTest, step_partially_succeded__can_sending_error)
{
    // In this test we expect to see error, but other nodes which are involved on the same step
    // should work.
    // Also we're checking that scenario is trying to turn the problematic node on and off.

    Scenario scenario{.stepCount = 2,
                      .steps = {
                          ScenarioStep{.nodes = {ScenarioStepNode{30, 0}, ScenarioStepNode{60, 0},
                                                 ScenarioStepNode{NO_NODE, 0}},
                                       .runTimeSec = 20},
                          ScenarioStep{.nodes = {ScenarioStepNode{40, 0}, ScenarioStepNode{50, 0},
                                                 ScenarioStepNode{NO_NODE, 0}},
                                       .runTimeSec = 30},
                      }};

    std::vector<std::shared_ptr<SlaveNodeSims>> slave_nodes(
        {std::make_shared<SlaveNodeSims>(can_bus, 30),
         std::make_shared<SlaveNodeSims>(can_bus, 40),
         std::make_shared<SlaveNodeSims>(can_bus, 50),
         std::make_shared<SlaveNodeSims>(can_bus, 60)});
    for (auto node : slave_nodes)
    {
        can_bus.attachDevice(node);
    }

    // error on CAN bus while sending command to node 40
    can_bus.sendMsgBufBehavior = CanBusSims::sendMsgBufCustomBehaviour_t{
        .on_event = [&](INT32U id, INT8U ext, INT8U len, INT8U *buf) -> INT8U {
            return id == 40 ? CAN_FAIL : CAN_OK;
        }};

    runner.start(&scenario);
    EXPECT_EQ(runner.currentState(), ScenarioRunnerState::Running);

    ctx.run_for(160 * 1000);
    EXPECT_EQ(runner.currentState(), ScenarioRunnerState::Finished);
    EXPECT_TRUE(runner.hasError());

    // time may flow a little due to simulated delays
    // and relatively high time increment between two loop() calls
    std::vector<Event> expected_events{
        DigitalPinChangedEvent{
            .timeMs = 0, .pin = PINS_PUMP_SWITCH[0], .value = true, .pinMode = OUTPUT},
        DigitalPinChangedEvent{
            .timeMs = 0, .pin = PINS_DC_ADAPTER_SWITCH[0], .value = true, .pinMode = OUTPUT},

        SetWaterSwitchEvent{.timeMs = 10001, .nodeId = 30, .devNo = 0, .value = true},
        SetWaterSwitchEvent{.timeMs = 10006, .nodeId = 60, .devNo = 0, .value = true},
        SetWaterSwitchEvent{.timeMs = 30000, .nodeId = 30, .devNo = 0, .value = false},
        SetWaterSwitchEvent{.timeMs = 30005, .nodeId = 60, .devNo = 0, .value = false},

        ErrorFlagChanged{.timeMs = 30011, .value = true},
        SetWaterSwitchEvent{.timeMs = 30011, .nodeId = 50, .devNo = 0, .value = true},
        SetWaterSwitchEvent{.timeMs = 60010, .nodeId = 50, .devNo = 0, .value = false},

        DigitalPinChangedEvent{
            .timeMs = 60016, .pin = PINS_PUMP_SWITCH[0], .value = false, .pinMode = OUTPUT},
        DigitalPinChangedEvent{
            .timeMs = 60016, .pin = PINS_DC_ADAPTER_SWITCH[0], .value = false, .pinMode = OUTPUT},
    };
    EXPECT_THAT(events, IsSupersequenceOf(expected_events));
}

TEST_F(ScenariosTest, whole_step_failed__can_sending_error)
{
    // In this test whole step fails. There's no sense in waiting while step finishes.
    // We're still checking that scenario is trying to turn the problematic nodes on and off.

    Scenario scenario{.stepCount = 2,
                      .steps = {
                          ScenarioStep{.nodes = {ScenarioStepNode{30, 0}, ScenarioStepNode{60, 0},
                                                 ScenarioStepNode{NO_NODE, 0}},
                                       .runTimeSec = 20},
                          ScenarioStep{.nodes = {ScenarioStepNode{40, 0}, ScenarioStepNode{50, 0},
                                                 ScenarioStepNode{NO_NODE, 0}},
                                       .runTimeSec = 30},
                      }};

    // nodes 30 and 60 do not answer (e.g. because they're not connected)
    std::vector<std::shared_ptr<SlaveNodeSims>> slave_nodes(
        {std::make_shared<SlaveNodeSims>(can_bus, 30),
         std::make_shared<SlaveNodeSims>(can_bus, 40), std::make_shared<SlaveNodeSims>(can_bus, 50),
         std::make_shared<SlaveNodeSims>(can_bus, 60)});
    for (auto node : slave_nodes)
    {
        can_bus.attachDevice(node);
    }

    // error on CAN bus while sending command to nodes 30 and 60
    can_bus.sendMsgBufBehavior = CanBusSims::sendMsgBufCustomBehaviour_t{
        .on_event = [&](INT32U id, INT8U ext, INT8U len, INT8U *buf) -> INT8U {
            return (id == 30 || id == 60) ? CAN_FAIL : CAN_OK;
        }};

    runner.start(&scenario);
    EXPECT_EQ(runner.currentState(), ScenarioRunnerState::Running);

    ctx.run_for(160 * 1000);
    EXPECT_EQ(runner.currentState(), ScenarioRunnerState::Finished);
    EXPECT_TRUE(runner.hasError());

    // time may flow a little due to simulated delays
    // and relatively high time increment between two loop() calls
    std::vector<Event> expected_events{
        DigitalPinChangedEvent{
            .timeMs = 0, .pin = PINS_PUMP_SWITCH[0], .value = true, .pinMode = OUTPUT},
        DigitalPinChangedEvent{
            .timeMs = 0, .pin = PINS_DC_ADAPTER_SWITCH[0], .value = true, .pinMode = OUTPUT},

        ErrorFlagChanged{.timeMs = 10001, .value = true},

        SetWaterSwitchEvent{.timeMs = 10003, .nodeId = 40, .devNo = 0, .value = true},
        SetWaterSwitchEvent{.timeMs = 10008, .nodeId = 50, .devNo = 0, .value = true},
        SetWaterSwitchEvent{.timeMs = 40003, .nodeId = 40, .devNo = 0, .value = false},
        SetWaterSwitchEvent{.timeMs = 40008, .nodeId = 50, .devNo = 0, .value = false},

        DigitalPinChangedEvent{
            .timeMs = 40014, .pin = PINS_PUMP_SWITCH[0], .value = false, .pinMode = OUTPUT},
        DigitalPinChangedEvent{
            .timeMs = 40014, .pin = PINS_DC_ADAPTER_SWITCH[0], .value = false, .pinMode = OUTPUT},
    };
    EXPECT_THAT(events, IsSupersequenceOf(expected_events));
}

TEST_F(ScenariosTest, scenario_stop)
{
    Scenario scenario{
        .stepCount = 4,
        .steps = {
            ScenarioStep{.nodes = {ScenarioStepNode{30, 0}, ScenarioStepNode{NO_NODE, 0}},
                         .runTimeSec = 20},
            ScenarioStep{.nodes = {ScenarioStepNode{40, 0}, ScenarioStepNode{NO_NODE, 0}},
                         .runTimeSec = 30},
            ScenarioStep{.nodes = {ScenarioStepNode{60, 0}, ScenarioStepNode{NO_NODE, 0}},
                         .runTimeSec = 50},
            ScenarioStep{.nodes = {ScenarioStepNode{50, 0}, ScenarioStepNode{NO_NODE, 0}},
                         .runTimeSec = 40},
        }};
    std::vector<std::shared_ptr<SlaveNodeSims>> slave_nodes(
        {std::make_shared<SlaveNodeSims>(can_bus, 30), std::make_shared<SlaveNodeSims>(can_bus, 40),
         std::make_shared<SlaveNodeSims>(can_bus, 50),
         std::make_shared<SlaveNodeSims>(can_bus, 60)});
    for (auto node : slave_nodes)
    {
        can_bus.attachDevice(node);
    }

    bool stopped = false;
    additional_loop_step = [&] {
        if (millis() > 20000 && !stopped)
        {
            runner.stop();
            stopped = true;
        }
    };

    runner.start(&scenario);
    EXPECT_EQ(runner.currentState(), ScenarioRunnerState::Running);

    ctx.run_for(23 * 1000);
    EXPECT_EQ(runner.currentState(), ScenarioRunnerState::Finished);
    EXPECT_FALSE(runner.hasError());

    // time may flow a little due to simulated delays
    // and relatively high time increment between two loop() calls
    std::vector<Event> expected_events{
        DigitalPinChangedEvent{
            .timeMs = 0, .pin = PINS_PUMP_SWITCH[0], .value = true, .pinMode = OUTPUT},
        DigitalPinChangedEvent{
            .timeMs = 0, .pin = PINS_DC_ADAPTER_SWITCH[0], .value = true, .pinMode = OUTPUT},

        SetWaterSwitchEvent{.timeMs = 10001, .nodeId = 30, .devNo = 0, .value = true},

        // stop called
        DigitalPinChangedEvent{
            .timeMs = 20002, .pin = PINS_PUMP_SWITCH[0], .value = false, .pinMode = OUTPUT},
        DigitalPinChangedEvent{
            .timeMs = 20002, .pin = PINS_DC_ADAPTER_SWITCH[0], .value = false, .pinMode = OUTPUT},
    };
    EXPECT_THAT(events, IsSupersequenceOf(expected_events));
}

TEST(ScenarioFuncs, SerializeDeserialize)
{
    struct TestData
    {
        Scenario scenario;
        std::string serialized;
    };
    std::vector<TestData> test_data =
        {
            {Scenario{.stepCount = 0, .steps = {}}, ""},
            {Scenario{.stepCount = 1,
                      .steps = {ScenarioStep{
                          .nodes = {ScenarioStepNode{20, 1}, ScenarioStepNode{NO_NODE, 0}},
                          .runTimeSec = 15}}},
             "15:20,1"},
            {Scenario{
                 .stepCount = 1,
                 .steps = {ScenarioStep{.nodes = {ScenarioStepNode{30, 0}, ScenarioStepNode{40, 1},
                                                  ScenarioStepNode{60, 2}, ScenarioStepNode{50, 3}},
                                        .runTimeSec = 5 * 60}}},
             "300:30,0;40,1;60,2;50,3"},
            {Scenario{
                 .stepCount = 4,
                 .steps =
                     {
                         ScenarioStep{.nodes = {ScenarioStepNode{30, 0},
                                                ScenarioStepNode{NO_NODE, 0}},
                                      .runTimeSec = 20},
                         ScenarioStep{.nodes = {ScenarioStepNode{40, 0},
                                                ScenarioStepNode{NO_NODE, 0}},
                                      .runTimeSec = 30},
                         ScenarioStep{.nodes = {ScenarioStepNode{60, 0},
                                                ScenarioStepNode{NO_NODE, 0}},
                                      .runTimeSec = 50},
                         ScenarioStep{.nodes = {ScenarioStepNode{50, 0},
                                                ScenarioStepNode{NO_NODE, 0}},
                                      .runTimeSec = 40},
                     }},
             "20:30,0|30:40,0|50:60,0|40:50,0"},
            {Scenario{
                 .stepCount = 2,
                 .steps =
                     {
                         ScenarioStep{.nodes = {ScenarioStepNode{30, 0}, ScenarioStepNode{60, 0},
                                                ScenarioStepNode{NO_NODE, 0}},
                                      .runTimeSec = 20},
                         ScenarioStep{.nodes = {ScenarioStepNode{40, 0}, ScenarioStepNode{50, 0},
                                                ScenarioStepNode{NO_NODE, 0}},
                                      .runTimeSec = 30},
                     }},
             "20:30,0;60,0|30:40,0;50,0"},
        };

    for (auto &item : test_data)
    {
        LOG_INFO("Testing pattern: ", item.serialized);

        Stream stream;
        serialize(item.scenario, stream);
        EXPECT_EQ(stream.str(), item.serialized);

        Scenario scenario_deserialized;
        // deserialize works until \n is found; next chars should be ignored
        stream.str(item.serialized + "\nabcd2345521");
        stream.seekg(0);
        EXPECT_TRUE(deserialize(stream, scenario_deserialized));
        EXPECT_EQ(item.scenario, scenario_deserialized);
        EXPECT_EQ(stream.peek(), 'a'); // first symbol after \n
    }
}
