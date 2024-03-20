#include "common.h"
#include "cap_implementation.h"
#include "config.h"
#include <EEPROM.h>

namespace
{
auto MainScenarioOffset = 64;
}

bool IsDcVoltageCorrect()
{
    float val_v = readSensorValue(S_DC_VOLTAGE);
    return (val_v >= DC_VOLTAGE_MIN) && (val_v <= DC_VOLTAGE_MAX);
}

void storeMainScenarioToPermanentStorage()
{
    auto &scenario = getMainScenario();
    UpdateCrc8(scenario);
    EEPROM.put(MainScenarioOffset, scenario);
}

void restoreMainScenarioFromPermanentStorage()
{
    auto &scenario = getMainScenario();
    EEPROM.get(MainScenarioOffset, scenario);
    if (!CheckCrc8(scenario))
    {
        scenario = Scenario{};
    }
}