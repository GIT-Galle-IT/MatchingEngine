#include <gtest/gtest.h>
#include "../src/orderbook/OrderBook.hpp"

class BaseTest : public testing::Test
{
protected:
  std::map<std::string, int> symbol_to_index_map;
  std::vector<orderbook::Order> sell_orders;
  std::vector<orderbook::Order> buy_orders;
  /*
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
    };
  */
  void SetUp()
  {
    symbol_to_index_map = {{"AAPL", 1}, {"GOOG", 2}, {"NVDA", 3}};
    sell_orders = {
                      // time_stamp,quantity,price,instrument,order_type
        orderbook::Order{100, 10, 10, 1, orderbook::Order::OrderType::SELL},
        orderbook::Order{200, 15, 300, 1, orderbook::Order::OrderType::SELL},
        orderbook::Order{300, 16, 150, 1, orderbook::Order::OrderType::SELL},
        orderbook::Order{400, 4, 8, 1, orderbook::Order::OrderType::SELL},
        orderbook::Order{150, 8, 45, 1, orderbook::Order::OrderType::SELL},
        orderbook::Order{388, 3, 280, 1, orderbook::Order::OrderType::SELL},
        orderbook::Order{498, 45, 298, 1, orderbook::Order::OrderType::SELL},
        orderbook::Order{1000, 900, 344, 1, orderbook::Order::OrderType::SELL},
        orderbook::Order{99, 10, 10, 1, orderbook::Order::OrderType::SELL},
        orderbook::Order{500, 99, 388, 1, orderbook::Order::OrderType::SELL}};
    buy_orders = {
        orderbook::Order{100, 10, 10, 1, orderbook::Order::OrderType::BUY},
        orderbook::Order{200, 15, 300, 1, orderbook::Order::OrderType::BUY},
        orderbook::Order{300, 16, 150, 1, orderbook::Order::OrderType::BUY},
        orderbook::Order{400, 4, 8, 1, orderbook::Order::OrderType::BUY},
        orderbook::Order{1, 10, 10, 1, orderbook::Order::OrderType::BUY},
        orderbook::Order{150, 8, 45, 1, orderbook::Order::OrderType::BUY},
        orderbook::Order{388, 3, 280, 1, orderbook::Order::OrderType::BUY},
        orderbook::Order{498, 45, 298, 1, orderbook::Order::OrderType::BUY},
        orderbook::Order{1000, 900, 344, 1, orderbook::Order::OrderType::BUY},
        orderbook::Order{500, 99, 388, 1, orderbook::Order::OrderType::BUY}};
  }
};