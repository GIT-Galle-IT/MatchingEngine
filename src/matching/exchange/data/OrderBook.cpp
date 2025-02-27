#include "OrderBook.h"

using namespace matching::exchange::data;

void OrderBook::addOrder(int orderId, std::string OrderSide, double price, int qty)
{
    double remainingQuantity = match(orderId, OrderSide, price, qty);
    if (remainingQuantity <= 0) return;

    Order* order = memPool.allocate();
    if (!order) return;

    if (isPriceLevelExists(price))
    {
        Order* existingOrder = priceTimePriotityOrderBook[price];
        if (existingOrder) {
            order->previous = existingOrder->previous;
            order->next = nullptr;
            existingOrder->previous->next = order;
            existingOrder->previous = order;
        }
        return;
    }

    priceTimePriotityOrderBook[price] = order;
}

bool OrderBook::isPriceLevelExists(double price)
{
    return priceTimePriotityOrderBook.find(price) != priceTimePriotityOrderBook.end();
}

void OrderBook::cancelOrder(int orderId, std::string OrderSide, double price)
{
    if (priceTimePriotityOrderBook.find(price) == priceTimePriotityOrderBook.end())
        return;

    Order* next = priceTimePriotityOrderBook[price]->next;
    while (next && next->getOrderId() != orderId)
    {
        next = next->next;
    }

    if (next) {
        if (next->previous) next->previous->next = next->next;
        if (next->next) next->next->previous = next->previous;
        memPool.deallocate(next);
    }
}

void OrderBook::setPassiveOrderBook(OrderBook* orderBook)
{
    passiveOrderBook = orderBook;
}

std::string OrderBook::to_string() const
{
    return "OrderBook state"; 
}

double OrderBook::match(int orderId, std::string OrderSide, double price, int qty)
{
    if (passiveOrderBook && passiveOrderBook->isPriceLevelExists(price))
    {
        // Return remaining quantity
    }
    return qty;
}