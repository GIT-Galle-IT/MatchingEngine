#pragma once

#include "matching/utils/defs.h"
#include <string> 

//TODO: try to remove string comparisons - move order side mapping to order gateway 
//TODO: need to add user id and instrument id filtering and usage


namespace matching::exchange::data
{

    class Order
    {
        public:
            Order() = default;
            Order(int orderId, int instrumentId, int userId, std::string side, int orderQuantity, double orderPrice): 
                gOrderId(orderId),
                gInstrumentId(instrumentId),
                gUserId(userId),
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

            ~Order();

            void setOrderId(const int& orderId);
            void setOrderSide(const std::string& side);
            void setOrderQuantity(const int& orderQuantity);
            void setOrderPrice(const double& orderPrice);
            void setInstrumentId(const int& instrumentId);
            void setUserId(const int& userId);

            int getOrderId() const {return gOrderId;}
            OrderBookSide getOrderSide()const {return gOrderSide;}
            int getOrderQuantity()const {return gOrderQuantity;}
            double getOrderPrice()const {return gOrderPrice;}
            double getInstrumetId()const {return gInstrumentId;}
            double getUserId()const {return gUserId;}

            std::string to_string() const;

            Order* next = nullptr;
            Order* previous = nullptr;

        private:
            int gOrderId;
            OrderBookSide gOrderSide;
            int gOrderQuantity;
            double gOrderPrice;
            int gUserId;
            int gInstrumentId;
    };

}