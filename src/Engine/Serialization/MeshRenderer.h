#pragma once

#include "Game.h"
#include "Graphics/MeshRenderer.h"

OBJECT_PARSER(
    Engine::Graphics::MeshRenderer,
    if (key == "mesh") {
      std::string value;
      begin = json<std::string>::parse_tokenstream(begin, end, value, context);
      output.mesh = ((Game *)context)->assetManager.LoadMesh(value.c_str());
    } else if (key == "material") {
      std::string value;
      begin = json<std::string>::parse_tokenstream(begin, end, value, context);
      output.material = ((Game *)context)->assetManager.LoadMaterial(value.c_str());
    } else)