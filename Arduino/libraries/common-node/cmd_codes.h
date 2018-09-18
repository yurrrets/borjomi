
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

/// When send to node 0x0, this means multicast message.
/// And every slave node in this case should answer with CMD_PONG.
/// Otherwise react only if node address matches, as always.
/// Value c
#define CMD_PING                 (2)

/// This is the answer to ping command (CMD_PING)
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

/// Reads the value of analog input pin
#define CMD_READ_ANALOG          (6)

/// Reads soil moisture value
/// CommandPin parameter is ignored, instead always is used PIN_SOIL_MOISTURE
#define CMD_READ_SOIL_MOISTURE   (7)
#define PIN_SOIL_MOISTURE        (0) // soil moisture sensor is on analog pin 0
