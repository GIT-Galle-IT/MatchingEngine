#include <string>
#include <vector>
#include <cstddef>
#include <iostream>
#include <utility>
#include <logging/gLog.h>
#include <boost/pool/singleton_pool.hpp>
#include <cassert>

void print_byte_arry(std::vector<std::byte> byte_array)
{
    std::cout << "[";
    for (int i = 0; i < byte_array.size(); i++)
    {
        (i < byte_array.size() - 1) ? 
            std::cout << std::to_integer<int>(byte_array.at(i)) << "," : std::cout << std::to_integer<int>(byte_array.at(i));
    }
    std::cout << "]" << std::endl;
}

class ByteBuffer
{
    struct VectorPool_tag
    {
    };
    typedef boost::singleton_pool<VectorPool_tag, sizeof(std::vector<std::byte>)> byte_buffer_pool;

private:
    std::vector<std::byte> *byte_array = nullptr;

public:
    ByteBuffer() {};
    ByteBuffer(const char *byteStream, size_t size)
    {
        byte_array = new (byte_buffer_pool::malloc()) std::vector<std::byte>();
        for (int i = 0; i < size; i++)
        {
            byte_array->push_back(static_cast<std::byte>(*(byteStream++))); 
        }
        print_byte_arry(*byte_array);
    }

    std::vector<std::byte> *get()
    {
        assert(byte_array != nullptr);
        return byte_array;
    }

};