#pragma once
#include "Order.h"
#include "matching/gbase/gMemoryPool.h"
#include <unordered_map>
#include <iostream>

namespace matching::exchange::data
{
    class OrderBook final
    {
        public:
            OrderBook() : passiveOrderBook(nullptr), memPool(1000) {}
            virtual ~OrderBook() = default;

            void init();
            void addOrder(int orderId, std::string OrderSide, double price, int qty);
            void cancelOrder(int orderId, std::string OrderSide, double price);

            std::unordered_map<double, Order*> getMap() { return priceTimePriotityOrderBook; }
            bool isPriceLevelExists(double price);

            void setPassiveOrderBook(OrderBook* orderBook);

            std::string to_string() const;

        private:
            double match(int orderId, std::string OrderSide, double price, int qty); 

            std::unordered_map<double, Order*> priceTimePriotityOrderBook;
            gbase::MemoryPool<Order> memPool;
            OrderBook* passiveOrderBook;
    };
}
