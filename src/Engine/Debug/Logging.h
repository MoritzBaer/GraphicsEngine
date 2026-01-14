#pragma once

#include <cstdint>
#define LOGGING_INCLUDED

#include <inttypes.h>
#include <iostream>
#include <cstring>

#define MAX_IDENTIFIER_LENGTH 16

namespace Engine::Debug::Logging {
class Logger {
  char const *const identifier;
  uint8_t const identifierLength;
  char const *const indentPattern;
  uint8_t numIndents;
  std::ostream &target;

  template <char LineDelim, typename... T_Args> inline void PrintWithAlignedIdentifier(const char *message, T_Args &&...args) const;

public:
  // For proper message alignment, identifier should not have more than MAX_IDENTIFIER_LENGTH characters
  Logger(char const *const identifier, char const *const indentPattern = " |  ", std::ostream &target = std::cout)
      : identifier(identifier), target(target), indentPattern(indentPattern), numIndents(0),
        identifierLength(strlen(identifier)) {}

  template <char LineDelim = '\n', typename... T_Args> void PrintSuccess(const char *format, T_Args &&...args) const;
  template <char LineDelim = '\n', typename... T_Args> void PrintMessage(const char *format, T_Args &&...args) const;
  template <char LineDelim = '\n', typename... T_Args> void PrintWarning(const char *format, T_Args &&...args) const;
  template <char LineDelim = '\n', typename... T_Args> void PrintError(const char *format, T_Args &&...args) const;

  inline void PushSection() { numIndents++; }
  inline void PopSection() { numIndents--; }
};

} // namespace Engine::Debug::Logging

// +-------------------+
// |  Implementations  |
// +-------------------+

#include <format>
#include <sstream>

namespace Engine::Debug::Logging {
static inline constexpr char const *MESSAGE_FORMAT = "\033[37m";
static inline constexpr char const *WARNING_FORMAT = "\033[33m";
static inline constexpr char const *ERROR_FORMAT = "\033[1;31m";
static inline constexpr char const *SUCCESS_FORMAT = "\033[1;32m";
static inline constexpr char const *CLEAR_FORMAT = "\033[0m";

// TODO: Figure out a way to nicely wrap the message if it's longer than a console line
template <char LineDelim, typename... T_Args>
inline void Logger::PrintWithAlignedIdentifier(const char *message, T_Args &&...args) const {
  auto const formattedMessage = std::vformat(message, std::make_format_args(args...));
  std::stringstream messageStream(formattedMessage);

  for (auto i = identifierLength; i < MAX_IDENTIFIER_LENGTH; i++) {
    target << " ";
  }
  target << "[" << identifier << "]  ";
  bool first = true;
  std::string line;
  while (messageStream.good()) {
    if (!first) {
      for (auto i = 0; i < MAX_IDENTIFIER_LENGTH; i++) {
        target << " ";
      }
      target << "*   ";
    }
    first = false;

    // Print indentations
    for (uint8_t i = 0; i < numIndents; i++) {
      target << indentPattern;
    }

    std::getline(messageStream, line, LineDelim);
    // Cut line in two if it is too long?
    target << line << std::endl;
  }
}

template <char LineDelim, typename... T_Args> void Logger::PrintMessage(const char *message, T_Args &&...args) const {
  target << MESSAGE_FORMAT;
  PrintWithAlignedIdentifier<LineDelim>(message, args...);
  target << CLEAR_FORMAT;
}

template <char LineDelim, typename... T_Args> void Logger::PrintSuccess(const char *message, T_Args &&...args) const {
  target << SUCCESS_FORMAT;
  PrintWithAlignedIdentifier<LineDelim>(message, args...);
  target << CLEAR_FORMAT;
}

template <char LineDelim, typename... T_Args> void Logger::PrintWarning(const char *message, T_Args &&...args) const {
  target << WARNING_FORMAT;
  PrintWithAlignedIdentifier<LineDelim>(message, args...);
  target << CLEAR_FORMAT;
}

template <char LineDelim, typename... T_Args> void Logger::PrintError(const char *message, T_Args &&...args) const {
  target << ERROR_FORMAT;
  PrintWithAlignedIdentifier<LineDelim>(message, args...);
  target << CLEAR_FORMAT;
}

} // namespace Engine::Debug::Logging
