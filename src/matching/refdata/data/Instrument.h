#pragma once

#include <string>
#include "matching/exchange/data/OrderBook.h"
#include "matching/refdata/data/Issuer.h"

namespace refdata::data
{
    class Instrument
    {
        public:
            Instrument(int instrumetId, std::string instrumentName):gInstrumentId(instrumetId), 
                                            gInstrumentName(instrumentName),
                                            gBuyOrderBook(nullptr),
                                            gSellOrderBook(nullptr)
            {
                init();
            }

            ~Instrument()
            {
                delete gBuyOrderBook;
                delete gSellOrderBook;
            };

            void init()
            {
                gBuyOrderBook->setPassiveOrderBook(gSellOrderBook);
                gSellOrderBook->setPassiveOrderBook(gBuyOrderBook);
            }

            int getInstrumentId() const { return gInstrumentId; }
            void setInstrumentId(int instrumentId) { gInstrumentId = instrumentId; }

            std::string getInstrumentName() const { return gInstrumentName; }
            void setInstrumentName(const std::string &instrumentName) { gInstrumentName = instrumentName; }

            matching::exchange::data::OrderBook* getGBuyOrderBook() const { return gBuyOrderBook; }
            void setGBuyOrderBook(matching::exchange::data::OrderBook* gBuyOrderBook_) { gBuyOrderBook = gBuyOrderBook_; }

            matching::exchange::data::OrderBook* getGSellOrderBook() const { return gSellOrderBook; }
            void setGSellOrderBook(matching::exchange::data::OrderBook* gSellOrderBook_) { gSellOrderBook = gSellOrderBook_; }
            
        private:
            int gInstrumentId;
            std::string gInstrumentName;

            Issuer* gIssuer;

            matching::exchange::data::OrderBook* gBuyOrderBook;
            matching::exchange::data::OrderBook* gSellOrderBook;

    };
}