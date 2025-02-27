#pragma once

#include "matching/utils/defs.h"
#include <string> 

namespace matching::exchange::data
{

    class Order
    {
        public:
            Order() = default;
            Order(int orderId, std::string side, int orderQuantity, double orderPrice): 
                gOrderId(orderId),
                gOrderQuantity(orderQuantity),
                gOrderPrice(orderPrice)
            {
                if (side == "BUY")
                    gOrderSide = OrderBookSide::Buy;
                else if (side == "SELL")
                    gOrderSide = OrderBookSide::Sell;
                else
                    gOrderSide = OrderBookSide::Invalid;
            }
            virtual ~Order() = default;

            void setOrderId(const int& orderId);
            void setOrderSide(const std::string& side);
            void setOrderQuantity(const int& orderQuantity);
            void setOrderPrice(const double& orderPrice);

            int getOrderId() const {return gOrderId;}
            OrderBookSide getOrderSide()const {return gOrderSide;}
            int getOrderQuantity()const {return gOrderQuantity;}
            double getOrderPrice()const {return gOrderPrice;}

            std::string to_string() const;

            Order* next = nullptr;
            Order* previous = nullptr;

        private:
            int gOrderId;
            OrderBookSide gOrderSide;
            int gOrderQuantity;
            double gOrderPrice;
    };

}