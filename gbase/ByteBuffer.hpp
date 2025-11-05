#pragma once
#include <string>
#include <vector>
#include <cstddef>
#include <iostream>
#include <cassert>
#include <new>
#include <memory>
#include <algorithm>

namespace gbase {
    inline void print_byte_array(const std::vector<std::byte> &byte_array)
    {
        std::cout << "[";
        for (unsigned int i = 0; i < byte_array.size(); i++)
        {
            (i < byte_array.size() - 1) ? std::cout << std::to_integer<int>(byte_array.at(i)) << "," : std::cout << std::to_integer<int>(byte_array.at(i));
        }
        std::cout << "]" << std::endl;
    }

    template<typename T>
    class ByteBuffer
    {
        std::shared_ptr<T> byte_array = nullptr;
        size_t buffer_size = 0;

    public:
        void allocate(const char *byteStream, const size_t size)
        {
            if (buffer_size < size) // need resize or complete allocate
            {
                T* ptr = new (malloc(sizeof(T) * size)) T;
                byte_array.reset(std::move(ptr));
            }
            T* _ba = byte_array.get();
            std::cout << buffer_size - size;
            for (int i = 0; i < size; i++)
            {
                _ba[i] = static_cast<T>(*byteStream++);
            }
            buffer_size = buffer_size > size ? buffer_size : size;
        }

        void append(const char *byteStream, const size_t size)
        {
            if (byte_array.get() != nullptr)
            {
                // resize
                // TODO:check for int overflows
                T* temp_byte_array = new (malloc(sizeof(T) * 2*(buffer_size + size))) T;
                T* underlying_byte_array = byte_array.get();
                for (size_t i = 0; i < buffer_size; i++)
                {
                    temp_byte_array[i] = *(underlying_byte_array++);
                }
                byte_array.reset(std::move(temp_byte_array));
            }
            else
            {
                // allocate
                T* ptr = new (malloc(sizeof(T) * size)) T;
                byte_array.reset(std::move(ptr));
            }
            T* _ba = byte_array.get();
            for (int i = buffer_size > 0 ? buffer_size : 0; i < buffer_size + size; i++)
            {
                _ba[i] = static_cast<T>(*byteStream++);
            }
            buffer_size += size;
        }

        void append(const T *byteStream, const size_t size)
        {

        }

        [[nodiscard]] auto get() const -> std::shared_ptr<T> {
            return byte_array;
        }

        void release()
        {
            byte_array.reset();
            byte_array = nullptr;
            buffer_size = 0;
        }
    };

    // TODO : create byte buffer iterator for constrained access to underlying buffer
}