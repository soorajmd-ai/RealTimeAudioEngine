#include "Logger.h"
#include "ConfigManager.h"
#include "buffer/RingBuffer.h"

#include <iostream>
#include <filesystem>

int main()
{
    AudioEngine::RingBuffer<int> buffer(5);

    buffer.push(10);
    buffer.push(20);
    buffer.push(30);

    std::cout << "\nAvailable : " << buffer.available() << std::endl;
    std::cout << "Free Space: " << buffer.freeSpace() << std::endl;

    int value;

    if (buffer.peek(value))
    {
        std::cout << "Peeked: " << value << std::endl;
    }

    while (buffer.pop(value))
    {
        std::cout << "Popped: " << value << std::endl;
    }

    std::cout << "\nAvailable : " << buffer.available() << std::endl;
    std::cout << "Free Space: " << buffer.freeSpace() << std::endl;
}