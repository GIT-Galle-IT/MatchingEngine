#include "OrderBook.h"

//TODO: need to check the matching logic and possible enhancement, have a chat
//TODO: need to add user id and instrument id filtering and usage

using namespace matching::exchange::data;

void OrderBook::addOrder(int orderId, int instrumentId, int userId, std::string orderSide, double price, int qty)
{
    double remainingQuantity = match(orderId, orderSide, price, qty);
    if (remainingQuantity <= 0)
        return;

    Order* order = memPool.allocate(orderId, instrumentId, userId, orderSide, price, qty);
    if (!order)
        return;

    if (isPriceLevelExists(price))
    {
        Order* existingOrder = priceTimePriotityOrderBook[price];
        if (existingOrder) 
        {
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

Order* OrderBook::getCurrentMaxPriority(double price)
{
    if (isPriceLevelExists(price))
        return priceTimePriotityOrderBook[price];
    else
        return nullptr;
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
        if (next->previous) 
            next->previous->next = next->next;
        if (next->next) 
            next->next->previous = next->previous;
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

/*

======================== Order Matching Logic ================================

    Step 1: Match Orders
        - If a new buy order comes in:
            - Compare it with the lowest-priced sell order.
            - If the buy price ≥ sell price → trade occurs.
            - Reduce the order quantity or remove the order if fully matched.
        - If a new sell order comes in:
            - Compare it with the highest-priced buy order.
            - If the sell price ≤ buy price → trade occurs.
    
    Step 2: Partial Execution Handling
        - If an order cannot be fully executed:
        - The matched portion executes.
        - The remaining quantity stays in the order book.

    Step 3: Order Book Updates
        - Remove fully executed orders.
        - Keep partially matched orders with updated quantities.

*/

double OrderBook::match(int orderId, std::string OrderSide, double price, int qty)
{
    if (passiveOrderBook && passiveOrderBook->isPriceLevelExists(price))
    {
        Order* currPriorityHighest = passiveOrderBook->getCurrentMaxPriority(price);
        if (currPriorityHighest == nullptr)
            return qty;

        double remainingQty = qty;
        if (OrderSide == "Buy")
        {
            MatchBuyOrder(price, remainingQty);
        }
        else if (OrderSide == "Sell")
        {
            MatchSellOrder(price, remainingQty);
        }
        else
        {
            //invalid
            return 0.0;
        }
    }
    return qty;
}

double OrderBook::MatchBuyOrder(double price, double remainingQty)
{
    Order* currPriorityHighest = passiveOrderBook->getCurrentMaxPriority(price);
    if (price >= currPriorityHighest->getOrderPrice())
    {
        if (remainingQty < currPriorityHighest->getOrderQuantity())
        {
            currPriorityHighest->setOrderQuantity(currPriorityHighest->getOrderQuantity()-remainingQty);
            return 0.0;
        }
        else if (remainingQty == currPriorityHighest->getOrderQuantity())
        {
            //change priotiy order here
            passiveOrderBook->setCurrentMaxPriority(price, currPriorityHighest);
            return 0.0;
        }
        else
        {
            //recurive call implementation
            if (passiveOrderBook->setCurrentMaxPriority(price, currPriorityHighest))
                MatchBuyOrder(price, remainingQty);
            return remainingQty;
        }
    }
    return remainingQty;
}

double OrderBook::MatchSellOrder(double price, double remainingQty)
{
    Order* currPriorityHighest = passiveOrderBook->getCurrentMaxPriority(price);
    if (price <= currPriorityHighest->getOrderPrice())
    {
        if (remainingQty < currPriorityHighest->getOrderQuantity())
        {
            currPriorityHighest->setOrderQuantity(currPriorityHighest->getOrderQuantity()-remainingQty);
            return 0.0;
        }
        else if (remainingQty == currPriorityHighest->getOrderQuantity())
        {
            //change priotiy order here
            passiveOrderBook->setCurrentMaxPriority(price, currPriorityHighest);
            return 0.0;
        }
        else
        {
            //recurive call implementation
            if (passiveOrderBook->setCurrentMaxPriority(price, currPriorityHighest))
                MatchSellOrder(price, remainingQty);
            return remainingQty;
        }
    }
    return remainingQty;
}

bool OrderBook::setCurrentMaxPriority(double price, Order* priorityOrder)
{
    if (priorityOrder->next == nullptr)
        return false;
    priceTimePriotityOrderBook[price] = priorityOrder->next;
    priorityOrder->previous = nullptr;
    delete priorityOrder;
    return true;
}
