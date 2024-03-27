#pragma once

#include "Debug/Logging.h"
#include "Macros.h"
#include <algorithm>
#include <inttypes.h>

#define STEP_OVER_COMMA(streamName)                                                                                    \
  if (*streamName == ',') {                                                                                            \
    streamName++;                                                                                                      \
  }

#define FIRST_TOKEN_REACTION(token, reaction)                                                                          \
  if (!strcmp(token, tokenBuffer)) {                                                                                   \
    reaction;                                                                                                          \
  }

#define LATER_TOKEN_REACTION(token, reaction) else FIRST_TOKEN_REACTION(token, reaction)

// TODO: Safely pass unknown token and corresponding values so next token can be read without issue.
#define UNKOWN_TOKEN_REACTION(origin)                                                                                  \
  else {                                                                                                               \
    ENGINE_WARNING("Unknown token '{}' in " origin " serialization!", tokenBuffer);                                    \
  }

#define PARSE_VALUE_LIST(streamName, content, source)                                                                  \
  const char *&sourceStream = streamName;                                                                              \
  SkipWhitespace(sourceStream);                                                                                        \
  ENGINE_ASSERT(*sourceStream == '{', "Expected '{' at start of block!");                                              \
  sourceStream++; /* Enter block*/                                                                                     \
  char tokenBuffer[64] = {0};                                                                                          \
  uint8_t lastChar = 0;                                                                                                \
  SkipWhitespace(sourceStream);                                                                                        \
  while (*streamName != '}') {                                                                                         \
    content;                                                                                                           \
    STEP_OVER_COMMA(sourceStream) SkipWhitespace(sourceStream);                                                        \
  }                                                                                                                    \
  sourceStream++; /* Exit block */

#define PARSE_BLOCK(streamName, content, source)                                                                       \
  PARSE_VALUE_LIST(streamName, ReadTokenToBuffer(sourceStream, tokenBuffer, sizeof(tokenBuffer) / sizeof(char));       \
                   content, source)

#define UNKOWN_TOKEN_WARNING(tokenBuffer, source)                                                                      \
  ENGINE_WARNING("Unknown token '{}' in " source " serialization!", tokenBuffer);

#define PARSE_ARRAY(streamName, itemHandling, source)                                                                  \
  SkipWhitespace(streamName);                                                                                          \
  ENGINE_ASSERT(*streamName == '[', "Expected '[' at start of " source " array!");                                     \
  streamName++; /* Enter array*/                                                                                       \
  while (*streamName != ']') {                                                                                         \
    itemHandling;                                                                                                      \
    STEP_OVER_COMMA(streamName);                                                                                       \
    SkipWhitespace(streamName);                                                                                        \
  }                                                                                                                    \
  streamName++; /* Exit array*/

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
// TODO: Code duplication between this and ReadTokenToBuffer. Figure out a way to refactor this.
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
