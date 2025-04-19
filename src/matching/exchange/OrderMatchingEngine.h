#include <OrderBook.h>
#include <map>
#include <memory>

using namespace matching::exchange::data;
class OrderMatchingEngine
{
private:
    std::map<int, std::shared_ptr<OrderBook>> marketVsOrderBookMap;
public:
    OrderMatchingEngine() = default;

    void init()
    {
        // load instruments
        std::vector<std::pair<int, std::string>> instruments{{1, "AAPL"}, {2, "GOOG"}, {3, "MSFT"}};

        for (auto& idVsSymbol : instruments)
            auto orderBook = std::make_shared<OrderBook>();
            orderBook->
            marketVsOrderBookMap[idVsSymbol.first] = std::
    };
};

OrderMatchingEngine::OrderMatchingEngine()
{
}

OrderMatchingEngine::~OrderMatchingEngine()
{
}
