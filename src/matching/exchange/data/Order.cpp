#include "Order.h"
#include <sstream>

using namespace matching::exchange::data;

Order::~Order()
{
    delete next;
    delete previous;
}

void Order::setOrderId(const int& orderId)
{
    gOrderId = orderId;
}

void Order::setOrderSide(const std::string& side)
{
    if (side == "BUY")
        gOrderSide = OrderBookSide::Buy;
    else if (side == "SELL")
        gOrderSide = OrderBookSide::Sell;
    else
        gOrderSide = OrderBookSide::Invalid;
}

void Order::setOrderQuantity(const int& orderQuantity)
{
    gOrderQuantity = orderQuantity;
}

void Order::setOrderPrice(const double& orderPrice)
{
    gOrderPrice = orderPrice;
}

void Order::setInstrumentId(const int& instrumentId)
{
    gInstrumentId = instrumentId;
}

void Order::setUserId(const int& userId)
{
    gUserId = userId;
}

std::string Order::to_string() const
{
    std::stringstream ss;
    ss  << "[ORDER] "
        << "Order ID: "         << gOrderId
        << "Instrument ID: "    << gInstrumentId
        << "User ID: "          << gUserId
        << "Order Side: "       << (int)gOrderSide
        << "Order Quantity: "   << gOrderQuantity
        << "Order Price: "      << gOrderPrice;

    return ss.str();
}