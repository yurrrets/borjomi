#ifndef RUN_SCENARIOS_H
#define RUN_SCENARIOS_H

#include "Arduino.h"
#include "can_commands.h"


#pragma pack(push, 1)

struct ScenarioStepNode
{
    uint8_t address;
    uint8_t devNo;
};

struct ScenarioStep
{
    static constexpr uint8_t MaxNodeCount = 4;
    ScenarioStepNode nodes[MaxNodeCount];
    uint16_t runTimeSec;
};

struct Scenario
{
    ScenarioStep steps[32];
    uint8_t stepCount;
};

#pragma pack(pop)


class ScenarioRunner
{
public:
    ScenarioRunner(CanCommands &canCommands);
    void loop();
    void start(Scenario *scenario);
    void stop();
    bool isRunning() const;
    int8_t currentStep() const;
    uint8_t totalSteps() const;
    uint16_t totalTimeSec() const;
    uint16_t timeLeftForCurrentStep() const;
    uint16_t totalTimeLeft() const;
    // void fillCurrentStepDescription();
private:
    CanCommands &canCommands;
    bool running;
    bool errorFlag;
    Scenario *scenario;
    int8_t step;
    int8_t node;
    unsigned long stepStartTime;
    uint8_t lastCanMsgNo;

    uint16_t timeLeftFromStep(int8_t step) const;
    uint16_t stepTimeSec(int8_t step) const;
    bool isWarmUpStep(int8_t step) const;
    bool isCoolDownStep(int8_t step) const;
    int8_t currentStepNodeCount() const; // number of nodes that should be served on current step

    void startPartStep();
    void finishPartStep();
    void processCanCommand(bool reqOpenOrClose);
};

#endif // RUN_SCENARIOS_H