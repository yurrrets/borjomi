#ifndef COMMON_H
#define COMMON_H

#include "nodes.h"
#include "scenarios.h"

MasterNodeID &getNodeConfig();
bool IsDcVoltageCorrect();

Scenario &getMainScenario();
void storeMainScenarioToPermanentStorage();
void restoreMainScenarioFromPermanentStorage();

#endif // COMMON_H
