#pragma once
namespace gbase::net
{
    enum Event : int
    {
        NONE = 0,
        MESSAGE_BUFFERRED = 1
    };

    enum GEventHandlingMode : int
    {
        SYNC,
        ASYNC
    };

    enum YesNo : int
    {
        NO = 0,
        YES = 1
    };
} // namespace gbase::net
