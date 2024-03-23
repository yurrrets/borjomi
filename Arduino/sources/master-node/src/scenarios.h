#ifndef RUN_SCENARIOS_H
#define RUN_SCENARIOS_H

#include "Arduino.h"
#include "can_commands.h"
#include "nodes.h"

#pragma pack(push, 1)

struct ScenarioStepNode
{
    uint8_t address = NO_NODE;
    uint8_t devNo = 0;
};

struct ScenarioStep
{
    static constexpr uint8_t MaxNodeCount = 4;
    ScenarioStepNode nodes[MaxNodeCount];
    uint16_t runTimeSec = 0;
};

struct Scenario
{
    static constexpr uint8_t MaxStepCount = 32;

    uint8_t crc = 0;
    uint8_t stepCount = 0;
    ScenarioStep steps[MaxStepCount];
};

#pragma pack(pop)

enum class ScenarioRunnerState : uint8_t
{
    Ready, Running, Finished
};

class ScenarioRunner
{
public:
    ScenarioRunner(CanCommands &canCommands);
    void loop();
    void start(Scenario *scenario);
    void stop();
    ScenarioRunnerState currentState() const;
    bool hasError() const;
    int8_t currentStep() const;
    uint8_t totalSteps() const;
    uint16_t totalTimeSec() const;
    uint16_t timeLeftForCurrentStep() const;
    uint16_t totalTimeLeft() const;
    // void fillCurrentStepDescription();
private:
    CanCommands &canCommands;
    ScenarioRunnerState state;
    bool errorFlag;
    Scenario *scenario;
    int8_t step;
    int8_t node;
    unsigned long stepStartTime;
    bool stepPartialSucceesFlag;
    uint8_t lastCanMsgNo;
    bool startPhase;

    uint16_t timeLeftFromStep(int8_t step) const;
    uint16_t stepTimeSec(int8_t step) const;
    bool isWarmUpStep(int8_t step) const;
    bool isCoolDownStep(int8_t step) const;
    int8_t currentStepNodeCount() const; // number of nodes that should be served on current step

    void startPartStep();
    void finishPartStep();
    void processCanCommand(bool reqOpenOrClose);
};

void serialize(const Scenario &scenario, Stream &stream);
bool deserialize(Stream &stream, Scenario &scenario);

#endif // RUN_SCENARIOS_H
