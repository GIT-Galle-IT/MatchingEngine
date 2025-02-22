#include "Setup.hpp"

TEST_F(BaseTest, OrderBook_Hash_Consistentency_Test)
{
  for (const auto &order : BaseTest::buy_orders)
  {
    auto hash = orderbook::OrderHash{}(orderbook::OrderKey(order));
    EXPECT_EQ(
        (std::hash<long>{}(order.instrument) ^ (std::hash<long>{}(order.quantity) << 1) ^ (std::hash<long>{}(static_cast<int>(order.order_type)) << 2) ^
         (std::hash<long>{}(order.time_stamp) << 4) ^ (std::hash<long>{}(order.price) << 6)),
        hash);
  }
}

TEST_F(BaseTest, OrderBook_Add_Buy_Orders)
{
  orderbook::OrderBook order_book;
  for (const auto order : BaseTest::buy_orders)
  {
    order_book.addOrder(order);
  }
  std::vector<orderbook::Order> priority_orderd_buy_orders = {
        orderbook::Order{500, 99, 388, 1, orderbook::Order::OrderType::BUY},
        orderbook::Order{1000, 900, 344, 1, orderbook::Order::OrderType::BUY},
        orderbook::Order{200, 15, 300, 1, orderbook::Order::OrderType::BUY},
        orderbook::Order{498, 45, 298, 1, orderbook::Order::OrderType::BUY},
        orderbook::Order{388, 3, 280, 1, orderbook::Order::OrderType::BUY},
        orderbook::Order{300, 16, 150, 1, orderbook::Order::OrderType::BUY},
        orderbook::Order{150, 8, 45, 1, orderbook::Order::OrderType::BUY},
        orderbook::Order{1, 10, 10, 1, orderbook::Order::OrderType::BUY}, // lowest time stamp (early to market)
        orderbook::Order{100, 10, 10, 1, orderbook::Order::OrderType::BUY},
        orderbook::Order{400, 4, 8, 1, orderbook::Order::OrderType::BUY}};

  auto buy_ordrs = order_book.getBuyOrders();
  int index = 0;
  while (buy_ordrs.empty() != true)
  {
    orderbook::Order buy_ordr = buy_ordrs.top();
    EXPECT_TRUE(buy_ordr == priority_orderd_buy_orders[index]);
    buy_ordrs.pop();
    index++;
  }
}

TEST_F(BaseTest, OrderBook_Add_Sell_Orders)
{
  orderbook::OrderBook order_book;
  for (const auto order : BaseTest::sell_orders)
  {
    order_book.addOrder(order);
  }
  std::vector<orderbook::Order> priority_orderd_sell_orders = {
        orderbook::Order{500, 99, 388, 1, orderbook::Order::OrderType::SELL},
        orderbook::Order{1000, 900, 344, 1, orderbook::Order::OrderType::SELL},
        orderbook::Order{200, 15, 300, 1, orderbook::Order::OrderType::SELL},
        orderbook::Order{498, 45, 298, 1, orderbook::Order::OrderType::SELL},
        orderbook::Order{388, 3, 280, 1, orderbook::Order::OrderType::SELL},
        orderbook::Order{300, 16, 150, 1, orderbook::Order::OrderType::SELL},
        orderbook::Order{150, 8, 45, 1, orderbook::Order::OrderType::SELL},
        orderbook::Order{100, 10, 10, 1, orderbook::Order::OrderType::SELL},
        orderbook::Order{99, 10, 10, 1, orderbook::Order::OrderType::SELL},
        orderbook::Order{400, 4, 8, 1, orderbook::Order::OrderType::SELL}};        

  auto sell_ordrs = order_book.getSellOrders();
  int index = priority_orderd_sell_orders.size() - 1; // reverse order, feeling lazy to reverse vector definition 🥱
  while (sell_ordrs.empty() != true)
  {
    orderbook::Order sell_ordr = sell_ordrs.top();
    EXPECT_TRUE(sell_ordr == priority_orderd_sell_orders[index]);
    sell_ordrs.pop();
    index--;
  }
}