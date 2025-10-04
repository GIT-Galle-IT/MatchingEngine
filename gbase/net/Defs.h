#pragma once
namespace GNet
{
    enum Event : int
    {
        NONE = 0,
        MESSAGE_BUFFERRED = 1
    };

    enum GServerMode : int
    {
        SYNC,
        ASYNC
    };

    enum YesNo : int
    {
        NO = 0,
        YES = 1
    };
} // namespace GNet
