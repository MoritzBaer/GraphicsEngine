#pragma once

#include "Buffer.h"
#include "Debug/Logging.h"
#include "GPUObjectManager.h"
#include "Graphics/CommandQueue.h"
#include "Graphics/MemoryAllocator.h"
#include <array>
#include <stack>

#define MIN_UNIFORM_SIZE_FOR_STAGING_IN_UPLOAD 256

namespace Engine::Graphics {

class BufferProducer {
public:
  virtual void ResetBuffers() = 0;
  virtual void FreeBuffers() = 0;
};

typedef uint8_t typeId_t;

template <typename T_Uniform> struct TypeID {
  inline static typeId_t value = -1;
};
inline typeId_t nextFreeType = 0;

class UniformBinder {
  std::array<BufferProducer *, std::numeric_limits<typeId_t>::max()> bufferProducers;
  GPUObjectManager RELEASE_CONST *gpuObjectManager;
  GPUDispatcher gpuDispatcher;

public:
  UniformBinder() : gpuObjectManager(nullptr), gpuDispatcher(), bufferProducers() {}

  UniformBinder(GPUObjectManager
#ifdef NDEBUG
                const
#endif
                    *gpuObjectManager)
      : gpuObjectManager(gpuObjectManager), gpuDispatcher(gpuObjectManager->CreateGPUDispatcher()),
        bufferProducers() {
  }

  template <typename T_Uniform> inline UniformBinding GetBinding(T_Uniform const &data);
  inline void ResetBuffers();
  inline void Destroy();
};

template <typename T_Uniform> class UniformBufferProducer : public BufferProducer {
  std::stack<Buffer<T_Uniform>> availableBuffers;
  std::stack<Buffer<T_Uniform>> usedBuffers;
  GPUObjectManager RELEASE_CONST *gpuObjectManager;
  GPUDispatcher const *gpuDispatcher;

  inline static constexpr bool STAGING_REQUIRED = sizeof(T_Uniform) > MIN_UNIFORM_SIZE_FOR_STAGING_IN_UPLOAD;
  Buffer<T_Uniform> stagingBuffer;// TODO: Destroy staging buffer when binder is destroyed

  inline constexpr void UploadUniform(Buffer<T_Uniform> &buffer, T_Uniform const &data) {
    if constexpr (STAGING_REQUIRED) {
      stagingBuffer.SetData(data);
      gpuDispatcher->Dispatch(
        [&](CommandRecorder const & recorder) {
          recorder.RecordCopy(stagingBuffer, buffer);
        });
    } else {
      buffer.SetData(data);
    }
  }

  inline Buffer<T_Uniform> ProduceBuffer() {
    if constexpr (STAGING_REQUIRED) {
      return gpuObjectManager->CreateBuffer<T_Uniform>(
          1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    } else {
      return gpuObjectManager->CreateBuffer<T_Uniform>(1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                       VMA_MEMORY_USAGE_CPU_TO_GPU);
    }
  }

public:
  UniformBufferProducer(GPUObjectManager RELEASE_CONST *gpuObjectManager, GPUDispatcher const *gpuDispatcher)
      : gpuObjectManager(gpuObjectManager), gpuDispatcher(gpuDispatcher), availableBuffers(), usedBuffers() {
    if constexpr (STAGING_REQUIRED) {
      stagingBuffer =
          gpuObjectManager->CreateBuffer<T_Uniform>(1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    }
  }

  inline Buffer<T_Uniform> &GetBuffer(T_Uniform const &data) {
    if (availableBuffers.empty()) {
      usedBuffers.push(ProduceBuffer());
    } else {
      usedBuffers.push(availableBuffers.top());
      availableBuffers.pop();
    }

    auto &buffer = usedBuffers.top();
    UploadUniform(buffer, data);
    return buffer;
  }

  void ResetBuffers() override {
    while (!usedBuffers.empty()) {
      availableBuffers.push(usedBuffers.top());
      usedBuffers.pop();
    }
  }

  void FreeBuffers() override {
    ResetBuffers();
    while (!availableBuffers.empty()) {
      auto &buffer = availableBuffers.top();
      gpuObjectManager->DestroyBuffer(buffer);
      availableBuffers.pop();
    }
  }
};

template <typename T_Uniform> inline UniformBinding UniformBinder::GetBinding(T_Uniform const &data) {
  if (TypeID<T_Uniform>::value == typeId_t(-1)) {
    TypeID<T_Uniform>::value = nextFreeType++;
  }
  if (bufferProducers[TypeID<T_Uniform>::value] == nullptr) {
    bufferProducers[TypeID<T_Uniform>::value] = new UniformBufferProducer<T_Uniform>(gpuObjectManager, &gpuDispatcher);
  }
  auto bufferProducer = static_cast<UniformBufferProducer<T_Uniform> *>(bufferProducers[TypeID<T_Uniform>::value]);
  return bufferProducer->GetBuffer(data).BindAsUniform();
}

inline void UniformBinder::ResetBuffers() {
  for (auto &bufferProducer : bufferProducers) {
    if (bufferProducer != nullptr) {
      bufferProducer->ResetBuffers();
    }
  }
}

inline void UniformBinder::Destroy() {
  for (auto &bufferProducer : bufferProducers) {
    if (bufferProducer != nullptr) {
      bufferProducer->FreeBuffers();
      delete bufferProducer;
      bufferProducer = nullptr;
    }
  }
  gpuObjectManager->DestroyGPUDispatcher(gpuDispatcher);
}

} // namespace Engine::Graphics
