#include <string>
#include <vector>
#include <cstddef>
#include <iostream>
#include <utility>
#include <logging/gLog.h>
#include <boost/pool/singleton_pool.hpp>
#include <boost/pool/object_pool.hpp>
// #include <ObjectPool.hpp>
#include <cassert>
#include <new>

void print_byte_arry(std::vector<std::byte> byte_array)
{
    std::cout << "[";
    for (int i = 0; i < byte_array.size(); i++)
    {
        (i < byte_array.size() - 1) ? std::cout << std::to_integer<int>(byte_array.at(i)) << "," : std::cout << std::to_integer<int>(byte_array.at(i));
    }
    std::cout << "]" << std::endl;
}

template<typename T>
class ByteBuffer
{
    struct VectorPool_tag
    {
    };
    typedef boost::singleton_pool<VectorPool_tag, sizeof(T)> singleton_byte_buffer_pool;

private:
    T *byte_array = nullptr;
    size_t buffer_size = 0;
    boost::object_pool<T> p;

public:
    ByteBuffer() {};

    void allocate_singleton_pool(const char *byteStream, size_t size)
    {
        buffer_size = size;
        byte_array = static_cast<T*>(singleton_byte_buffer_pool::ordered_malloc(size));
        
        for (int i = 0; i < size; i++)
        {
            byte_array[i] = static_cast<T>(*(byteStream++));
        }
        // for (int i = 0; i < size; i++) {
        //     std::cout << (char)byte_array[i];
        // }
    }

    void allocate_object_pool(const char *byteStream, size_t size)
    {
        buffer_size = size;
        byte_array = static_cast<T*>(p.malloc());
        for (int i = 0; i < size; i++)
        {
            byte_array[i] = static_cast<T>(*(byteStream++));
        }
        // for (int i = 0; i < size; i++) {
        //     std::cout << (char)byte_array[i];
        // }
    }

    void allocate_new(const char *byteStream, size_t size)
    {
        buffer_size = size;
        // byte_array = new T[size];
        if (byte_array == nullptr)
            byte_array = new (malloc(sizeof(T) * size)) T;
        for (int i = 0; i < size; i++)
        {
            byte_array[i] = static_cast<T>(*(byteStream++));
        }
        // for (int i = 0; i < size; i++) {
        //     std::cout << (char)byte_array[i];
        // }
    }

    T *get()
    {
        assert(byte_array != nullptr);
        return byte_array;
    }

    void release_singleton_pool()
    {
        assert(byte_array != nullptr);
        // singleton_byte_buffer_pool::release_memory();
        singleton_byte_buffer_pool::ordered_free(byte_array, buffer_size);
        byte_array = nullptr;
    }

    void release_object_pool()
    {
        assert(byte_array != nullptr);
        // p.free(byte_array);
        // p.ordered_free(byte_array, buffer_size);
        byte_array = nullptr;
    }

    void release_new()
    {
        assert(byte_array != nullptr);
        // singleton_byte_buffer_pool::release_memory();
        // singleton_byte_buffer_pool::ordered_free(byte_array, buffer_size);
        delete[] byte_array;
        byte_array = nullptr;
    }
};