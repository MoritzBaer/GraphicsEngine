#pragma once

#include "Debug/Logging.h"
#include "Macros.h"
#include <algorithm>
#include <inttypes.h>

namespace Engine::Util {

inline bool IsWhitespace(char c) { return c == ' ' || c == '\n'; }

inline bool isDigit(char c) { return c >= '0' && c <= '9'; }

inline bool IsDelimiter(char c) { return c == ':' || c == ',' || c == '{' || c == '}' || c == '[' || c == ']'; }

inline void SkipWhitespace(char const *&charStream) {
  while (IsWhitespace(*charStream)) {
    charStream++;
  }
}

inline void ReadTokenToBuffer(char const *&charStream, char *tokenBuffer, uint8_t tokenBufferSize) {
  // Clear token buffer
  std::fill(tokenBuffer, tokenBuffer + tokenBufferSize, 0);
  uint8_t lastTokenChar = 0;

  SkipWhitespace(charStream);

  while (!IsWhitespace(*charStream) && !IsDelimiter(*charStream)) { // Write token to buffer
    tokenBuffer[lastTokenChar++] = *charStream++;
  }

  SkipWhitespace(charStream);

  if (*charStream != ':') {
    ENGINE_ERROR("Label token was not followed by delimiter!");
  }
  *charStream++;
}

// TODO: Strings containing spaces or commas are not handled correctly. Implement a proper string parser.
inline void ReadValueToBuffer(char const *&charStream, char *valueBuffer, uint8_t valueBufferSize) {
  // Clear float buffer
  std::fill(valueBuffer, valueBuffer + valueBufferSize, 0);
  uint8_t lastBufferChar = 0;

  SkipWhitespace(charStream);

  while (!IsWhitespace(*charStream) && !IsDelimiter(*charStream)) { // Write token to buffer
    valueBuffer[lastBufferChar++] = *charStream++;
  }
  if (*charStream == ',') {
    charStream++;
  }
}

inline float ReadFloat(char const *&charStream) {
  char floatBuffer[64] = {0};
  uint8_t lastBufferChar = 0;

  SkipWhitespace(charStream);

  ReadValueToBuffer(charStream, floatBuffer, sizeof(floatBuffer) / sizeof(char));

  return std::atof(floatBuffer);
}
} // namespace Engine::Util
