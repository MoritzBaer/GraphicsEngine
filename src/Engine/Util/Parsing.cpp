#include "Parsing.h"

#include "Debug/Logging.h"
#include "Debug/Profiling.h"
#include "Macros.h"
#include "ParsingSubfunctions.h"
#include <unordered_map>
#include <vector>
struct IndexTriple {
  int64_t pos, uv, normal;
  inline bool operator==(IndexTriple const &other) const {
    return pos == other.pos && uv == other.uv && normal == other.normal;
  }
};

namespace std {
template <> struct hash<IndexTriple> {
  inline size_t operator()(IndexTriple const &t) const {
    return hash<size_t>{}(hash<int64_t>{}(t.pos) + hash<int64_t>{}(t.uv) + hash<int64_t>{}(t.normal));
  }
};
} // namespace std

namespace Engine::Util {

struct OBJVertex {
  Maths::Vector3 position;
  Maths::Vector2 uv;
  Maths::Vector3 normal;
};

inline std::unordered_map<std::string, std::function<void(Core::Entity const &, char const *&)>> componentParsers{};

enum class ObjParsingState {
  BEGIN_LINE,
  GOBBLE_LINE,
  GOBBLE_SPACE,
  GOBBLE_CHARACTER,
  READ_FLOAT,
  READ_INT,
  READ_INDEX_TRIPLE,
  RESOLVE_INDEX_TRIPLE,
  READ_VERTEX_POSITION,
  READ_VERTEX_UV,
  READ_VERTEX_NORMAL,
  READ_FACE
};

Graphics::Mesh CalculateTangentSpace(Graphics::MeshT<OBJVertex, OBJVertex> &objMesh) {
  PROFILE_FUNCTION()

  std::vector<uint16_t> triangleParticipations(objMesh.vertices.size(), 0);
  std::vector<Maths::Vector3> cotangents(objMesh.vertices.size(), Maths::Vector3({0, 0, 0}));
  std::vector<Maths::Vector3> cobitangents(objMesh.vertices.size(), Maths::Vector3({0, 0, 0}));

  for (int i = 0; i < objMesh.indices.size(); i += 3) {

    auto i0 = objMesh.indices[i];
    auto i1 = objMesh.indices[i + 1];
    auto i2 = objMesh.indices[i + 2];

    Maths::Vector3 p0 = objMesh.vertices[i0].position;
    Maths::Vector3 p1 = objMesh.vertices[i1].position;
    Maths::Vector3 p2 = objMesh.vertices[i2].position;

    Maths::Vector3 q1 = p1 - p0;
    Maths::Vector3 q2 = p2 - p0;

    Maths::Vector3 N = q1.Cross(q2).Normalized();

    Maths::Matrix3 mat = Maths::Matrix3(q1[X], q1[Y], q1[Z], q2[X], q2[Y], q2[Z], N[X], N[Y], N[Z]);
    Maths::Matrix3 invMat = mat.Inverse();

    Maths::Vector3 T = (invMat * Maths::Vector3(objMesh.vertices[i1].uv[X] - objMesh.vertices[i0].uv[X],
                                                objMesh.vertices[i1].uv[Y] - objMesh.vertices[i0].uv[Y], 0))
                           .Normalized();
    Maths::Vector3 B = (invMat * Maths::Vector3(objMesh.vertices[i2].uv[X] - objMesh.vertices[i0].uv[X],
                                                objMesh.vertices[i2].uv[Y] - objMesh.vertices[i0].uv[Y], 0))
                           .Normalized();

    for (int j = 0; j < 3; j++) {
      triangleParticipations[objMesh.indices[i + j]]++;
      cotangents[objMesh.indices[i + j]] += T;
      cobitangents[objMesh.indices[i + j]] += B;
    }
  }

  Graphics::Mesh result{};
  result.indices.swap(objMesh.indices);
  result.vertices.resize(objMesh.vertices.size());
  for (int v = 0; v < objMesh.vertices.size(); v++) {
    result.vertices[v].position = objMesh.vertices[v].position;
    result.vertices[v].uv = objMesh.vertices[v].uv;
    Maths::Vector3 T = cotangents[v] / triangleParticipations[v];
    Maths::Vector3 B = cobitangents[v] / triangleParticipations[v];
    Maths::Vector3 N = objMesh.vertices[v].normal;
    result.vertices[v].TBN = Maths::Matrix3(T[X], T[Y], T[Z], B[X], B[Y], B[Z], N[X], N[Y], N[Z]);
    // result.vertices[v].TBN = Maths::Matrix3(N[X], N[Y], N[Z], B[0], B[1], B[2], N[0], N[1], N[2]);
  }

  return result;
}

Graphics::Mesh ParseOBJ(char const *charStream) {
  PROFILE_FUNCTION()
  std::vector<ObjParsingState> stateStack{ObjParsingState::BEGIN_LINE};

  std::vector<Maths::Vector3> vertexPositions;
  std::vector<Maths::Vector2> vertexUVs;
  std::vector<Maths::Vector3> vertexNormals;

  std::vector<float> floatBuffer;
  std::vector<uint32_t> intBuffer;
  std::vector<char> charBuffer;

  std::unordered_map<IndexTriple, uint32_t> vertexIndices{};
  IndexTriple indexTriple;

  Graphics::MeshT<OBJVertex, OBJVertex> objMesh;

  char currentSymbol;
  while (currentSymbol = *charStream) {
    auto state = stateStack.back();
    stateStack.pop_back();
    switch (state) {
    case ObjParsingState::BEGIN_LINE:
      switch (currentSymbol) {
      case 'v':
        if (*(charStream + 1) == 't') { // Read vertex texture
          charStream += 2;
          stateStack.push_back(ObjParsingState::GOBBLE_LINE);
          stateStack.push_back(ObjParsingState::READ_VERTEX_UV);
          stateStack.push_back(ObjParsingState::READ_FLOAT);
          stateStack.push_back(ObjParsingState::GOBBLE_SPACE);
          stateStack.push_back(ObjParsingState::READ_FLOAT);
          stateStack.push_back(ObjParsingState::GOBBLE_SPACE);
        } else if (*(charStream + 1) == 'n') { // Read vertex normal
          charStream += 2;
          stateStack.push_back(ObjParsingState::GOBBLE_LINE);
          stateStack.push_back(ObjParsingState::READ_VERTEX_NORMAL);
          stateStack.push_back(ObjParsingState::READ_FLOAT);
          stateStack.push_back(ObjParsingState::GOBBLE_SPACE);
          stateStack.push_back(ObjParsingState::READ_FLOAT);
          stateStack.push_back(ObjParsingState::GOBBLE_SPACE);
          stateStack.push_back(ObjParsingState::READ_FLOAT);
          stateStack.push_back(ObjParsingState::GOBBLE_SPACE);
        } else if (*(charStream + 1) == ' ') { // Read vertex Position
          charStream += 2;
          stateStack.push_back(ObjParsingState::GOBBLE_LINE);
          stateStack.push_back(ObjParsingState::READ_VERTEX_POSITION);
          stateStack.push_back(ObjParsingState::READ_FLOAT);
          stateStack.push_back(ObjParsingState::GOBBLE_SPACE);
          stateStack.push_back(ObjParsingState::READ_FLOAT);
          stateStack.push_back(ObjParsingState::GOBBLE_SPACE);
          stateStack.push_back(ObjParsingState::READ_FLOAT);
          stateStack.push_back(ObjParsingState::GOBBLE_SPACE);
        } else {
          // Unsupported keyword
          stateStack.push_back(ObjParsingState::GOBBLE_LINE);
        }
        break;
      case 'f':
        charStream++;
        stateStack.push_back(ObjParsingState::GOBBLE_LINE);
        stateStack.push_back(ObjParsingState::READ_FACE);
        stateStack.push_back(ObjParsingState::READ_INDEX_TRIPLE);
        stateStack.push_back(ObjParsingState::READ_INDEX_TRIPLE);
        stateStack.push_back(ObjParsingState::READ_INDEX_TRIPLE);

        break;

      case '#': // Comment
      default:  // Unsupported keyword
        stateStack.push_back(ObjParsingState::GOBBLE_LINE);
        break;
      }
      break;
    case ObjParsingState::GOBBLE_SPACE:
      if (currentSymbol == ' ') {
        stateStack.push_back(ObjParsingState::GOBBLE_SPACE);
        charStream++;
      }
      break;
    case ObjParsingState::GOBBLE_LINE:
      if (currentSymbol != '\n') {
        stateStack.push_back(ObjParsingState::GOBBLE_LINE);
        charStream++;
      } else {
        stateStack.push_back(ObjParsingState::BEGIN_LINE), charStream++;
      }
      break;
    case ObjParsingState::READ_FLOAT:
      if (isDigit(currentSymbol) || currentSymbol == '.' || currentSymbol == '-') {
        stateStack.push_back(ObjParsingState::READ_FLOAT);
        charBuffer.push_back(currentSymbol);
        charStream++;
      } else {
        charBuffer.push_back(0);
        floatBuffer.push_back(std::atof(charBuffer.data()));
        charBuffer = {};
      }
      break;
    case ObjParsingState::READ_INT:
      if (isDigit(currentSymbol) || currentSymbol == '-') {
        stateStack.push_back(ObjParsingState::READ_INT);
        charBuffer.push_back(currentSymbol);
        charStream++;
      } else {
        charBuffer.push_back(0);
        intBuffer.push_back(std::atoi(charBuffer.data()));
        charBuffer = {};
      }
      break;
    case ObjParsingState::READ_VERTEX_POSITION: // floatBuffer should contain exactly 3 floats now
      vertexPositions.push_back(Maths::Vector3({floatBuffer[0], floatBuffer[1], floatBuffer[2]}));
      floatBuffer = {};
      break;
    case ObjParsingState::READ_VERTEX_NORMAL: // floatBuffer should contain exactly 3 floats now
      vertexNormals.push_back(Maths::Vector3({floatBuffer[0], floatBuffer[1], floatBuffer[2]}));
      floatBuffer = {};
      break;
    case ObjParsingState::READ_VERTEX_UV: // floatBuffer should contain exactly 2 floats now
      vertexUVs.push_back(Maths::Vector2({floatBuffer[0], floatBuffer[1]}));
      floatBuffer = {};
      break;
    case ObjParsingState::READ_FACE:
      break;
    case ObjParsingState::READ_INDEX_TRIPLE:
      stateStack.push_back(ObjParsingState::RESOLVE_INDEX_TRIPLE);
      stateStack.push_back(ObjParsingState::READ_INT);
      stateStack.push_back(ObjParsingState::GOBBLE_CHARACTER);
      stateStack.push_back(ObjParsingState::READ_INT);
      stateStack.push_back(ObjParsingState::GOBBLE_CHARACTER);
      stateStack.push_back(ObjParsingState::READ_INT);
      stateStack.push_back(ObjParsingState::GOBBLE_SPACE);
      break;
    case ObjParsingState::RESOLVE_INDEX_TRIPLE:
      indexTriple = {intBuffer[0], intBuffer[1], intBuffer[2]};
      if (vertexIndices.find(indexTriple) == vertexIndices.end()) {
        vertexIndices.emplace(indexTriple, static_cast<uint32_t>(objMesh.vertices.size()));
        objMesh.vertices.push_back(OBJVertex{.position = vertexPositions[indexTriple.pos - 1],
                                             .uv = vertexUVs[indexTriple.uv - 1],
                                             .normal = vertexNormals[indexTriple.normal - 1]});
      }
      objMesh.indices.push_back(vertexIndices.at(indexTriple));
      intBuffer = {};
      break;
    case ObjParsingState::GOBBLE_CHARACTER:
      charStream++;
      break;
    default:
      break;
    }
  }

  return CalculateTangentSpace(objMesh);
}

enum class EntityParsingState {
  GOBBLE_LINE,
  GOBBLE_SPACE,
  GOBBLE_CHARACTER,
};

Core::Entity ParseEntity(const char *&charStream) {
  SkipWhitespace(charStream);

  Core::Entity e = ENGINE_NEW_ENTITY();

  PARSE_BLOCK(charStream,
              FIRST_TOKEN_REACTION(
                  "Components",
                  PARSE_ARRAY(
                      charStream,
                      // Read component type to buffer
                      ReadTokenToBuffer(charStream, tokenBuffer, sizeof(tokenBuffer) / sizeof(uint8_t));
                      if (componentParsers.find(tokenBuffer) != componentParsers.end()) {
                        componentParsers[tokenBuffer](e, charStream);
                      } else {
                        ENGINE_WARNING(
                            "Component '{}' found in entity serialization, but no deserializer for {} was registered!",
                            tokenBuffer, tokenBuffer);
                      },
                      "component")),
              "Entity")

  // TODO: Returns valid (but empty) entity even if parsing fails. Figure out if this is desirable.
  return e;
}

std::vector<Core::Entity> ParseEntityArray(char const *&charStream) {
  std::vector<Core::Entity> result;
  PARSE_ARRAY(charStream, result.push_back(ParseEntity(charStream)), "entity")
  return result;
}

std::vector<float> ParseFloatArray(char const *&charStream) {
  std::vector<float> result;
  PARSE_VALUE_LIST(charStream, result.push_back(ReadFloat(charStream)), "float array")
  return result;
}

Maths::Vector3 ParseVector3(char const *&charStream) {
  auto valueArray = ParseFloatArray(charStream);
  ENGINE_ASSERT(valueArray.size() == 3, "Expected 3 values in Vector3!");
  return {valueArray[0], valueArray[1], valueArray[2]};
}

Maths::Quaternion ParseQuaternion(char const *&charStream) {
  Maths::Quaternion result{};

  PARSE_BLOCK(charStream,
              FIRST_TOKEN_REACTION("x", result.x = ReadFloat(charStream))
                  LATER_TOKEN_REACTION("y", result.y = ReadFloat(charStream))
                      LATER_TOKEN_REACTION("z", result.z = ReadFloat(charStream))
                          LATER_TOKEN_REACTION("w", result.w = ReadFloat(charStream)),
              "quaternion")
  return result;
}

void RegisterComponentParser(const char *identifier, std::function<void(Core::Entity const &, char const *&)> parser) {
  componentParsers.emplace(identifier, parser);
}

} // namespace Engine::Util