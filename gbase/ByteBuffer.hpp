#pragma once
#include <string>
#include <vector>
#include <cstddef>
#include <iostream>
#include <cassert>
#include <new>
#include <memory>
#include <algorithm>
#include <logging/gLog.h>

namespace gbase
{
    template <typename T>
    class ByteBuffer
    {
        std::shared_ptr<T> byte_array = nullptr;
        size_t buffer_size = 0;
        size_t filled_size = 0;
        size_t read_itr_ptr = 0;

    public:
        ByteBuffer() : byte_array{nullptr}, buffer_size{0}, filled_size{0}, read_itr_ptr{0} {}

        ByteBuffer(ByteBuffer &&other)
        {
            printf("moving byte buffer at [%p] to this byte buffer [%p]\n", (void *)&other, (void *)this);
            if (other.get() == nullptr)
                return;
            std::shared_ptr<T> other_byte_array = other.get();
            byte_array.swap(other_byte_array);
            buffer_size = other.get_buffer_size();
            filled_size = other.get_filled_size();
            other.release();
        };

        ByteBuffer(const ByteBuffer &other)
        {
            printf("copying byte buffer at [%p] to this byte buffer [%p]\n", (void *)&other, (void *)this);
            if (other.get() == nullptr)
                return;
            // allocate(*(other.get().get()), other.get_filled_size());
            std::shared_ptr<T> other_byte_array = other.get();
            size_t other_byte_array_size = other.get_filled_size();
            if (buffer_size < other_byte_array_size) // need resize or complete allocate
            {
                T *ptr = new (malloc(sizeof(T) * other_byte_array_size)) T;
                byte_array.reset(std::move(ptr));
            }
            T *_ba = byte_array.get();
            T *_other_ba = other_byte_array.get();
            for (size_t i = 0; i < other_byte_array_size; i++)
            {
                _ba[i] = (*(_other_ba++));
            }
            buffer_size = buffer_size > other_byte_array_size ? buffer_size : other_byte_array_size;
            filled_size = other_byte_array_size;
        };

        ByteBuffer &operator=(const ByteBuffer &) = delete;
        ByteBuffer &operator=(ByteBuffer &) = delete;
        ByteBuffer &operator=(ByteBuffer &&) = delete;

        constexpr void resetReadIterator()
        {
            read_itr_ptr = 0;
        }

        ~ByteBuffer()
        {
            release();
        }

        void allocate(const char *byteStream, const size_t size)
        {
            if (buffer_size < size) // need resize or complete allocate
            {
                T *ptr = new (malloc(sizeof(T) * size)) T;
                byte_array.reset(std::move(ptr));
            }
            T *_ba = byte_array.get();
            for (size_t i = 0; i < size; i++)
            {
                _ba[i] = static_cast<T>(*byteStream++);
            }
            buffer_size = buffer_size > size ? buffer_size : size;
            filled_size = size;
        }

        void allocate(const T byteStream[], const size_t size)
        {
            if (buffer_size < size) // need resize or complete allocate
            {
                T *ptr = new (malloc(sizeof(T) * size)) T;
                byte_array.reset(std::move(ptr));
            }
            T *_ba = byte_array.get();
            for (int i = 0; i < size; i++)
            {
                _ba[i] = byteStream[i];
            }
            buffer_size = buffer_size > size ? buffer_size : size;
            filled_size = size;
        }

        void append(const char *byteStream, const size_t size)
        {
            if (byte_array.get() != nullptr)
            {
                // resize
                // TODO:check for int overflows
                if (buffer_size < filled_size + size)
                {
                    T *temp_byte_array = new (malloc(sizeof(T) * 2 * (buffer_size + size))) T;
                    T *underlying_byte_array = byte_array.get();
                    for (size_t i = 0; i < filled_size; i++)
                    {
                        temp_byte_array[i] = underlying_byte_array[i];
                    }
                    byte_array.reset(std::move(temp_byte_array));
                    buffer_size = 2 * (buffer_size + size);
                }
            }
            else
            {
                // allocate
                T *ptr = new (malloc(sizeof(T) * size)) T;
                byte_array.reset(std::move(ptr));
                buffer_size = size;
            }
            T *_ba = byte_array.get();
            for (size_t i = filled_size > 0 ? filled_size : 0; i < filled_size + size; i++)
            {
                _ba[i] = static_cast<T>(*byteStream++);
            }
            filled_size += size;
        }

        void append(const T byteStream[], const size_t size)
        {
            if (byte_array.get() != nullptr)
            {
                // resize
                // TODO:check for int overflows
                if (buffer_size < filled_size + size)
                {
                    T *temp_byte_array = new (malloc(sizeof(T) * 2 * (buffer_size + size))) T;
                    T *underlying_byte_array = byte_array.get();
                    for (size_t i = 0; i < filled_size; i++)
                    {
                        temp_byte_array[i] = underlying_byte_array[i];
                    }
                    byte_array.reset(std::move(temp_byte_array));
                    buffer_size = 2 * (buffer_size + size);
                }
            }
            else
            {
                // allocate
                T *ptr = new (malloc(sizeof(T) * size)) T;
                byte_array.reset(std::move(ptr));
                buffer_size = size;
            }
            T *_ba = byte_array.get();
            for (size_t i = filled_size > 0 ? filled_size : 0; i < filled_size + size; i++)
            {
                _ba[i] = byteStream[i];
            }
            filled_size += size;
        }

        template <typename U>
        constexpr void read(const size_t start, char *dest) const
        {
            this->read(start, sizeof(U), dest);
        }

        template <size_t size>
        constexpr void read(char *dest)
        {
            this->read(read_itr_ptr, size, dest);
            read_itr_ptr += size;
        }

        // read while iterating
        constexpr void read(const size_t size, char *dest)
        {
            this->read(read_itr_ptr, size, dest);
            read_itr_ptr += size;
        }

        constexpr void read(const size_t start, size_t size, char *dest) const
        {
            assert(start < buffer_size && start + size <= buffer_size);
            if (byte_array.get() != nullptr)
            {
                for (size_t i = start; i < start + size; i++)
                {
                    dest[i - start] = static_cast<char>(byte_array.get()[i]);
                }
            }
        }

        [[nodiscard]] auto get() const -> std::shared_ptr<T>
        {
            return byte_array;
        }

        [[nodiscard]] auto get_buffer_size() const -> size_t
        {
            return buffer_size;
        }

        [[nodiscard]] auto get_filled_size() const -> size_t
        {
            return filled_size;
        }

        void release()
        {
            byte_array.reset();
            buffer_size = 0;
            filled_size = 0;
            read_itr_ptr = 0;
        }
    };

    inline void print_byte_array(const std::vector<std::byte> &byte_array)
    {
        std::cout << "[";
        for (unsigned int i = 0; i < byte_array.size(); i++)
        {
            (i < byte_array.size() - 1)
                ? std::cout << std::to_integer<int>(byte_array.at(i)) << ","
                : std::cout << std::to_integer<int>(byte_array.at(i));
        }
        std::cout << "]" << std::endl;
    }

    inline void print_byte_array(const gbase::ByteBuffer<std::byte> &byte_array)
    {
        std::cout << "[";
        for (unsigned int i = 0; i < byte_array.get_filled_size(); i++)
        {
            std::cout << std::to_integer<int>(byte_array.get().get()[i]);
            (i < byte_array.get_filled_size() - 1) ? std::cout << "," : std::cout << "";
        }
        std::cout << "]" << std::endl;
    }

    // TODO : create byte buffer iterator for constrained access to underlying buffer
}
