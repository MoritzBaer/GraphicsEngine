#pragma once

#include <cstring>

namespace Engine::Graphics {
class PushConstantsAggregate {
private:
  void *data;
  size_t size;

public:
  PushConstantsAggregate() : data(nullptr), size(0) {}
  ~PushConstantsAggregate() { delete data; }
  template <typename T> PushConstantsAggregate &PushData(T *data) {
    char *newData = reinterpret_cast<char *>(malloc(size + sizeof(T)));
    memcpy(newData, this->data, size);
    memcpy(newData + size, data, sizeof(T));
    this->data = newData;
    size += sizeof(T);

    return *this;
  }

  inline void *Data() { return data; }
  inline size_t Size() { return size; }
};

} // namespace Engine::Graphics
