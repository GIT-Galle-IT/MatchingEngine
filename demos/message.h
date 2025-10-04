#include <iostream>
#include <string>

// predefine message
// change according to order book object structure
struct Message
{
    int int_data;
    short short_data;
    int STRING_LENGTH = 150;
    std::string string = "";
    bool bool_data;
    long long_data;

    Message(int _1, short _2, const char* _3, bool _4, long _5) :
        int_data(_1), short_data(_2), string(_3), STRING_LENGTH(strlen(_3)), bool_data(_4), long_data(_5){}

    void serialize(std::ostream& os)
    {
        os.write(reinterpret_cast<const char*>(&int_data), sizeof(int_data));
        os.write(reinterpret_cast<const char*>(&short_data), sizeof(short_data));
        os.write(reinterpret_cast<const char*>(&STRING_LENGTH), sizeof(STRING_LENGTH));
        os.write(string.c_str(), string.length());
        os.write(reinterpret_cast<const char*>(&bool_data), sizeof(bool_data));
        os.write(reinterpret_cast<const char*>(&long_data), sizeof(long_data));
    }

    void deserialize(std::istream& is)
    {
        is.read(reinterpret_cast<char*>(&int_data), sizeof(int_data));
        is.read(reinterpret_cast<char*>(&short_data), sizeof(short_data));
        is.read(reinterpret_cast<char*>(&STRING_LENGTH), sizeof(STRING_LENGTH));
        char buffer[STRING_LENGTH];
        buffer[STRING_LENGTH] = '\0';
        is.read(buffer, STRING_LENGTH);
        string = buffer; // copy assign
        is.read(reinterpret_cast<char*>(&bool_data), sizeof(bool_data));
        is.read(reinterpret_cast<char*>(&long_data), sizeof(long_data));
    }
};

std::ostream& operator<<(std::ostream& os, Message message)
{
    os << message.int_data << " " << message.short_data << " " << 
        message.string << " " << message.bool_data << " " << message.long_data;
    return os;
}

std::string to_string(const Message& message)
{
    return std::to_string(message.int_data) + " " + std::to_string(message.short_data) + " " +
           message.string + " " + (message.bool_data ? "true" : "false") + " " + std::to_string(message.long_data);
}