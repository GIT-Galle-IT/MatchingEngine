#include "../src/matching/MatchingEngine.hpp"
#include "Setup.hpp"

TEST_F(BaseTest, Matching_Algorithm_Test_Add_Mtaching_Sell_and_Buy_Order)
{
  long instrumentIndex(symbol_to_index_map["AAPL"]);
  MatchingEngine me{{instrumentIndex}};
                                // time_stamp,quantity,price,instrument,order_type
  auto order1 = orderbook::Order{400, 4, 8, 1, orderbook::Order::OrderType::SELL};
  auto order2 = orderbook::Order{400, 10, 10, 1, orderbook::Order::OrderType::BUY};
  me.matchOrder(order1);
  me.matchOrder(order2);

  auto sell_ordrs = me.getOrderBook(instrumentIndex)->getSellOrders();
  auto buy_ordrs = me.getOrderBook(instrumentIndex)->getBuyOrders();

  auto resulting_buy_ordr = orderbook::Order{400, 6, 10, 1, orderbook::Order::OrderType::BUY};
  EXPECT_TRUE(sell_ordrs.empty());
  EXPECT_TRUE(buy_ordrs.size() == 1);
  auto buy_ordr = buy_ordrs.top();
  EXPECT_EQ(buy_ordr, resulting_buy_ordr);
}

TEST_F(BaseTest, Matching_Algorithm_Test_Add_Mtaching_Buy_and_Sell_Order)
{
  long instrumentIndex(symbol_to_index_map["AAPL"]);
  MatchingEngine me{{instrumentIndex}};
                                // time_stamp,quantity,price,instrument,order_type
  auto order1 = orderbook::Order{400, 4, 8, 1, orderbook::Order::OrderType::SELL};
  auto order2 = orderbook::Order{400, 10, 10, 1, orderbook::Order::OrderType::BUY};
  me.matchOrder(order2);
  me.matchOrder(order1);

  auto sell_ordrs = me.getOrderBook(instrumentIndex)->getSellOrders();
  auto buy_ordrs = me.getOrderBook(instrumentIndex)->getBuyOrders();

  auto resulting_buy_ordr = orderbook::Order{400, 6, 10, 1, orderbook::Order::OrderType::BUY};
  EXPECT_TRUE(sell_ordrs.empty());
  EXPECT_TRUE(buy_ordrs.size() == 1);
  auto buy_ordr = buy_ordrs.top();
  EXPECT_EQ(buy_ordr, resulting_buy_ordr);
}

TEST_F(BaseTest, Matching_Algorithm_Test_Add_Mtaching_Buy_and_Multiple_Sell_Order)
{
  long instrumentIndex(symbol_to_index_map["AAPL"]);
  MatchingEngine me{{instrumentIndex}};
                                // time_stamp,quantity,price,instrument,order_type
  auto orders = {orderbook::Order{400, 4, 10, 1, orderbook::Order::OrderType::SELL},
                 orderbook::Order{401, 4, 9, 1, orderbook::Order::OrderType::SELL},
                 orderbook::Order{402, 4, 8, 1, orderbook::Order::OrderType::SELL},
                 orderbook::Order{403, 4, 8, 1, orderbook::Order::OrderType::SELL},
                 orderbook::Order{400, 17, 10, 1, orderbook::Order::OrderType::BUY}};
  for (auto& order : orders)
    me.matchOrder(const_cast<orderbook::Order&>(order));

  auto sell_ordrs = me.getOrderBook(instrumentIndex)->getSellOrders();
  auto buy_ordrs = me.getOrderBook(instrumentIndex)->getBuyOrders();

  auto resulting_buy_ordr = orderbook::Order{400, 1, 10, 1, orderbook::Order::OrderType::BUY};
  EXPECT_TRUE(sell_ordrs.empty());
  EXPECT_TRUE(buy_ordrs.size() == 1);
  auto buy_ordr = buy_ordrs.top();
  EXPECT_EQ(buy_ordr, resulting_buy_ordr);
}