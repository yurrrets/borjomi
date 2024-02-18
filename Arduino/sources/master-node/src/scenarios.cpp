#include "scenarios.h"
#include "cap_implementation.h"
#include "nodes.h"

// First step is always special "warm up" step #0.
// Then steps from scenario follow.
// And after them "cool down" step is executed.
// So, we always have two more steps then scenario has

namespace
{
uint16_t StepWarmUpSec = 10;
uint16_t StepCoolDowsSec = 2;
} // namespace

ScenarioRunner::ScenarioRunner(CanCommands &canCommands)
    : canCommands(canCommands), running(false), errorFlag(false), scenario(nullptr), step(0),
      stepStartTime(0), lastCanMsgNo(INVALID_CAN_MSG_NO)
{
}

void ScenarioRunner::loop()
{
    if (step >= totalSteps())
    {
        running = false;
        return;
    }
    if (step < 0)
    {
        step = -1;
    }
    if ((step < 0) || (timeLeftForCurrentStep() == 0 && node >= currentStepNodeCount()))
    {
        step++;
        node = 0;
        return;
    }

    // start/finish are actual when there's smth. to do;
    // nothing to do - just return
    if (node >= currentStepNodeCount())
    {
        return;
    }

    if (timeLeftForCurrentStep() == 0)
        finishPartStep();
    else
        startPartStep();
}

void ScenarioRunner::start(Scenario *scenario)
{
    if (running)
    {
        // already running
        // TODO: print error
        return;
    }
    this->scenario = scenario;
    errorFlag = false;

    if (scenario->steps == 0)
    {
        return;
    }

    running = true;
    step = -1;

    // loop() will do the rest
}

void ScenarioRunner::stop()
{
    // jump to last step
}

bool ScenarioRunner::isRunning() const
{
    return running;
}

int8_t ScenarioRunner::currentStep() const
{
    return step;
}

uint8_t ScenarioRunner::totalSteps() const
{
    if (!scenario)
    {
        return 0;
    }
    // plus "warm up" and "cool down" steps
    return scenario->stepCount + 2;
}

uint16_t ScenarioRunner::totalTimeSec() const
{
    return timeLeftFromStep(0);
}

uint16_t ScenarioRunner::timeLeftForCurrentStep() const
{
    unsigned long stepRunTime = millis() - stepStartTime;
    long diff = stepTimeSec(currentStep()) - stepRunTime;
    return diff < 0 ? 0 : diff;
}

uint16_t ScenarioRunner::totalTimeLeft() const
{
    return timeLeftForCurrentStep() + timeLeftFromStep(currentStep() + 1);
}

uint16_t ScenarioRunner::timeLeftFromStep(int8_t step) const
{
    uint16_t ret = 0;
    for (uint8_t i = step; i < totalSteps(); i++)
    {
        ret += stepTimeSec(i);
    }
    return ret;
}

uint16_t ScenarioRunner::stepTimeSec(int8_t step) const
{
    if (step < 0 || step >= totalSteps())
    {
        return 0;
    }
    if (isWarmUpStep(step))
    {
        return StepWarmUpSec;
    }
    if (isCoolDownStep(step))
    {
        return StepCoolDowsSec;
    }
    return scenario->steps[step - 1].runTimeSec;
}

bool ScenarioRunner::isWarmUpStep(int8_t step) const
{
    return (scenario != nullptr) && (step == 0);
}

bool ScenarioRunner::isCoolDownStep(int8_t step) const
{
    return (scenario != nullptr) && (step == scenario->stepCount + 1);
}

int8_t ScenarioRunner::currentStepNodeCount() const
{
    if (step < 0 || step >= totalSteps())
    {
        return 0;
    }
    if (isWarmUpStep(step) || isCoolDownStep(step))
    {
        return 1;
    }

    auto &scenarioStep = scenario->steps[step - 1];
    for (int8_t i = 0; i < ScenarioStep::MaxNodeCount; i++)
    {
        if (scenarioStep.nodes[i].address == NO_NODE)
        {
            return i;
        }
    }
    return ScenarioStep::MaxNodeCount;
}

void ScenarioRunner::startPartStep()
{
    if (isWarmUpStep(step))
    {
        setControlSwitchVal(CS_PUMP, true);
        setControlSwitchVal(CS_DC_ADAPTER, true);
        node = 1;
    }
    else if (isCoolDownStep(step))
    {
        node = 1;
    }
    else
    {
        processCanCommand(true);
    }
}

void ScenarioRunner::finishPartStep()
{
    if (isWarmUpStep(step))
    {
        node = 1;
    }
    else if (isCoolDownStep(step))
    {
        setControlSwitchVal(CS_PUMP, false);
        setControlSwitchVal(CS_DC_ADAPTER, false);
        node = 1;
    }
    else
    {
        processCanCommand(false);
    }
}

void ScenarioRunner::processCanCommand(bool reqOpenOrClose)
{
    if (canCommands.isBusy())
    {
        return;
    }
    switch (canCommands.readStatus())
    {
    case CanCommands::S_OK: {
        if (canCommands.getAnswer().msgno == lastCanMsgNo)
        {
            // it's our answer
            lastCanMsgNo = INVALID_CAN_MSG_NO;
            node++;
        }
        break;
    }
    case CanCommands::S_INVALID_MSG:
    case CanCommands::S_INVALID_CRC:
    case CanCommands::S_TIMEOUT_ERR: {
        if (lastCanMsgNo != INVALID_CAN_MSG_NO)
        {
            // it was our request
            errorFlag = true;
            lastCanMsgNo = INVALID_CAN_MSG_NO;
            node++;
        }
        break;
    }
    case CanCommands::S_NO_DATA: {
        auto &scenarioStepNode = scenario->steps[step - 1].nodes[node];
        canCommands.sendRequest(scenarioStepNode.address, CMD_SET_WATER_SWITCH,
                                scenarioStepNode.devNo, reqOpenOrClose ? CVAL_OPENED : CVAL_CLOSED);
        lastCanMsgNo = canCommands.getRequest().msgno;
        break;
    }
    default:
        break;
    }
}