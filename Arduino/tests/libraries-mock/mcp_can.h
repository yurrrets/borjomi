#pragma once

#include "../../libraries/MCP_CAN/mcp_can_dfs.h"
#define MAX_CHAR_IN_MESSAGE 8

#include "gmock/gmock.h"

class MCP_CAN
{
  public:
    virtual INT8U begin(INT8U idmodeset, INT8U speedset, INT8U clockset) = 0;
    virtual INT8U init_Mask(INT8U num, INT8U ext, INT32U ulData) = 0;
    virtual INT8U init_Mask(INT8U num, INT32U ulData) = 0;
    virtual INT8U init_Filt(INT8U num, INT8U ext, INT32U ulData) = 0;
    virtual INT8U init_Filt(INT8U num, INT32U ulData) = 0;
    virtual INT8U setMode(INT8U opMode) = 0;
    virtual INT8U sendMsgBuf(INT32U id, INT8U ext, INT8U len, INT8U *buf) = 0;
    virtual INT8U sendMsgBuf(INT32U id, INT8U len, INT8U *buf) = 0;
    virtual INT8U readMsgBuf(INT32U *id, INT8U *ext, INT8U *len, INT8U *buf) = 0;
    virtual INT8U readMsgBuf(INT32U *id, INT8U *len, INT8U *buf) = 0;
    virtual INT8U checkReceive(void) = 0;
    virtual INT8U checkError(void) = 0;
    virtual INT8U getError(void) = 0;
    virtual INT8U errorCountRX(void) = 0;
    virtual INT8U errorCountTX(void) = 0;
    virtual INT8U enOneShotTX(void) = 0;
    virtual INT8U disOneShotTX(void) = 0;
    virtual INT8U abortTX(void) = 0;
    virtual INT8U setGPO(INT8U data) = 0;
    virtual INT8U getGPI(void) = 0;
};

class MCP_CAN_mock : public MCP_CAN
{
  public:
    MOCK_METHOD(INT8U, begin, (INT8U idmodeset, INT8U speedset, INT8U clockset));
    MOCK_METHOD(INT8U, init_Mask, (INT8U num, INT8U ext, INT32U ulData));
    MOCK_METHOD(INT8U, init_Mask, (INT8U num, INT32U ulData));
    MOCK_METHOD(INT8U, init_Filt, (INT8U num, INT8U ext, INT32U ulData));
    MOCK_METHOD(INT8U, init_Filt, (INT8U num, INT32U ulData));
    MOCK_METHOD(INT8U, setMode, (INT8U opMode));
    MOCK_METHOD(INT8U, sendMsgBuf, (INT32U id, INT8U ext, INT8U len, INT8U *buf));
    MOCK_METHOD(INT8U, sendMsgBuf, (INT32U id, INT8U len, INT8U *buf));
    MOCK_METHOD(INT8U, readMsgBuf, (INT32U * id, INT8U *ext, INT8U *len, INT8U *buf));
    MOCK_METHOD(INT8U, readMsgBuf, (INT32U * id, INT8U *len, INT8U *buf));
    MOCK_METHOD(INT8U, checkReceive, ());
    MOCK_METHOD(INT8U, checkError, ());
    MOCK_METHOD(INT8U, getError, ());
    MOCK_METHOD(INT8U, errorCountRX, ());
    MOCK_METHOD(INT8U, errorCountTX, ());
    MOCK_METHOD(INT8U, enOneShotTX, ());
    MOCK_METHOD(INT8U, disOneShotTX, ());
    MOCK_METHOD(INT8U, abortTX, ());
    MOCK_METHOD(INT8U, setGPO, (INT8U data));
    MOCK_METHOD(INT8U, getGPI, ());
};
