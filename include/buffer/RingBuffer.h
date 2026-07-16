#pragma once

#include <vector>
#include <cstddef>

namespace AudioEngine
{
    template<typename T>
    class RingBuffer
    {
    public:

        explicit RingBuffer(std::size_t capacity)
            :
            m_buffer(capacity),
            m_capacity(capacity),
            m_head(0),
            m_tail(0),
            m_size(0)
        {}

        void clear()
        {
            m_head = 0;
            m_tail = 0;
            m_size = 0;
        }

        std::size_t capacity() const
        {
            return m_capacity;
        }

        std::size_t size() const
        {
            return m_size;
        }

        bool empty() const
        {
            return m_size == 0;
        }

        bool full() const
        {
            return m_size == m_capacity;
        }

        bool push(const T& value)
        {
            if (full())
            {
                return false;
            }

            m_buffer[m_tail] = value;

            m_tail = (m_tail + 1) % m_capacity; //The modulo operator will wrap it back. This is the heart of a ring buffer.

            ++m_size;

            return true;
        }

        bool pop(T& value)
        {
            if (empty())
            {
                return false;
            }

            value = m_buffer[m_head];

            m_head = (m_head + 1) % m_capacity;

            --m_size;

            return true;
        }

    private:

        std::vector<T> m_buffer; //This is our actual memory.

        std::size_t m_capacity; // Stores 8 or 1024 or 4096, depending on the buffer.

        std::size_t m_head; //Read Pointer, consumer reads here.

        std::size_t m_tail; //Write Pointer, producer writes here.

        std::size_t m_size; //Tracks Current number of elements, example, capacity = 8, current elements = 3, then m_size == 3.

    };
}