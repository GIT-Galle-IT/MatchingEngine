#include "OrderBook.hpp"

using namespace orderbook;


void OrderBook::removeOrder(const Order &order_to_remove)
{
    // order removal - 
        // will deactivate order. EOD will clean up the orders from the list
}

// should we use polymorphism? (for buy and sell order additions)
void OrderBook::addOrder(const orderbook::Order &order_to_add)
{
    const OrderKey orderKey(order_to_add);
    m_all_orders.emplace(std::make_pair(orderKey, order_to_add));

    if (Order::OrderType::BUY == order_to_add.order_type)
    {
        m_buy_orders_priority_q.emplace(order_to_add);
    }
    else if (Order::OrderType::SELL == order_to_add.order_type)
    {
        m_sell_orders_priority_q.emplace(order_to_add);
    }
    else
    {
        // unhandled order type
    }
}

OrderBook::SellOrderPriorityQueue& OrderBook::getSellOrders()
{
    return m_sell_orders_priority_q;
}

OrderBook::BuyOrderPriorityQueue& OrderBook::getBuyOrders()
{
    return m_buy_orders_priority_q;
}

void orderbook::OrderBook::getOrderBookCurrentSnapshot(std::vector<Order> &asks, std::vector<Order> &bids) const
{
    auto sell_ordrs = getSellOrders();
    auto buy_ordrs = getBuyOrders();
    while (sell_ordrs.empty() != true)
    {
        bids.push_back(sell_ordrs.top());
        sell_ordrs.pop();
    }
    while (buy_ordrs.empty() != true)
    {
        asks.push_back(buy_ordrs.top());
        buy_ordrs.pop();
    }
}

OrderBook::SellOrderPriorityQueue orderbook::OrderBook::getSellOrders() const
{
    return m_sell_orders_priority_q;
}

OrderBook::BuyOrderPriorityQueue orderbook::OrderBook::getBuyOrders() const
{
    return m_buy_orders_priority_q;
}
