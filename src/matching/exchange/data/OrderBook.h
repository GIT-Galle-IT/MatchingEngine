#pragma once
#include "Order.h"

class OrderBook final
{
    public:
        OrderBook();
        virtual ~OrderBook() = default;
        
        void addOrder() const;
        void cancelOrder() const;

        std::string to_string() const;

    private:
        void match(); 
};