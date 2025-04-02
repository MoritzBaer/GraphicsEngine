#pragma once

#include "Core/Script.h"
#include "Graphics/Transform.h"
#include "Util/AssetParsing/ScriptParsing.h"
#include "json-parsing.h"

#define USER_SCRIPTS Demo::SpinnyScriptDSO, Demo::BobbyScriptDSO

namespace Demo {

struct SpinnyScript : public Engine::Core::Script {
  Engine::Graphics::Transform *transform;
  float rotationSpeed = 1.0f;

  SpinnyScript(Engine::Core::Entity entity, float rotationSpeed = 1.0f)
      : Engine::Core::Script(entity), rotationSpeed(rotationSpeed), transform(nullptr) {}
  void OnCreate() override { transform = entity.GetComponent<Engine::Graphics::Transform>(); }

  void OnUpdate(Engine::Core::Clock const &clock) override {
    Engine::Maths::Vector3 oldRotation = transform->rotation.EulerAngles();
    transform->rotation = (Engine::Maths::Transformations::RotateAroundAxis(Engine::Maths::Vector3(0, 1, 0),
                                                                            rotationSpeed * clock.deltaTime * 0.2f) *
                           transform->rotation)
                              .Normalized();
  }

  inline void Clone(Engine::Core::ScriptComponent *target) override {
    target->InstantiateScript<SpinnyScript>(rotationSpeed);
  }
};

struct SpinnyScriptDSO : public Engine::ScriptDSO {
  float rotationSpeed;
  void Attach(Engine::Core::ScriptComponent *scriptComponent, Engine::AssetManager *) override {
    scriptComponent->InstantiateScript<SpinnyScript>(rotationSpeed);
  }
};

struct BobbyScript : public Engine::Core::Script {
  Engine::Graphics::Transform *transform;
  float initialY;
  float bobbingAmplitude;

  BobbyScript(Engine::Core::Entity entity, float bobbingAmplitude)
      : Engine::Core::Script(entity), bobbingAmplitude(bobbingAmplitude), transform(nullptr) {}

  void OnStart() override { initialY = transform->position.y(); }
  void OnCreate() override { transform = entity.GetComponent<Engine::Graphics::Transform>(); }
  void OnUpdate(Engine::Core::Clock const &clock) override {
    transform->position.y() = initialY + std::sin(clock.time) * bobbingAmplitude;
  }

  void Clone(Engine::Core::ScriptComponent *target) override {
    target->InstantiateScript<BobbyScript>(bobbingAmplitude);
  }
};

struct BobbyScriptDSO : public Engine::ScriptDSO {
  float bobbingAmplitude;
  void Attach(Engine::Core::ScriptComponent *scriptComponent, Engine::AssetManager *) override {
    scriptComponent->InstantiateScript<BobbyScript>(bobbingAmplitude);
  }
};

} // namespace Demo

JSON(Demo::SpinnyScriptDSO, FIELDS(rotationSpeed));
JSON(Demo::BobbyScriptDSO, FIELDS(bobbingAmplitude));