#include <iostream>
#include <string>

// predefine message
// change according to order book object structure
struct OrderMessage
{
    std::string instrumentID = "";
    int INSTRUMENT_ID_LENGTH = 150;
    int orderType;
    int orderQuantity;
    int price;

    OrderMessage(std::string instrumentID_, int orderType_, int orderQuantity_, int price_) : instrumentID(instrumentID_),
                                                                                              orderType(orderType_),
                                                                                              orderQuantity(orderQuantity_),
                                                                                              INSTRUMENT_ID_LENGTH(instrumentID_.size()),
                                                                                              price(price_) {}

    void serialize(std::ostream &os)
    {
        os.write(reinterpret_cast<const char *>(&orderType), sizeof(orderType));
        os.write(reinterpret_cast<const char *>(&orderQuantity), sizeof(orderQuantity));
        os.write(reinterpret_cast<const char *>(&INSTRUMENT_ID_LENGTH), sizeof(INSTRUMENT_ID_LENGTH));
        os.write(instrumentID.c_str(), instrumentID.length());
        os.write(reinterpret_cast<const char *>(&price), sizeof(price));
    }

    void deserialize(std::istream &is)
    {
        is.read(reinterpret_cast<char *>(&orderType), sizeof(orderType));
        is.read(reinterpret_cast<char *>(&orderQuantity), sizeof(orderQuantity));
        is.read(reinterpret_cast<char *>(&INSTRUMENT_ID_LENGTH), sizeof(INSTRUMENT_ID_LENGTH));
        char buffer[INSTRUMENT_ID_LENGTH];
        buffer[INSTRUMENT_ID_LENGTH] = '\0';
        is.read(buffer, INSTRUMENT_ID_LENGTH);
        instrumentID = buffer; // copy assign
        is.read(reinterpret_cast<char *>(&price), sizeof(price));
    }
};

std::ostream &operator<<(std::ostream &os, OrderMessage message)
{
    os << message.instrumentID << " " << message.orderType << " " << message.orderQuantity << " " << message.price;
    return os;
}