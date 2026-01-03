#include <iostream>
#include <string>
#include <memory>

// predefine message
// change according to order book object structure
struct Message
{
    int int_data{};
    short short_data{};
    int STRING_LENGTH = 150;
    std::string string;
    bool bool_data{};
    long long_data{};

    bool operator==(const Message &other)
    {
        return int_data == other.int_data &&
               short_data == other.short_data &&
               STRING_LENGTH == other.STRING_LENGTH &&
               string == other.string &&
               bool_data == other.bool_data &&
               long_data == other.long_data;
    }

    Message() = default;

    Message(int _1, short _2, const char *_3, bool _4, long _5) : int_data(_1), short_data(_2), STRING_LENGTH(strlen(_3)), string(_3), bool_data(_4), long_data(_5) {}

    void serialize(std::ostream &os) const
    {
        os.write(reinterpret_cast<const char *>(&int_data), sizeof(int_data));
        os.write(reinterpret_cast<const char *>(&short_data), sizeof(short_data));
        os.write(reinterpret_cast<const char *>(&STRING_LENGTH), sizeof(STRING_LENGTH));
        os.write(string.c_str(), string.length());
        os.write(reinterpret_cast<const char *>(&bool_data), sizeof(bool_data));
        os.write(reinterpret_cast<const char *>(&long_data), sizeof(long_data));
    }

    void serialize(gbase::ByteBuffer<std::byte> &bb) const
    {
        bb.append(reinterpret_cast<const char *>(&int_data), sizeof(int_data));
        bb.append(reinterpret_cast<const char *>(&short_data), sizeof(short_data));
        bb.append(reinterpret_cast<const char *>(&STRING_LENGTH), sizeof(STRING_LENGTH));
        bb.append(string.c_str(), string.length());
        bb.append(reinterpret_cast<const char *>(&bool_data), sizeof(bool_data));
        bb.append(reinterpret_cast<const char *>(&long_data), sizeof(long_data));
    }

    void deserialize(std::istream &is)
    {
        is.read(reinterpret_cast<char *>(&int_data), sizeof(int_data));
        is.read(reinterpret_cast<char *>(&short_data), sizeof(short_data));
        is.read(reinterpret_cast<char *>(&STRING_LENGTH), sizeof(STRING_LENGTH));
        auto buffer = std::make_unique<char[]>(STRING_LENGTH + 1);
        buffer[STRING_LENGTH] = '\0';
        is.read(buffer.get(), STRING_LENGTH);
        string.assign(buffer.get()); // copy assign
        buffer.reset();
        is.read(reinterpret_cast<char *>(&bool_data), sizeof(bool_data));
        is.read(reinterpret_cast<char *>(&long_data), sizeof(long_data));
    }

    void deserialize(gbase::ByteBuffer<std::byte> &bb)
    {
        bb.read<sizeof(int)>(reinterpret_cast<char *>(&int_data));
        bb.read<sizeof(short)>(reinterpret_cast<char *>(&short_data));
        bb.read<sizeof(int)>(reinterpret_cast<char *>(&STRING_LENGTH));
        auto buffer = std::make_unique<char[]>(STRING_LENGTH + 1);
        buffer[STRING_LENGTH] = '\0';
        bb.read(STRING_LENGTH, buffer.get());
        string.assign(buffer.get()); // copy assign
        buffer.reset();
        bb.read<sizeof(bool)>(reinterpret_cast<char *>(&bool_data));
        bb.read<sizeof(long)>(reinterpret_cast<char *>(&long_data));
    }
};

inline std::ostream &operator<<(std::ostream &os, const Message &message)
{
    os << message.int_data << " " << message.short_data << " " << message.string << " " << message.bool_data << " " << message.long_data;
    return os;
}

inline std::string to_string(const Message &message)
{
    return std::to_string(message.int_data) + " " + std::to_string(message.short_data) + " " +
           message.string + " " + (message.bool_data ? "true" : "false") + " " + std::to_string(message.long_data);
}