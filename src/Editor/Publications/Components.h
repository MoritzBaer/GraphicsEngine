#pragma once

#include "Editor/Display.h"
#include "Editor/Publications/NumericTypes.h"
#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/MeshRenderer.h"
#include "Engine/Graphics/Transform.h"

namespace Editor {

// template <> const char *Publishable<Engine::Graphics::Camera>::typeLabel = "Camera";
// template <> inline Publication Publishable<Engine::Graphics::Camera>::Publish(Engine::Graphics::Camera &value) {
//   return Publication{.label = typeLabel,
//                      .type = Publication::Type::FLOAT1,
//                      .style = Publication::Style::DRAG,
//                      .flags = 0,
//                      .floatRange = {},
//                      .intRange = {},
//                      .referencedPointer = &value};
// };

inline struct PublishedRotation {
  Engine::Maths::Vector3 rotation;
  Engine::Maths::Vector3 oldRotation;
  Engine::Graphics::Transform *referredTransform;
} publishedRotation{};

template <> inline constexpr char const *Publishable<Engine::Graphics::Transform>::typeLabel = "Transform";
template <>
inline Publication Publishable<Engine::Graphics::Transform>::Publish(Engine::Graphics::Transform &value,
                                                                     const char *label) {
  if (publishedRotation.referredTransform && publishedRotation.oldRotation != publishedRotation.rotation) {
    publishedRotation.referredTransform->rotation =
        Engine::Maths::Quaternion::FromEulerAngles(publishedRotation.rotation);
  }
  publishedRotation.rotation = value.rotation.EulerAngles();
  publishedRotation.oldRotation = publishedRotation.rotation;
  publishedRotation.referredTransform = &value;
  return COMPOSITE(PUBLISH(position),
                   Editor::_Publication::AddRange(Editor::Publishable<decltype(publishedRotation.rotation)>::Publish(
                                                      publishedRotation.rotation, "rotation"),
                                                  -(float)3.14159265359, (float)3.14159265359, 0.001f),
                   PUBLISH_RANGE(scale, 0.01f, 100.0f, 0.01f));
}

template <> constexpr char const *Publishable<Engine::Graphics::MeshRenderer>::typeLabel = "MeshRenderer";
template <>
Publication Publishable<Engine::Graphics::MeshRenderer>::Publish(Engine::Graphics::MeshRenderer &value,
                                                                 const char *label) {
  return COMPOSITE();
}

template <> constexpr char const *Publishable<Engine::Graphics::Camera>::typeLabel = "Camera";
template <>
Publication Publishable<Engine::Graphics::Camera>::Publish(Engine::Graphics::Camera &value, const char *label) {
  return COMPOSITE();
}

template <> constexpr char const *Publishable<Editor::Display>::typeLabel = "Display";
template <> Publication Publishable<Editor::Display>::Publish(Editor::Display &value, const char *label) {
  return Publication{.label = value.label.data(), .type = Publication::Type::TEXT, .referencedPointer = &value.label};
}

template <> inline constexpr char const *Publishable<Engine::Core::Component *>::typeLabel = nullptr;
template <>
inline Publication Publishable<Engine::Core::Component *>::Publish(Engine::Core::Component *&value, const char *label) {
  if (auto transform = dynamic_cast<Engine::Graphics::Transform *>(value)) {
    return Publishable<Engine::Graphics::Transform>::Publish(*transform, label ? label : "Transform");
  } else if (auto meshRenderer = dynamic_cast<Engine::Graphics::MeshRenderer *>(value)) {
    return Publishable<Engine::Graphics::MeshRenderer>::Publish(*meshRenderer, label ? label : "MeshRenderer");
  } else if (auto camera = dynamic_cast<Engine::Graphics::Camera *>(value)) {
    return Publishable<Engine::Graphics::Camera>::Publish(*camera, label ? label : "Camera");
  } else if (auto display = dynamic_cast<Editor::Display *>(value)) {
    return Publishable<Editor::Display>::Publish(*display, label ? label : "Display");
  } else {
    return Publication{.label = "Unknown component", .type = Publication::Type::TEXT};
  }
}
} // namespace Editor
