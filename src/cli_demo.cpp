#include <iostream>
#include "matching/MatchingEngine.hpp"
#include <chrono>

/*
Defining life of an order and trade

order -> matching engine -> *process -> order not matched -> put all/remaining on the order book
                            |                                           A
                            v                                           |
                            order matched -> partially fullfilled -------
                            |                       |
                            v                       |
                            fullfilled              | (async)
                            |                       |
                            v             (async)   v
                            trade done ---------> post trade activities (async)
                            |
                            v
                            update order book
*/
template<typename T>
void getInput(std::string prompt, T& output)
{
    std::cout << prompt << "\n";
    std::cin >> output;
};

void validateInput()
{

};

int main() {
    // start matching engine thread
    const std::unordered_map<std::string, int> markets = {{"AAPL", 1}, {"GOOG", 2}, {"NVDA", 3}}; // instruments
    orderbook::Order incoming_order;
    MatchingEngine me({1});
    while (true)
    {
        int order_type;
        getInput<int>("Enter price, quantity and order type for AAPL (index = 1) instrument\nAsk (1) or Bid (2) order ?", order_type);
        incoming_order.order_type = static_cast<orderbook::Order::OrderType>(order_type-1);
        getInput<int>("Price ?", incoming_order.price);
        getInput<long>("Quantity ?", incoming_order.quantity);
        const auto now     = std::chrono::system_clock::now();
        const auto epoch   = now.time_since_epoch();
        incoming_order.time_stamp = epoch.count();
        incoming_order.instrument = 1;
        
        me.matchOrder(incoming_order);

        std::vector<orderbook::Order> asks;
        std::vector<orderbook::Order> bids;
        me.getOrderBook(1)->getOrderBookCurrentSnapshot(bids, asks);

        std::cout << "\n++++++++++++++OrderBook Snapshot++++++++++++++\n";
        std::cout << "\n==============Asks==============\n";

        auto it = asks.rbegin();
        while (it != asks.rend())
        {
            it->toString();
            it++;
        }
        std::cout << "==============Bids==============\n";
        for (auto& bid : bids)
        {
            bid.toString();
        }
        std::cout << "==============================================\n\n";
    }

}