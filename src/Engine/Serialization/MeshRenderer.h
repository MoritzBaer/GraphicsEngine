#pragma once

#include "Game.h"
#include "Graphics/MeshRenderer.h"

template <>
template <class TokenIterator>
inline constexpr TokenIterator
json<Engine::Graphics::MeshRenderer>::parse_tokenstream(TokenIterator begin, TokenIterator end,
                                                        Engine::Graphics::MeshRenderer &output) {
  if (begin->type == Token::Type::LBrace) {
    begin++;
    std::string key;
    bool is_last;
    do {
      begin = parse_key(begin, end, key);
      if (key == "mesh") {
        std::string value;
        begin = json<std::string>::parse_tokenstream(begin, end, value);
      } else if (key == "material") {
        std::string value;
        begin = json<std::string>::parse_tokenstream(begin, end, value);
      } else {
        throw std::runtime_error("Unexpected key in "
                                 "Engine::Graphics::MeshRenderer"
                                 " : " +
                                 key);
      }
      begin = is_last_in_list(begin, end, is_last);
    } while (!is_last);
    return ++begin;
  }
  throw std::runtime_error("Expected left brace, got " + token_type_to_string(begin->type) + ".");
}