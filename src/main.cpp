#include "Engine/Engine.h"

#include "Editor/EntityDetails.h"
#include "Engine/AssetManager.h"
#include "Engine/Core/SceneHierarchy.h"
#include "Engine/Editor/Display.h"
#include "Engine/Editor/EntityDetails.h"
#include "Util/Deserializers/DisplayParser.h"
#include "Util/Deserializers/MeshRendererParser.h"
#include "Util/Deserializers/TransformParser.h"
#include "Util/FileIO.h"
#include "Util/Parsing.h"

Engine::Core::Entity dummyEntity() {
  Engine::Core::Entity prefab = ENGINE_NEW_ENTITY();
  prefab.AddComponent<Engine::Graphics::Transform>()->scale = Vector3{0.5, 0.5, 1};
  prefab.AddComponent<Engine::Graphics::MeshRenderer>()->AssignMesh("suzanne.obj");
  prefab.GetComponent<Engine::Graphics::MeshRenderer>()->AssignMaterial("dummy");
  prefab.AddComponent<Engine::Editor::Display>()->AssignLabel("monke");

  Engine::Core::SceneHierarchy::BuildHierarchy();

  return prefab;
}

int main() {

  Engine::Init("Test project");

  Engine::Util::RegisterComponentParser("Transform", Engine::Util::Deserializers::ParseTransform);
  Engine::Util::RegisterComponentParser("MeshRenderer", Engine::Util::Deserializers::ParseMeshRenderer);
  Engine::Util::RegisterComponentParser("Display", Engine::Util::Deserializers::ParseDisplay);

  // TODO: Remove (dummy scene for testing)

  Engine::AssetManager::LoadPrefab("suzanne.pfb");

  // auto monke2 = dummyEntity();
  // auto monke21 = dummyEntity();
  // monke21.GetComponent<Engine::Editor::Display>()->label = "monke 1.1";
  // monke21.GetComponent<Engine::Graphics::Transform>()->SetParent(monke2);

  // std::stringstream ss;
  // monke2.Serialize(ss);
  // Engine::Util::FileIO::WriteFile("test.txt", ss.str());
  //
  // auto serialization = Engine::Util::FileIO::ReadFile("test.txt");
  // const char *parseData = serialization.data();
  // auto monke3 = Engine::Util::ParseEntity(parseData);
  // monke3.AddComponent<Engine::Editor::Display>()->label = "monke 2";
  // Engine::Core::SceneHierarchy::BuildHierarchy();

  Engine::Editor::SceneView sceneView{};
  Engine::Editor::EntityDetails entityDetailView{sceneView};

  Engine::RunMainLoop();
  Engine::Cleanup();

  return 0;
}