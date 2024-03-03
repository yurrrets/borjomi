#include "gmock/gmock.h"
#include "gtest/gtest.h"

template <typename ContainerA, typename ContainerB>
bool IsSupersequenceOfImpl(const ContainerA &a, const ContainerB &b)
{
    auto ita = std::begin(a);
    auto itb = std::begin(b);
    while (true)
    {
        if (itb == std::end(b))
        {
            return true;
        }
        if (ita == std::end(a))
        {
            return false;
        }
        if (*ita == *itb)
        {
            ita++;
            itb++;
            continue;
        }
        ita++;
    }
}

MATCHER_P(IsSupersequenceOf, seq, "")
{
    return IsSupersequenceOfImpl(arg, seq);
}
