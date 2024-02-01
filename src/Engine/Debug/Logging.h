#pragma once

#include <inttypes.h>

namespace Engine::Debug::Logging
{
    // For proper alignment, sender should not have more than 16 characters
    void PrintMessage(const char * message, const char * sender);
    // For proper alignment, sender should not have more than 16 characters
    void PrintWarning(const char * message, const char * sender);
    // For proper alignment, sender should not have more than 16 characters
    void PrintError(const char * message, const char * sender);
    
} // namespace Engine::Debug::Logging
