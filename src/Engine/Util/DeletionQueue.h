#pragma once

#include <inttypes.h>

namespace Engine {
class Destroyable {
public:
  virtual void Destroy() = 0;
};
class ConstDestroyable : public Destroyable {
public:
  void Destroy() override { ((ConstDestroyable const *)this)->Destroy(); }
  virtual void Destroy() const = 0;
};

class Initializable : public Destroyable {
public:
  virtual void Create() = 0;
};

// Only destruction is const
class ConstInitializable : public Initializable {
public:
  void Destroy() override { ((ConstInitializable const *)this)->Destroy(); }
  virtual void Destroy() const = 0;
};

struct DeletionQueue {
private:
  Destroyable **queue;
  uint32_t queueSize;
  void ChangeBaseMemory();

public:
  uint32_t queueCapacity;
  void Create();
  void Destroy() const;
  void Push(Initializable *object, bool preInitialized = false);
  inline void Push(Initializable &object, bool preInitialized = false) { Push(&object, preInitialized); }
  void Push(Destroyable *object);
  inline void Push(Destroyable &object) { Push(&object); };
  void Flush();
};

inline DeletionQueue mainDeletionQueue;

} // namespace Engine
