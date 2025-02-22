#include "Order.h"
#include <sstream>

using namespace matching::exchange;

void order::setOrderId(const int& orderId)
{
    gOrderId = orderId;
}

void order::setOrderSide(const std::string& side)
{
    if (side == "BUY")
        gSide = OrderBookSide::Buy;
    else if (side == "SELL")
        gSide = OrderBookSide::Sell;
    else
        gSide = OrderBookSide::Invalid;
}

void order::setOrderQuantity(const int& orderQuantity)
{
    gOrderQuantity = orderQuantity;
}

void order::setOrderPrice(const double& orderPrice)
{
    gOrderPrice = orderPrice;
}

std::string Order::to_string() const
{
    std::stringstream ss;
    ss  << "[ORDER] "
        << "Order ID: "         << gOrderId
        << "Order Side: "       << (int)gOrderSide
        << "Order Quantity: "   << gOrderQuantity
        << "Order Price: "      << gOrderPrice;

    return ss.str();
}