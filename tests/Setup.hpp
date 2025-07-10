#include <gtest/gtest.h>

class BaseTest : public testing::Test
{
protected:
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
    
  }
};