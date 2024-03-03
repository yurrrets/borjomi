#include "ArduinoSims.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Arduino.h"

using namespace testing;

TEST(ArduinoSimulation, basic_test)
{
    int setup_cnt = 0, loop_cnt = 0;
    auto curr_ms = millis();
    ArduinoRunContext ctx;
    ctx.set_callbacks([&] { setup_cnt++; }, [&] { loop_cnt++; });
    ctx.run_for(20 * 1000);

    EXPECT_EQ(setup_cnt, 1);
    EXPECT_GE(loop_cnt, 20*1000);
    EXPECT_EQ(millis(), curr_ms + 20*1000);
}
