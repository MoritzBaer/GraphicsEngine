#pragma once

#include <cstdlib>
#include <cstring>

namespace Engine::Graphics {
class PushConstantsAggregate {
private:
  void *data;
  size_t size;

public:
  PushConstantsAggregate() : data(nullptr), size(0) {}
  ~PushConstantsAggregate() { free(data); }
  template <typename T> PushConstantsAggregate &PushData(T *data) {
    char *newData = reinterpret_cast<char *>(malloc(size + sizeof(T)));
    memcpy(newData, this->data, size);
    memcpy(newData + size, data, sizeof(T));
    free(this->data);
    this->data = newData;
    size += sizeof(T);

    return *this;
  }

  inline void const *Data() const { return data; }
  inline size_t Size() const { return size; }
};

} // namespace Engine::Graphics
