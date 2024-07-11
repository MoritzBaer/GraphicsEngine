#include "Engine/Engine.h"

#include "Debug/Profiling.h"
#include "Editor/EntityDetails.h"
#include "Engine/AssetManager.h"
#include "Engine/Core/SceneHierarchy.h"
#include "Engine/Editor/Display.h"
#include "Engine/Editor/EntityDetails.h"
#include "Engine/Graphics/Texture.h"
#include "Util/FileIO.h"
#include "Util/Parsing.h"
#include "json-parsing.h"

int main() {

  Engine::Init("Test project");

  BEGIN_PROFILE_SESSION()
  Engine::AssetManager::LoadPrefab("speeder.pfb");
  WRITE_PROFILE_SESSION("Scene-Loading")

  Engine::Editor::SceneView sceneView{};
  Engine::Editor::EntityDetails entityDetailView{sceneView};

  Engine::RunMainLoop();
  Engine::Cleanup();

  return 0;
}