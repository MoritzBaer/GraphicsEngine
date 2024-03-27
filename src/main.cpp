#include "Engine/Engine.h"

#include "Editor/EntityDetails.h"
#include "Engine/AssetManager.h"
#include "Engine/Core/SceneHierarchy.h"
#include "Engine/Editor/Display.h"
#include "Engine/Editor/EntityDetails.h"
#include "Util/Deserializers/TransformParser.h"
#include "Util/FileIO.h"
#include "Util/Parsing.h"

int main() {

  Engine::Init("Test project");

  // TODO: Remove (dummy scene for testing)
  auto monke2 = Engine::AssetManager::LoadPrefab("suzanne.obj");
  monke2.GetComponent<Engine::Editor::Display>()->label = "monke 2";
  auto monke21 = Engine::AssetManager::LoadPrefab("suzanne.obj");
  monke21.GetComponent<Engine::Editor::Display>()->label = "monke 2.1";
  monke21.GetComponent<Engine::Graphics::Transform>()->SetParent(monke2);

  Engine::Util::RegisterComponentParser("Transform", Engine::Util::Deserializers::ParseTransform);

  std::stringstream ss;
  monke2.Serialize(ss);
  Engine::Util::FileIO::WriteFile("test.txt", ss.str());

  auto serialization = Engine::Util::FileIO::ReadFile("test.txt");
  const char *parseData = serialization.data();
  auto monke3 = Engine::Util::ParseEntity(parseData);
  monke3.AddComponent<Engine::Editor::Display>()->label = "monke 3";
  Engine::Core::SceneHierarchy::BuildHierarchy();

  Engine::Editor::SceneView sceneView{};
  Engine::Editor::EntityDetails entityDetailView{sceneView};

  Engine::RunMainLoop();
  Engine::Cleanup();

  return 0;
}