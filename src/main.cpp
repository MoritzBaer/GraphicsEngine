#include "Engine/Engine.h"

#include "Editor/EntityDetails.h"
#include "Engine/AssetManager.h"
#include "Engine/Core/SceneHierarchy.h"
#include "Engine/Editor/Display.h"
#include "Engine/Editor/EntityDetails.h"
#include "Engine/Graphics/Texture.h"
#include "Util/Deserializers/DisplayParser.h"
#include "Util/Deserializers/MeshRendererParser.h"
#include "Util/Deserializers/TransformParser.h"
#include "Util/FileIO.h"
#include "Util/Parsing.h"

int main() {

  Engine::Init("Test project");

  Engine::Util::RegisterComponentParser("Transform", Engine::Util::Deserializers::ParseTransform);
  Engine::Util::RegisterComponentParser("MeshRenderer", Engine::Util::Deserializers::ParseMeshRenderer);
  Engine::Util::RegisterComponentParser("Display", Engine::Util::Deserializers::ParseDisplay);

  Engine::AssetManager::LoadPrefab("generator.pfb");

  Engine::Editor::SceneView sceneView{};
  Engine::Editor::EntityDetails entityDetailView{sceneView};

  Engine::RunMainLoop();
  Engine::Cleanup();

  return 0;
}