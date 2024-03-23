// CAN Send Example
//

#include <SoftwareSerial.h>

#include "log.h"
#include "can_message.h"
#include "cmd_codes.h"
#include "config.h"
#include "stream_ext.h"

#include "bt_commands.h"
#include "button.h"
#include "can_commands.h"
#include "cap_implementation.h"
#include "common.h"
#include "nodes.h"
#include "pc_start.h"
#include "scenarios.h"

#include "LiquidCrystalDisplayCache.h"
#include "LiquidCrystalDisplayEx.h"
#include "LiquidCrystalDisplayEx.ipp"

#include <EEPROM.h>

#define BT_RX_PIN (4)
#define BT_TX_PIN (10)
#define CAN_CS_PIN (9)
#define CAN_INT_PIN (5)
#define BTN_MAIN_PIN (2)
#define LCD_CS_PIN (6)

#ifdef PC_START
#define PC_START_PIN (8) // digital pin
#endif


Print &dbgOutput = Serial;

// SoftwareSerial btSerial(BT_RX_PIN, BT_TX_PIN, 0);
// auto &ioSerial = btSerial;
auto &ioSerial = Serial;

StreamExt ioExtSerial(ioSerial);
MCP_CAN can0(CAN_CS_PIN);
CanCommands canCommands(can0, CAN_INT_PIN);
BTCommandParser ioCommandParser(ioExtSerial);
BTCommandProcessor btCommandProcessor(ioCommandParser, canCommands);
Button mainButton(BTN_MAIN_PIN);
ScenarioRunner scenarioRunner(canCommands);

LiquidCrystalShiftRegSPIIface lcdIface(LCD_CS_PIN);
LiquidCrystalDisplay lcd_hw(lcdIface);
LiquidCrystalDisplayCache<16, 2, decltype(lcd_hw)> lcd(lcd_hw);

MasterNodeID NodeConfig;
Scenario mainScenario;
#ifdef PC_START
PcStart pcStart(PC_START_PIN);
#endif

MasterNodeID &getNodeConfig()
{
    return NodeConfig;
}

Scenario &getMainScenario()
{
    return mainScenario;
}

void updateLcd();

void setup()
{
    // set the data rate for the SoftwareSerial port
    ioSerial.begin(9600);

    // wait for serial port to connect. Needed for native USB
    while (!Serial)
    {
        ;
    }
    LOG_INFO(F("Starting"));

    lcd.begin();
    lcd.print(F("Starting..."));
    lcd.flush();

#ifdef PC_START
    pcStart.setup();
#endif

    EEPROM.get(0, NodeConfig);
    if (CheckCrc8(NodeConfig) && NodeConfig.nodeId)
    {
        LOG_INFO(F("I'm the Master Node ID "), NodeConfig.nodeId);
    }
    else
    {
        LOG_WARNING(F("I'm the Master Node with UNDEFINED ID!"));
        NodeConfig.nodeId = UNKNOWN_NODE;
    }

    restoreMainScenarioFromPermanentStorage();

    canCommands.setup();
    btCommandProcessor.setup();
    mainButton.setup();

    pinMode(PINS_DC_ADAPTER_SWITCH[0], OUTPUT);
    digitalWrite(PINS_DC_ADAPTER_SWITCH[0], LOW);
    pinMode(PINS_PUMP_SWITCH[0], OUTPUT);
    digitalWrite(PINS_PUMP_SWITCH[0], LOW);

    analogReference(EXTERNAL);

    lcd.setCursor(0, 0);
    lcd.print(F("starting done"));
    lcd.flush();

    LOG_INFO(F("Setup done"));
}

void loop()
{
#ifdef PC_START
    pcStart.loop();
#endif

    // check if voltage is correct
    if (getControlSwitchVal(CS_DC_ADAPTER) && !IsDcVoltageCorrect())
        setControlSwitchVal(CS_DC_ADAPTER, false); // turn it off

    canCommands.loop();
    btCommandProcessor.loop();
    mainButton.loop();
    scenarioRunner.loop();

    if (mainButton.clicked())
    {
        if (scenarioRunner.currentState() == ScenarioRunnerState::Running)
        {
            scenarioRunner.stop();
        }
        else
        {
            scenarioRunner.start(&mainScenario);
        }
    }

    updateLcd();
}

template <typename Printer> inline void printMS(Printer &printer, uint16_t seconds)
{
    uint16_t tmp = seconds / 60;
    if (tmp < 10)
    {
        printer.print(" ");
    }
    printer.print(tmp);
    printer.print(":");

    tmp = seconds % 60;
    if (tmp < 10)
    {
        printer.print("0");
    }
    printer.print(tmp);
}

template <typename Printer> inline void printHMS(Printer &printer, uint16_t seconds)
{
    uint16_t tmp = seconds / 3600;
    if (tmp == 0)
    {
        printer.print("  ");
    }
    else
    {
        // hope 'hours' part is not more than 9, otherwise total len of string
        // would be more than expected
        printer.print(tmp);
        printer.print(":");
    }
    printMS(printer, seconds);
}

void updateLcd()
{
    if (scenarioRunner.currentState() == ScenarioRunnerState::Ready ||
        scenarioRunner.currentState() == ScenarioRunnerState::Finished)
    {
        lcd.setCursor(0, 0);
        lcd.print(F("Ready           ")); // update whole row
        lcd.setCursor(0, 1);
        if (scenarioRunner.currentState() == ScenarioRunnerState::Finished)
        {
            if (scenarioRunner.hasError())
            {
                lcd.print(F("finished w error"));
            }
            else
            {
                lcd.print(F("finished ok     "));
            }
        }
        else
        {
            lcd.print(F("press to start->"));
        }
        lcd.flush();
        return;
    }

    lcd.setCursor(0, 0);
    lcd.print(F("step "));
    lcd.print(scenarioRunner.currentStep() + 1);
    lcd.print("/");
    lcd.print(scenarioRunner.totalSteps());

    lcd.setCursor(11, 0);
    printMS(lcd, scenarioRunner.timeLeftForCurrentStep());

    lcd.setCursor(0, 1);
    lcd.print(scenarioRunner.hasError() ? "!" : " ");
    lcd.print(F(" left   "));
    printHMS(lcd, scenarioRunner.totalTimeLeft());
    lcd.flush();
}
