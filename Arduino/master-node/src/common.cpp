#include "common.h"
#include "config.h"
#include "cap_implementation.h"


bool IsDcVoltageCorrect()
{
    float val_v = readSensorValue(S_DC_VOLTAGE);
    return (val_v >= DC_VOLTAGE_MIN) && (val_v <= DC_VOLTAGE_MAX);
}
