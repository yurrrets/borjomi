#include "scenarios.h"
#include "cap_implementation.h"
#include "log.h"

// First step is always special "warm up" step #0.
// Then steps from scenario follow.
// And after them "cool down" step is executed.
// So, we always have two more steps then scenario has

namespace
{
constexpr uint16_t StepWarmUpSec = 10;
constexpr uint16_t StepCoolDowsSec = 2;

// better to use numeric_limits, but can't compile it with Arduino.h
constexpr int8_t max_node = INT8_MAX;
} // namespace

ScenarioRunner::ScenarioRunner(CanCommands &canCommands)
    : canCommands(canCommands), state(ScenarioRunnerState::Ready), errorFlag(false),
      scenario(nullptr), step(0), stepStartTime(0), lastCanMsgNo(INVALID_CAN_MSG_NO)
{
}

void ScenarioRunner::loop()
{
    if (totalSteps() == 0)
    {
        // prevent state change to Finished
        return;
    }
    if (step >= totalSteps())
    {
        state = ScenarioRunnerState::Finished;
        return;
    }
    if (step < 0)
    {
        step = -1;
    }
    if ((step < 0) || (!startPhase && node >= currentStepNodeCount()))
    {
        step++;
        stepStartTime = millis();
        node = 0;
        stepPartialSucceesFlag = false;
        startPhase = true;
        return;
    }

    if (startPhase && timeLeftForCurrentStep() == 0)
    {
        startPhase = false;
        node = 0;
    }

    // start/finish are actual when there's smth. to do;
    // nothing to do - just return
    if (node >= currentStepNodeCount())
    {
        return;
    }

    if (startPhase)
        startPartStep();
    else
        finishPartStep();
}

void ScenarioRunner::start(Scenario *scenario)
{
    if (state != ScenarioRunnerState::Ready && state != ScenarioRunnerState::Finished)
    {
        // already running
        LOG_ERROR(F("can't start scenario, invalid state"));
        return;
    }
    this->scenario = scenario;
    errorFlag = false;

    if (scenario->steps == 0)
    {
        return;
    }

    state = ScenarioRunnerState::Running;
    step = -1;

    // loop() will do the rest
}

void ScenarioRunner::stop()
{
    // jump to last step
    // it's not super clear, but should work
    uint8_t total = totalSteps();
    if (total >= 2 && step <= total - 2 && node != max_node)
    {
        // fast forward to the moment right before latest step
        step = total - 2;
        startPhase = false;
        node = max_node;
    }
}

ScenarioRunnerState ScenarioRunner::currentState() const
{
    return state;
}

bool ScenarioRunner::hasError() const
{
    return errorFlag;
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
    // special case: detect the situation when all nodes on current step failed
    // there's no sense to wait until step finish, we can finish it now
    if (startPhase && node >= currentStepNodeCount() && !stepPartialSucceesFlag)
    {
        return 0;
    }

    unsigned long stepRunTimeMs = millis() - stepStartTime;
    long diff = stepTimeSec(currentStep()) - (stepRunTimeMs / 1000);
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
        stepPartialSucceesFlag = true;
        node = 1;
    }
    else if (isCoolDownStep(step))
    {
        setControlSwitchVal(CS_PUMP, false);
        setControlSwitchVal(CS_DC_ADAPTER, false);
        stepPartialSucceesFlag = true;
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
            // reset lastCanMsgNo for indicating we're not waiting for an asnwer
            lastCanMsgNo = INVALID_CAN_MSG_NO;
            stepPartialSucceesFlag = true;
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
        auto sendRes = canCommands.sendRequest(scenarioStepNode.address, CMD_SET_WATER_SWITCH,
                                               scenarioStepNode.devNo,
                                               reqOpenOrClose ? CVAL_OPENED : CVAL_CLOSED);
        if (sendRes == CAN_OK)
        {
            lastCanMsgNo = canCommands.getRequest().msgno;
        }
        else
        {
            errorFlag = true;
            node++;
        }
        break;
    }
    default:
        break;
    }
}

void serialize(const Scenario &scenario, Stream &stream)
{
    for (uint8_t i = 0; i < scenario.stepCount; i++)
    {
        if (i != 0)
        {
            stream.print('|'); // new step number
        }
        auto &step = scenario.steps[i];
        stream.print(step.runTimeSec);
        stream.print(":");
        for (uint8_t j = 0; j < ScenarioStep::MaxNodeCount; j++)
        {
            auto &node = step.nodes[j];
            if (node.address == NO_NODE)
            {
                break;
            }

            if (j != 0)
            {
                stream.print(';');
            }
            stream.print((int)node.address);
            stream.print(',');
            stream.print((int)node.devNo);
        }
    }
}

bool deserialize(Stream &stream, Scenario &scenario)
{
    scenario.stepCount = 0;
    while (scenario.stepCount < Scenario::MaxStepCount)
    {
        if (stream.peek() == '\n')
        {
            stream.read();
            break;
        }
        auto &step = scenario.steps[scenario.stepCount];
        step.runTimeSec = stream.parseInt();
        int c = stream.read();
        if (c != ':')
        {
            return false;
        }
        for (uint8_t i = 0; i < ScenarioStep::MaxNodeCount; i++)
        {
            c = stream.peek();
            if (c == '|')
            {
                stream.read();
                step.nodes[i].address = NO_NODE;
                step.nodes[i].devNo = 0;
                break;
            }
            if (c == ';')
            {
                stream.read();
            }
            if (c == '\n')
            {
                break;
            }
            step.nodes[i].address = stream.parseInt();
            c = stream.read();
            if (c != ',')
            {
                return false;
            }
            step.nodes[i].devNo = stream.parseInt();
            if (!stream.available())
            {
                break;
            }
        }
        scenario.stepCount++;
    }
    return true;
}
