#pragma once

#include <inttypes.h>

namespace Engine::Debug::Logging
{
    // For proper alignment, sender should not have more than 16 characters
    template<typename ...T_Args>
    void PrintMessage(const char * sender, const char * format, T_Args&& ... args);

    // For proper alignment, sender should not have more than 16 characters
    template<typename ...T_Args>
    void PrintWarning(const char * sender, const char * format, T_Args&& ... args);

    // For proper alignment, sender should not have more than 16 characters
    template<typename ...T_Args>
    void PrintError(const char * sender, const char * format, T_Args&& ... args);
    
} // namespace Engine::Debug::Logging

// +-------------------+
// |  Implementations  |
// +-------------------+

#include <iostream>
#include <format>

namespace Engine::Debug::Logging
{
    static const uint8_t MAX_SENDER_LENGTH = 16;
    static inline char const * MESSAGE_FORMAT = "\033[37m";
    static inline char const * WARNING_FORMAT = "\033[33m";
    static inline char const * ERROR_FORMAT = "\033[1;31m";
    static inline char const * SUCCESS_FORMAT = "\033[1;32m";
    static inline char const * CLEAR_FORMAT = "\033[0m";

    // TODO: Figure out a way to nicely wrap the message if it's longer than a console line
    template<typename ...T_Args>
    inline void PrintWithAlignedSender(const char *sender, const char *message, T_Args&& ... args)
    {
        size_t senderLength = strlen(sender);
        for(int i = senderLength; i < MAX_SENDER_LENGTH; i++) { std::cout << " "; }
        std::cout << "[" << sender << "]  " << std::vformat(message, std::make_format_args(args...)) << std::endl;
    }

    template<typename ...T_Args>
    void PrintMessage(const char *sender, const char *message, T_Args&& ... args)
    {
        std::cout << MESSAGE_FORMAT;
        PrintWithAlignedSender(sender, message, args...);
        std::cout << CLEAR_FORMAT;
    }

    template<typename ...T_Args>
    void PrintSuccess(const char *sender, const char *message, T_Args&& ... args)
    {
        std::cout << SUCCESS_FORMAT;
        PrintWithAlignedSender(sender, message, args...);
        std::cout << CLEAR_FORMAT;
    }

    template<typename ...T_Args>
    void PrintWarning(const char *sender, const char *message, T_Args&& ... args)
    {
        std::cout << WARNING_FORMAT;
        PrintWithAlignedSender(sender, message, args...);
        std::cout << CLEAR_FORMAT;
    }

    template<typename ...T_Args>
    void PrintError(const char *sender, const char *message, T_Args&& ... args)
    {
        std::cout << ERROR_FORMAT;
        PrintWithAlignedSender(sender, message, args...);
        std::cout << CLEAR_FORMAT;
    }   


} // namespace Engine::Debug::Logging
