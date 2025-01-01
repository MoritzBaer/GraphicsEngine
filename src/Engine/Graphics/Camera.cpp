#include "Camera.h"

#include "Debug/Logging.h"

namespace Engine::Graphics {

void Camera::CopyFrom(Core::Component const *other) {
  Camera const *otherCamera = dynamic_cast<Camera const *>(other);
  ENGINE_ASSERT(otherCamera, "Tried to copy from a non-camera component!")
  projection = otherCamera->projection;
}

} // namespace Engine::Graphics
