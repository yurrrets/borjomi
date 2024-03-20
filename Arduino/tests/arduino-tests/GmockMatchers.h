#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "log.h"

template <typename ContainerA, typename ContainerB, typename Listener>
bool IsSupersequenceOfImpl(const ContainerA &superseq, const ContainerB &subseq,
                           Listener *result_listener)
{
    auto it_super = std::begin(superseq);
    auto it_sub = std::begin(subseq);
    while (true)
    {
        if (it_sub == std::end(subseq))
        {
            return true;
        }
        if (it_super == std::end(superseq))
        {
            if (result_listener)
            {
                *result_listener << "item index " << std::distance(std::begin(subseq), it_sub)
                                 << " with value '" << *it_sub << "' didn't match";
            }
            return false;
        }
        if (*it_super == *it_sub)
        {
            it_super++;
            it_sub++;
            continue;
        }
        it_super++;
    }
}

MATCHER_P(IsSupersequenceOf, seq, "")
{
    return IsSupersequenceOfImpl(arg, seq, result_listener);
}
