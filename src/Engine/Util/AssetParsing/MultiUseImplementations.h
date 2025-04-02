#pragma once

#include "json-parsing.h"

namespace Engine {
template <typename T> struct JsonParser {
  T ParseDSO(std::vector<char> const &source) const { return json<T>::deserialize(source); }
};

template <typename T> struct OwnedDestroyer {
  void DestroyAsset(T *&asset) const { delete asset; }
};
} // namespace Engine
