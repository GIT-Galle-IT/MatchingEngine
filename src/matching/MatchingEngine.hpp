#include <unordered_map>
#include "../orderbook/OrderBook.hpp"

class MatchingEngine
{
    typedef long InstrumentIndex;
private:
    std::unordered_map<InstrumentIndex, orderbook::OrderBook*> instrument_to_order_books_map;
public:
    MatchingEngine(std::vector<InstrumentIndex> instrument_indexes);

    // match order
    // try matchOrder
    void matchOrder(orderbook::Order& order);
    orderbook::OrderBook* getOrderBook(const InstrumentIndex& instrumentIndex) const;
};
