#ifndef CAP_IMPLEMENTATION_H
#define CAP_IMPLEMENTATION_H

enum SensorType
{
    S_PRESSURE = 1,
    S_DC_CURRENT,
    S_DC_VOLTAGE
};
float readSensorValue(SensorType sensorType);

enum ControlSwitch
{
    CS_DC_ADAPTER = 1,
    CS_PUMP
};
void setControlSwitchVal(ControlSwitch controlSwitch, bool state);
bool getControlSwitchVal(ControlSwitch controlSwitch);

#endif // CAP_IMPLEMENTATION_H
