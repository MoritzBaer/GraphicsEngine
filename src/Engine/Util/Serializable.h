#pragma once

#include <sstream>
#include <vector>

namespace Engine::Util {

class Serializable {
public:
  virtual void Serialize(std::stringstream &targetStream) const = 0;
};

} // namespace Engine::Util
