#include "Logging.h"

#include <iostream>

namespace Engine::Debug::Logging
{
    static const uint8_t MAX_SENDER_LENGTH = 16;
    static inline char const * MESSAGE_FORMAT = "\033[37m";
    static inline char const * WARNING_FORMAT = "\033[33m";
    static inline char const * ERROR_FORMAT = "\033[1;31m";
    static inline char const * CLEAR_FORMAT = "\033[0m";

    inline void PrintWithAlignedSender(const char *message, const char *sender)
    {
        size_t senderLength = strlen(sender);
        for(int i = senderLength; i < MAX_SENDER_LENGTH; i++) { std::cout << " "; }
        std::cout << "[" << sender << "]  " << message << "\n";
    }

    void PrintMessage(const char *message, const char *sender)
    {
        std::cout << MESSAGE_FORMAT;
        PrintWithAlignedSender(message, sender);
        std::cout << CLEAR_FORMAT;
    }

    void PrintWarning(const char *message, const char *sender)
    {
        std::cout << WARNING_FORMAT;
        PrintWithAlignedSender(message, sender);
        std::cout << CLEAR_FORMAT;
    }

    void PrintError(const char *message, const char *sender)
    {
        std::cout << ERROR_FORMAT;
        PrintWithAlignedSender(message, sender);
        std::cout << CLEAR_FORMAT;
    }   
} // namespace Engine::Debug::Logging