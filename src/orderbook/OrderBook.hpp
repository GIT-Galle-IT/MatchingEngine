#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <unordered_map>
#include <memory>
#include <chrono>

namespace orderbook
{
    struct Order
    {
        enum class OrderType : int
        {
            SELL = 0,
            BUY = 1
        };

        long time_stamp;
        long quantity;
        int price;
        long instrument; // read size from configs
        OrderType order_type;

        bool operator==(const Order& other) const
        {
            return (this->time_stamp == other.time_stamp &&
                    this->quantity == other.quantity &&
                    this->price == other.price &&
                    this->instrument == other.instrument &&
                    this->order_type == other.order_type);
        }

        void toString() {
            std::cout << "time_stamp=" << time_stamp << " quantity=" << quantity << " price=" << price << " instrument_index=" << instrument 
                << " order_type=" << (order_type == OrderType::SELL ? "SELL" : "BUY") << std::endl;
        }
    };

    struct OrderKey
    {
        long time_stamp;
        long quantity;
        int price;
        long instrument; // read size from configs
        Order::OrderType order_type;

        OrderKey(const Order &order)
        {
            time_stamp = order.time_stamp;
            quantity = order.quantity;
            price = order.price;
            instrument = order.instrument;
            order_type = order.order_type;
        }

        bool operator==(const OrderKey &other) const noexcept
        {
            return (this->time_stamp == other.time_stamp &&
                    this->quantity == other.quantity &&
                    this->price == other.price &&
                    this->instrument == other.instrument &&
                    this->order_type == other.order_type);
        }
    };

    // think abouth the hash function
    struct OrderHash
    {
        std::size_t operator()(const OrderKey &orderKey) const noexcept
        {
            std::size_t h1 = std::hash<long>{}(orderKey.instrument);
            std::size_t h2 = std::hash<long>{}(orderKey.quantity);
            std::size_t h3 = std::hash<int>{}(static_cast<int>(orderKey.order_type));
            std::size_t h4 = std::hash<long>{}(orderKey.time_stamp);
            std::size_t h5 = std::hash<long>{}(orderKey.price);
            return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 4) ^ (h5 << 6); // or use boost::hash_combine
        }
    };

    template <class T>
    class StateLessOrderPriorityComparator
    {
        public:
        bool operator()(const Order &leftOrder, const Order &rightOrder) const noexcept
        {
            return static_cast<const T&>(*this).compare(leftOrder, rightOrder);
        }

        bool compare(const Order &leftOrder, const Order &rightOrder) { return false; }
    };

    class SellOrderPriorityCompartor : public StateLessOrderPriorityComparator<SellOrderPriorityCompartor>
    {
        // lowest price at top, price is equal then highest qunatity is at top, if both equal oldest order at the top
        public:
        bool compare(const Order &leftOrder, const Order &rightOrder) const noexcept
        {
            if (leftOrder.price > rightOrder.price) return true;
            else if (leftOrder.price < rightOrder.price) return false;
            else if (leftOrder.quantity > rightOrder.quantity) return true; // price is equal
            else if (leftOrder.quantity < rightOrder.quantity) return false;
            else if (rightOrder.time_stamp < leftOrder.time_stamp) return true; // price and quantity equal
            return false; // price , quantity and timestamp is equal (what to do if time stamp is also equal ?)
        }
    };

    class BuyOrderPriorityCompartor : public StateLessOrderPriorityComparator<BuyOrderPriorityCompartor>
    {
        // highest price at top, price is equal then lowest qunatity is at top, if both equal oldest order at the top
        public:
        bool compare(const Order &leftOrder, const Order &rightOrder) const noexcept
        {
            if (leftOrder.price < rightOrder.price) return true;
            else if (leftOrder.price > rightOrder.price) return false;
            else if (leftOrder.quantity < rightOrder.quantity) return true; // price is equal
            else if (leftOrder.quantity > rightOrder.quantity) return false;
            else if (rightOrder.time_stamp < leftOrder.time_stamp) return true; // price and quantity equal
            return false; // price , quantity and timestamp is equal (what to do if time stamp is also equal ?)
        }
    };

    class OrderBook
    {
        typedef std::priority_queue<Order, std::vector<Order>, SellOrderPriorityCompartor> SellOrderPriorityQueue;
        typedef std::priority_queue<Order, std::vector<Order>, BuyOrderPriorityCompartor> BuyOrderPriorityQueue;
    public:
        OrderBook(){};

        OrderBook(const OrderBook &other) = delete;       // no copying
        OrderBook &operator=(const OrderBook &) = delete; // no copy assignment

        OrderBook(OrderBook &&other) = delete;            // no moving
        OrderBook &operator=(OrderBook &&other) = delete; // no move assignment

        void addOrder(const orderbook::Order &order_to_add);
        void removeOrder(const Order &order_to_remove);

        SellOrderPriorityQueue& getSellOrders();
        BuyOrderPriorityQueue& getBuyOrders();

        void getOrderBookCurrentSnapshot(std::vector<Order> &buy_ords, std::vector<Order> &sell_ords) const;
    
    private:
        SellOrderPriorityQueue getSellOrders() const;
        BuyOrderPriorityQueue getBuyOrders() const;

        std::unordered_map<orderbook::OrderKey, Order, OrderHash> m_all_orders; // let's think about hashing
        SellOrderPriorityQueue m_sell_orders_priority_q;
        BuyOrderPriorityQueue m_buy_orders_priority_q;

    };

} // namespace orderbook

// wisdom - The difference between move and copy is that after a copy two objects must have the same value, whereas after a move the source of the move is not required to have its original value.
#endif