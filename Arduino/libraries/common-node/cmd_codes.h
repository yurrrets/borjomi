
// Message format (8-byte message)
//   ByteNo     Description
//   0          CRC8 of next 7 bytes
//   1          Command No
//   2          Command code CMD_XXX
//   3          Command pin (if any)
//   4-7        Value


#define CMD_INVALID              (0)

/// This is the answer for most commands (except PING).
/// Keep CommandNo and CommandPin.
/// Value in answer depends on command type.
/// For CMD_SET_XXX is usually CVAL_ACCEPTED
#define CMD_OK                   (1)
#define CVAL_ACCEPTED            (0)

/// When send to node 0x0 (MULTICAST_NODE), this means multicast message.
/// And every slave node in this case should answer with CMD_PONG.
/// Otherwise react only if node address matches, as always.
#define CMD_PING                 (2)

/// This is the answer to ping command (CMD_PING)
/// Value of command answer is slave node's address.
#define CMD_PONG                 (3)

/// Sets state of water switch.
/// Value is CVAL_CLOSED or CVAL_OPENED
/// CommandPin parameter is ignored, instead always is used PIN_WATER_SWITCH
#define CMD_SET_WATER_SWITCH     (4)
#define PIN_WATER_SWITCH         (3) // water switch is on digital pin 4
#define CVAL_CLOSED              (0)
#define CVAL_OPENED              (1)

/// Gets current state of water switch.
/// Value is CVAL_CLOSED or CVAL_OPENED
/// CommandPin parameter is ignored, instead always is used PIN_WATER_SWITCH
#define CMD_GET_WATER_SWITCH     (5)
//#define CVAL_CLOSING             (3)
//#define CVAL_OPENING             (4)

/// Reads soil moisture value.
/// The same as CMD_ANALOG_READ command, but
/// commandPin parameter is ignored, instead always is used PIN_SOIL_MOISTURE
#define CMD_READ_SOIL_MOISTURE   (6)
#define PIN_SOIL_MOISTURE        (0) // soil moisture sensor is on analog pin 0

/// Returns current version of firmware.
/// Answer is CMD_OK, and value is in format <major>.<minor>.
/// High 16 bits of value is <major>, and low 16 bits are for <minor>.
#define CMD_VERSION              (7)
#define MAKE_VERSION(mj,mn)      ((((uint32_t)mj & 0xFFFF) << 16) | ((uint32_t)mn & 0xFFFF))

/// Calls analogWrite Arduino function. Uses commandPin part of message.
/// Answers with CMD_OK, value equals to value parameter of input messge.
#define CMD_ANALOG_WRITE         (32)

/// Reads the value of analog input pin. Uses commandPin part of message.
/// Just calls analogRead Arduino function, and return the value.
#define CMD_ANALOG_READ          (33)



/// Next functions should be used carefully.
/// Remember that function do not perform any checks.
/// However, pins 0,1 are usually used as RX,TX pin on built-in COM port.
/// Pins 2,9,11,12,13 are used for SPI bus for CAN module.
/// Pin 3 is used for Water Switch.
/// Avoid using this pins directly with next functions.


/// Calls digitalWrite Arduino function. Uses commandPin part of message.
/// Answers with CMD_OK, value equals to value parameter of input messge.
#define CMD_DIGITAL_WRITE        (34)

/// Calls digitalRead Arduino function. Uses commandPin part of message.
/// Answers with CMD_OK, value equals to result of digitalRead call.
#define CMD_DIGITAL_READ         (35)

/// Calls pinMode Arduino function. Uses commandPin part of message.
/// Answers with CMD_OK, value equals to CVAL_ACCEPTED.
#define CMD_DIGITAL_PIN_MODE     (36)
