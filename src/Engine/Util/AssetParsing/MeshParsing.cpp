#include "AssetManager.h"
#include "Debug/Logging.h"
#include "Debug/Profiling.h"
#include "Game.h"

namespace Engine {

// OBJ Mesh Parsing

struct OBJVertex {
  Maths::Vector3 position;
  Maths::Vector2 uv;
  Maths::Vector3 normal;
};

struct Token {
  enum class Type {
    OBJECT_NAME,
    SMOOTHING_GROUP,
    VERTEX_POSITION,
    VERTEX_NORMAL,
    VERTEX_TEXTURE_COORDINATES,
    FACE,
    INTEGER,
    FLOAT,
    STRING,
    SLASH,
    EOL,
    EOI,
    ERROR
  } type;
  void *data;
};

Token ErrorToken(const char *message) {
  Token error{Token::Type::ERROR, malloc(strlen(message) + 1)};
  strcpy((char *)error.data, message);
  return error;
}

inline const uint8_t INTEGRAL = 2;
inline const uint8_t NUMERIC = 1;
inline const uint8_t ALPHABETIC = 0;

// Could be made much more sophisticated
template <typename InputIterator> InputIterator tokenizeObj(InputIterator begin, InputIterator end, Token &out) {
  // Skip comments and unsupported object types
  while (begin != end && *begin == '#') {
    while (*begin++ != '\n' && begin != end) {
    }
  }

  // Skip initial whitespace
  while (begin != end && (*begin == ' ' || *begin == '\r' || *begin == '\n')) {
    begin++;
  }

  if (begin == end) {
    out = Token{Token::Type::EOI, nullptr};
    return end;
  }

  if (*begin == '/') {
    out = Token{Token::Type::SLASH, nullptr};
    return ++begin;
  }

  std::array<char, 16> buffer{0};
  uint8_t bufferIndex = 0;
  uint8_t type = INTEGRAL;

  while (begin != end && *begin != ' ' && *begin != '\r' && *begin != '\n' && *begin != '/') {

    if (*begin > '9') {
      type = ALPHABETIC;
    } else if (*begin < '0') {
      if (*begin == '.') {
        type = NUMERIC;
      } else {
        type = ALPHABETIC;
      }
    }

    buffer[bufferIndex++] = *begin++;
  }

  if (out.data) {
    free(out.data);
  }

  switch (type) {
  case INTEGRAL:
    out = Token{Token::Type::INTEGER, malloc(sizeof(uint32_t))};
    *(uint32_t *)out.data = atoi(buffer.data());
    break;
  case NUMERIC:
    out = Token{Token::Type::FLOAT, malloc(sizeof(float))};
    *(float *)out.data = atof(buffer.data());
    break;
  case ALPHABETIC:
    if (memcmp(buffer.data(), "v", bufferIndex) == 0) {
      out = Token{Token::Type::VERTEX_POSITION, nullptr};
    } else if (memcmp(buffer.data(), "vn", bufferIndex) == 0) {
      out = Token{Token::Type::VERTEX_NORMAL, nullptr};
    } else if (memcmp(buffer.data(), "vt", bufferIndex) == 0) {
      out = Token{Token::Type::VERTEX_TEXTURE_COORDINATES, nullptr};
    } else if (memcmp(buffer.data(), "f", bufferIndex) == 0) {
      out = Token{Token::Type::FACE, nullptr};
    } else if (memcmp(buffer.data(), "o", bufferIndex) == 0) {
      out = Token{Token::Type::OBJECT_NAME, nullptr};
    } else if (memcmp(buffer.data(), "s", bufferIndex) == 0) {
      out = Token{Token::Type::SMOOTHING_GROUP, nullptr};
    } else {
      out = Token{Token::Type::STRING, malloc(bufferIndex)};
      memcpy((char *)out.data, buffer.data(), bufferIndex);
    }
    break;
  }

  return begin;
}

template <> struct AssetManager::AssetDSO<Graphics::AllocatedMesh *> {
  std::vector<Maths::Vector3> vertexPositions;
  std::vector<Maths::Vector3> vertexNormals;
  std::vector<Maths::Vector2> vertexUVs;
  std::vector<Maths::VectorT<3, Maths::VectorT<3, uint32_t>>> triangles;
};

template <> std::string AssetManager::GetAssetPath<Graphics::AllocatedMesh *>(char const *assetName) const {
  return std::string("meshes/") + assetName + ".obj";
}

enum class ObjParserState {
  IDLE,
  READ_VERTEX_POSITION,
  READ_VERTEX_NORMAL,
  READ_VERTEX_TEXTURE_COORDINATES,
  READ_TRIANGLE,
  READ_TRIANGLE_VERTEX,
  GOBBLE_ONE
};

template <>
AssetManager::AssetDSO<Graphics::AllocatedMesh *> *
AssetManager::ParseAsset<Graphics::AllocatedMesh *>(std::string const &assetSource) const {
  auto begin = assetSource.begin();
  Token currentToken{};
  uint8_t itemsRead = 0;

  float fBuffer[4];
  uint8_t fBufferIndex = 0;

  Maths::VectorT<3, Maths::VectorT<3, uint32_t>> tBuffer;
  uint8_t vBufferIndex = 0;
  uint8_t tBufferIndex = 0;

  auto state = ObjParserState::IDLE;
  AssetManager::AssetDSO<Graphics::AllocatedMesh *> *dso = new AssetManager::AssetDSO<Graphics::AllocatedMesh *>();

  bool readNewToken = true;

  while (currentToken.type != Token::Type::EOI) {
    if (readNewToken) {
      begin = tokenizeObj(begin, assetSource.end(), currentToken);
    }
    readNewToken = true;

    switch (state) {
    case ObjParserState::IDLE:
      switch (currentToken.type) {
      case Token::Type::VERTEX_POSITION:
        state = ObjParserState::READ_VERTEX_POSITION;
        fBufferIndex = 0;
        break;

      case Token::Type::VERTEX_NORMAL:
        state = ObjParserState::READ_VERTEX_NORMAL;
        fBufferIndex = 0;
        break;

      case Token::Type::VERTEX_TEXTURE_COORDINATES:
        state = ObjParserState::READ_VERTEX_TEXTURE_COORDINATES;
        fBufferIndex = 0;
        break;

      case Token::Type::FACE:
        state = ObjParserState::READ_TRIANGLE;
        tBufferIndex = 0;
        vBufferIndex = 0;
        break;
      case Token::Type::OBJECT_NAME:
      case Token::Type::SMOOTHING_GROUP:
        state = ObjParserState::GOBBLE_ONE;
        break;
      case Token::Type::EOI:
        break;
      default:
        ENGINE_ERROR("Unexpected token while in idle state!");
        break;
      }
      break;

    case ObjParserState::GOBBLE_ONE:
      state = ObjParserState::IDLE;
      break;

    case ObjParserState::READ_VERTEX_POSITION:
      if (currentToken.type == Token::Type::FLOAT || currentToken.type == Token::Type::INTEGER) {
        fBuffer[fBufferIndex++] = *(float *)currentToken.data;
        if (fBufferIndex == 3) {
          state = ObjParserState::IDLE;
          dso->vertexPositions.push_back(Maths::Vector3{fBuffer[0], fBuffer[1], fBuffer[2]});
        }
      } else {
        ENGINE_ERROR("Unexpected token while reading vertex position!");
      }
      break;

    case ObjParserState::READ_VERTEX_NORMAL:
      if (currentToken.type == Token::Type::FLOAT || currentToken.type == Token::Type::INTEGER) {
        fBuffer[fBufferIndex++] = *(float *)currentToken.data;
        if (fBufferIndex == 3) {
          state = ObjParserState::IDLE;
          dso->vertexNormals.push_back(Maths::Vector3{fBuffer[0], fBuffer[1], fBuffer[2]});
        }
      } else {
        ENGINE_ERROR("Unexpected token while reading vertex normal!");
      }
      break;

    case ObjParserState::READ_VERTEX_TEXTURE_COORDINATES:
      if (currentToken.type == Token::Type::FLOAT || currentToken.type == Token::Type::INTEGER) {
        fBuffer[fBufferIndex++] = *(float *)currentToken.data;
        if (fBufferIndex == 2) {
          state = ObjParserState::IDLE;
          dso->vertexUVs.push_back(Maths::Vector2{fBuffer[0], fBuffer[1]});
        }
      } else {
        ENGINE_ERROR("Unexpected token while reading vertex texture coordinates!");
      }
      break;

    case ObjParserState::READ_TRIANGLE_VERTEX:
      if (currentToken.type == Token::Type::SLASH) {
        state = ObjParserState::READ_TRIANGLE;
        break;
      }
      if (currentToken.type != Token::Type::INTEGER) {
        state = ObjParserState::IDLE;
        readNewToken = false;
      }
      for (int i = vBufferIndex; i < 3; i++) {
        tBuffer[tBufferIndex][vBufferIndex++] = 0;
      }
      vBufferIndex = 0;
      tBufferIndex++;
    case ObjParserState::READ_TRIANGLE:
      if (tBufferIndex == 3) {
        dso->triangles.push_back(tBuffer);
        state = ObjParserState::IDLE;
        break;
      }
      if (currentToken.type == Token::Type::INTEGER) {
        tBuffer[tBufferIndex][vBufferIndex++] = *(uint32_t *)currentToken.data;
        state = ObjParserState::READ_TRIANGLE_VERTEX;
      } else {
        ENGINE_ERROR("Unexpected token while reading triangle!");
      }
      break;
    }
  }

  return dso;
}

inline Graphics::MeshT<OBJVertex>
DeduplicateVertices(AssetManager::AssetDSO<Graphics::AllocatedMesh *> const *parsedOBJ) {
  Graphics::MeshT<OBJVertex> objMesh;
  std::unordered_map<Maths::VectorT<3, uint32_t>, uint32_t> vertexIndices{};

  for (auto const &triangle : parsedOBJ->triangles) {
    for (int v = 0; v < 3; v++) {
      auto const &idTriple = triangle[v];
      if (vertexIndices.find(idTriple) == vertexIndices.end()) {
        vertexIndices.emplace(idTriple, static_cast<uint32_t>(objMesh.vertices.size()));
        objMesh.vertices.push_back(OBJVertex{.position = parsedOBJ->vertexPositions[idTriple[0] - 1],
                                             .uv = parsedOBJ->vertexUVs[idTriple[1] - 1],
                                             .normal = parsedOBJ->vertexNormals[idTriple[2] - 1]});
      }
      objMesh.indices.push_back(vertexIndices[idTriple]);
    }
  }
  return objMesh;
}

inline Graphics::Mesh CalculateTangentSpace(Graphics::MeshT<OBJVertex> &objMesh) {
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

    Maths::Matrix3 test = mat * invMat;

    Maths::Vector3 T = (invMat * Maths::Vector3(objMesh.vertices[i1].uv[X] - objMesh.vertices[i0].uv[X],
                                                objMesh.vertices[i2].uv[X] - objMesh.vertices[i0].uv[X], 0))
                           .Normalized();
    Maths::Vector3 B = (invMat * Maths::Vector3(objMesh.vertices[i1].uv[Y] - objMesh.vertices[i0].uv[Y],
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
    result.vertices[v].TBN = Maths::Matrix3(T[X], B[X], N[X], T[Y], B[Y], N[Y], T[Z], B[Z], N[Z]);
  }

  return result;
}

template <>
Graphics::AllocatedMesh *
AssetManager::ConvertDSO<Graphics::AllocatedMesh *>(AssetDSO<Graphics::AllocatedMesh *> const *dso) {
  auto objMesh = DeduplicateVertices(dso);
  auto const &mesh = CalculateTangentSpace(objMesh);
  return new Graphics::AllocatedMesh(
      game->gpuObjectManager.AllocateMesh<Graphics::Vertex, Graphics::VertexFormat>(mesh));
}

template <> void AssetManager::DestroyAsset<Graphics::AllocatedMesh *>(Graphics::AllocatedMesh *&asset) const {
  game->gpuObjectManager.DeallocateMesh(asset);
}

} // namespace Engine