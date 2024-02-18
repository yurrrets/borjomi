// CAN Send Example
//

#include <SoftwareSerial.h>

#include "config.h"
#include "stream_ext.h"
#include "cmd_codes.h"
#include "can_message.h"

#include "button.h"
#include "bt_commands.h"
#include "can_commands.h"
#include "nodes.h"
#include "pc_start.h"
#include "common.h"
#include "cap_implementation.h"
#include "scenarios.h"

#include "LiquidCrystalDisplayEx.h"
#include "LiquidCrystalDisplayEx.ipp"

#include <EEPROM.h>

#define BT_RX_PIN   (4)
#define BT_TX_PIN   (10)
#define CAN_CS_PIN  (9)
#define BTN_MAIN_PIN (12) // TODO: define true pin

#define LCD_CS_PIN    (8) // TODO: define true pin
#define LCD_CLOCK_PIN (12)
#define LCD_MOSI_PIN  (11)

#ifdef PC_START
#define PC_START_PIN (8)   // digital pin
#endif

#ifdef DEBUG
Stream &dbgSerial = Serial;
#endif

//SoftwareSerial btSerial(BT_RX_PIN, BT_TX_PIN, 0);
//auto &ioSerial = btSerial;
auto &ioSerial = Serial;

StreamExt ioExtSerial(ioSerial);
CanCommands canCommands(CAN_CS_PIN);
BTCommandParser ioCommandParser(ioExtSerial);
BTCommandProcessor btCommandProcessor(ioCommandParser, canCommands);
Button mainButton(BTN_MAIN_PIN);
ScenarioRunner scenarioRunner(canCommands);

LiquidCrystalShiftRegIface lcdIface(LCD_CS_PIN, LCD_CLOCK_PIN, LCD_MOSI_PIN);
LiquidCrystalDisplay lcd(lcdIface);

MasterNodeID NodeConfig;
Scenario mainScenario;
#ifdef PC_START
PcStart pcStart(PC_START_PIN);
#endif


void updateLcd();

void setup()
{
#ifdef DEBUG
    Serial.begin(9600);
#endif

    // set the data rate for the SoftwareSerial port
    ioSerial.begin(9600);

    // wait for serial port to connect. Needed for native USB
    while (!Serial) {
        ;
    }
//    ioSerial.println("Hello, World!");

#ifdef PC_START
    pcStart.setup();
#endif

    EEPROM.get(0, NodeConfig);
    if (NodeConfig.checkCrc() && NodeConfig.nodeId)
    {
#ifdef DEBUG
        dbgSerial << "I'm the Master Node ID " << NodeConfig.nodeId << endl;
#endif
    }
    else
    {
#ifdef DEBUG
        dbgSerial.println("I'm the Master Node with UNDEFINED ID!");
#endif
        NodeConfig.nodeId = UNKNOWN_NODE;
    }

    lcd.begin(16, 2);
    canCommands.setup();
    btCommandProcessor.setup();
    mainButton.setup();

    pinMode(PINS_DC_ADAPTER_SWITCH[0], OUTPUT);
    digitalWrite(PINS_DC_ADAPTER_SWITCH[0], LOW);
    pinMode(PINS_PUMP_SWITCH[0], OUTPUT);
    digitalWrite(PINS_PUMP_SWITCH[0], LOW);

    analogReference(EXTERNAL);
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

    if (mainButton.clicked())
    {
        if (scenarioRunner.isRunning())
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

template <typename Printer>
inline void printMS(Printer &printer, uint16_t seconds)
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
    }
    printMS(printer, seconds);
}

void updateLcd()
{
    if (!scenarioRunner.isRunning())
    {
        lcd.setCursor(0, 0);
        lcd.print("Ready           "); // update whole row
        lcd.setCursor(0, 1);
        lcd.print("press btn to start");
        lcd.autoscroll();
        return;
    }

    lcd.noAutoscroll();

    lcd.setCursor(0, 0);
    lcd.print("step ");
    lcd.print(scenarioRunner.currentStep() + 1);
    lcd.print("/");
    lcd.print(scenarioRunner.totalSteps());

    lcd.setCursor(11, 0);
    printMS(lcd, scenarioRunner.timeLeftForCurrentStep());

    lcd.setCursor(0, 1);
    lcd.print("left    ");
    printHMS(lcd, scenarioRunner.totalTimeLeft());
}
