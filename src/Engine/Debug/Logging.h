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
#include <sstream>

namespace Engine::Debug::Logging
{
    static constexpr uint8_t MAX_SENDER_LENGTH = 16;
    static inline constexpr char const * MESSAGE_FORMAT = "\033[37m";
    static inline constexpr char const * WARNING_FORMAT = "\033[33m";
    static inline constexpr char const * ERROR_FORMAT = "\033[1;31m";
    static inline constexpr char const * SUCCESS_FORMAT = "\033[1;32m";
    static inline constexpr char const * CLEAR_FORMAT = "\033[0m";

    // TODO: Figure out a way to nicely wrap the message if it's longer than a console line
    template<typename ...T_Args>
    inline void PrintWithAlignedSender(const char *sender, const char *message, T_Args&& ... args)
    {
        auto const formattedMessage = std::vformat(message, std::make_format_args(args...));
        std::stringstream messageStream(formattedMessage);

        size_t senderLength = strlen(sender);
        for(auto i = senderLength; i < MAX_SENDER_LENGTH; i++) { std::cout << " "; }
        std::cout << "[" << sender << "]  ";
        bool first = true;
        while (messageStream.good()) {
            if (!first) {
                for(auto i = 0; i < MAX_SENDER_LENGTH; i++) { std::cout << " "; }
                std::cout << "*   ";
            }
            first = false;
            std::string line;
            std::getline(messageStream, line, '\n');
            std::cout << line << std::endl;
        }
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
