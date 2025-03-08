#pragma once
#include "Order.h"
#include <gbase/gMemoryPool.h>
#include <unordered_map>
#include <iostream>

// TODO: try implementing buy sell order book differentiation

namespace matching::exchange::data
{
    class OrderBook final
    {
        public:
            OrderBook() : passiveOrderBook(nullptr), memPool(1000) {}
            virtual ~OrderBook() = default;

            void init();
            void addOrder(int orderId, int instrumentId, int userId, std::string orderSide, double price, int qty);
            void cancelOrder(int orderId, std::string OrderSide, double price);

            std::unordered_map<double, Order*> getMap() { return priceTimePriotityOrderBook; }
            bool isPriceLevelExists(double price);
            Order* getCurrentMaxPriority(double price);
            bool setCurrentMaxPriority(double price, Order* priorityOrder);

            void setPassiveOrderBook(OrderBook* orderBook);

            std::string to_string() const;

        private:
            double match(int orderId, std::string OrderSide, double price, int qty); 
            double MatchSellOrder(double price, double remainingQty);
            double MatchBuyOrder(double price, double remainingQty);

            std::unordered_map<double, Order*> priceTimePriotityOrderBook;
            gbase::MemoryPool<Order> memPool;
            OrderBook* passiveOrderBook;
    };
}
