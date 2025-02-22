#pragma once

#include "defs.h"

namespace matching::exchange
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
                    gSide = OrderBookSide::Buy;
                else if (side == "SELL")
                    gSide = OrderBookSide::Sell;
                else
                    gSide = OrderBookSide::Invalid;
            }
            virtual ~Order() = default;

            void setOrderId(const int& orderId);
            void setOrderSide(const std::string& side);
            void setOrderQuantity(const int& orderQuantity);
            void setOrderPrice(const double& orderPrice);

            int getOrderId() const {return gOrderId;}
            OrderBookSide getOrderSide()const {return gSide;}
            int getOrderQuantity()const {return gOrderQuantity;}
            double getOrderPrice()const {return gOrderPrice;}

            std::string to_string() const;

        private:
            int gOrderId;
            OrderBookSide gSide;
            int gOrderQuantity;
            double gOrderPrice;
    };

}