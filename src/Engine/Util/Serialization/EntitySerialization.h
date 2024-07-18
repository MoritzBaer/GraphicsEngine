#include "Core/ECS.h"
#include "Editor/Display.h"
#include "Graphics/MeshRenderer.h"
#include "Graphics/Transform.h"
#include "json-parsing.h"

#define COMPONENT_OPTION(Name)                                                                                         \
  if (key == #Name) {                                                                                                  \
    begin = json<Name>::parse_tokenstream(begin, end, *output.AddComponent<Name>(), context);                          \
  } else

template <>
template <class TokenIterator>
inline constexpr TokenIterator json<Engine::Core::Entity>::parse_tokenstream(TokenIterator begin, TokenIterator end,
                                                                             Engine::Core::Entity &output,
                                                                             void *context) {
  if (begin->type == Token::Type::LBrace) {
    begin++;
    std::string key;
    bool is_last;
    do {
      begin = parse_key(begin, end, key);
      if (key == "components") {
        // Parse component array
        if (begin->type == Token::Type::LBracket) {
          begin++;
          while (begin->type != Token::Type::RBracket) {
            // Parse single component
            if (begin->type == Token::Type::LBrace) {
              begin++;
              std::string key;
              begin = parse_key(begin, end, key);
              COMPONENT_OPTION(Engine::Graphics::MeshRenderer)
              COMPONENT_OPTION(Engine::Graphics::Transform)
              COMPONENT_OPTION(Engine::Editor::Display) {
                throw std::runtime_error("Unexpected key in components: " + key);
              }
              if (begin->type != Token::Type::RBrace) {
                throw std::runtime_error("Expected right brace, got " + token_type_to_string(begin->type) + ".");
              }
              begin++;
            } else {
              throw std::runtime_error("Expected left brace, got " + token_type_to_string(begin->type) + ".");
            }
            if (begin->type == Token::Type::Comma) {
              begin++;
            }
          }
          begin++;
        } else {
          throw std::runtime_error("Expected left bracket, got " + token_type_to_string(begin->type) + ".");
        }
      } else {
        throw std::runtime_error("Unexpected key in Engine::Core::Entity: " + key);
      }
      begin = is_last_in_list(begin, end, is_last);
    } while (!is_last);
    return ++begin;
  }
  throw std::runtime_error("Expected left brace, got " + token_type_to_string(begin->type) + ".");
}