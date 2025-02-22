#include "MatchingEngine.hpp"

MatchingEngine::MatchingEngine(std::vector<InstrumentIndex> instrument_indexes)
{
    for (auto& instrument_index : instrument_indexes)
    {
        instrument_to_order_books_map[instrument_index] = 
            new orderbook::OrderBook();
    }
}

void MatchingEngine::matchOrder(orderbook::Order &order)
{
    auto it = instrument_to_order_books_map.find(order.instrument);
    if (it == instrument_to_order_books_map.end())
    {
        // error return
    }

    const auto orderbook = it->second;
    if (orderbook == nullptr)
    {
        // error return
    }

    if (orderbook::Order::OrderType::SELL == order.order_type)
    {
        orderbook::Order outstanding_buy_order{};
        while (orderbook->getBuyOrders().empty() != true)
        {
            outstanding_buy_order = orderbook->getBuyOrders().top();
            orderbook->getBuyOrders().pop();
            if (outstanding_buy_order.price >= order.price)
            {
                if (outstanding_buy_order.quantity >= order.quantity)
                {
                    outstanding_buy_order.quantity -= order.quantity;
                    if (outstanding_buy_order.quantity > 0)
                    {
                        orderbook->addOrder(outstanding_buy_order);
                    }
                    return;
                }
                order.quantity -= outstanding_buy_order.quantity;
                outstanding_buy_order.quantity = 0;
            }
            else
            {
                orderbook->addOrder(outstanding_buy_order);
                orderbook->addOrder(order);
                return;
            }
        }
        if (orderbook->getBuyOrders().empty() == true)
        {
            orderbook->addOrder(order);
        }
    }
    else if (orderbook::Order::OrderType::BUY == order.order_type)
    {
        orderbook::Order outstanding_sell_order{};
        while (orderbook->getSellOrders().empty() != true)
        {
            outstanding_sell_order = orderbook->getSellOrders().top();
            orderbook->getSellOrders().pop();
            if (outstanding_sell_order.price <= order.price)
            {
                if (outstanding_sell_order.quantity >= order.quantity)
                {
                    outstanding_sell_order.quantity -= order.quantity;
                    if (outstanding_sell_order.quantity > 0)
                    {
                        orderbook->addOrder(outstanding_sell_order);
                    }
                    return;
                }
                order.quantity -= outstanding_sell_order.quantity;
                outstanding_sell_order.quantity = 0;
            }
            else
            {
                orderbook->addOrder(outstanding_sell_order);
                orderbook->addOrder(order);
                return;
            }
        }

        if (orderbook->getSellOrders().empty() == true)
        {
            orderbook->addOrder(order);
        }
    }

}

orderbook::OrderBook* MatchingEngine::getOrderBook(const InstrumentIndex& instrumentIndex) const
{
    auto it = instrument_to_order_books_map.find(instrumentIndex);
    if (it == instrument_to_order_books_map.end())
    {
        return nullptr;
    }
    return it->second;
}
