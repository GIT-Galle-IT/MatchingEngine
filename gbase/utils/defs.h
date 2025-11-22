#pragma once


constexpr int MAX_MESSAGE_SIZE = 1 << 13; // 8192 bytes, 8KB
enum OrderBookSide 
{
    Invalid = 0,
    Buy     = 1,
    Sell    = 2
};