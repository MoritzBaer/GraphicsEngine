#include "AssetManager.h"

#include "Util/FileIO.h"
#include "Debug/Profiling.h"

#include <stdlib.h>
#include <unordered_map>
#include <functional>

#define RESOURCE_PATH "res/"
#define SHADER_PATH "shaders/"
#define MESH_PATH "meshes/"

#define CONSTRUCT_PATHS(a, b) a b

#define _RETURN_ASSET(key, table, constructor)                                                                  \
    if (instance->table.find(key) == instance->table.end()) { instance->table.insert({key, constructor}); }     \
    return instance->table.at(key);

namespace Engine
{
    struct IndexTriple {
        int64_t pos, uv, normal;
        inline bool operator== (IndexTriple const & other) const { return pos == other.pos && uv == other.uv && normal == other.normal; }
    };
} // namespace Engine

namespace std {
    template <>
    struct hash<Engine::IndexTriple> {
        inline size_t operator() (Engine::IndexTriple const& t) const {
            return hash<size_t>{}(hash<int64_t>{}(t.pos) + hash<int64_t>{}(t.uv) + hash<int64_t>{}(t.normal));
        }
    };
}

namespace Engine
{
    AssetManager::AssetManager() : loadedShaders() { }
    AssetManager::~AssetManager() { }
    void AssetManager::Init() { instance = new AssetManager(); }
    void AssetManager::Cleanup() { delete instance; }

    Graphics::Shader AssetManager::LoadShader(char const *shaderName, Graphics::ShaderType shaderType)
    {
        PROFILE_FUNCTION()
        char dirPath[] = CONSTRUCT_PATHS(RESOURCE_PATH, SHADER_PATH);
        char * filePath = static_cast<char *>(malloc(strlen(shaderName) * sizeof(char) + sizeof(dirPath)));
        strcpy(filePath, dirPath);
        strcat(filePath, shaderName); 
        _RETURN_ASSET(shaderName, loadedShaders, Graphics::ShaderCompiler::CompileShaderCode(Util::FileIO::ReadFile(filePath), shaderType))
    }

    Graphics::Shader AssetManager::LoadShaderWithInferredType(char const * shaderName)
    {
        // TODO: Infer shader type from ending of file
        return LoadShader(shaderName, Graphics::ShaderType::COMPUTE);
    }

    
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

    inline bool isDigit(char c) { return c >= '0' && c <= '9'; }

    Graphics::Mesh ParseOBJ(char const * charStream) {
        PROFILE_FUNCTION()
        std::vector<ObjParsingState> stateStack { ObjParsingState::BEGIN_LINE };

        std::vector<Maths::Vector3> vertexPositions;
        std::vector<Maths::Vector2> vertexUVs;
        std::vector<Maths::Vector3> vertexNormals;

        std::vector<float> floatBuffer;
        std::vector<uint32_t> intBuffer;
        std::vector<char> charBuffer;

        std::unordered_map<IndexTriple, uint32_t> vertexIndices {}; 
        IndexTriple indexTriple;

        Graphics::Mesh resultMesh;

        char currentSymbol;
        while (currentSymbol = *charStream) {
            auto state = stateStack.back();
            stateStack.pop_back();
            switch (state)
            {
            case ObjParsingState::BEGIN_LINE: 
                switch (currentSymbol)
                {
                case 'v':
                    if(*(charStream + 1) == 't') {          // Read vertex texture
                        charStream += 2;
                        stateStack.push_back(ObjParsingState::GOBBLE_LINE);
                        stateStack.push_back(ObjParsingState::READ_VERTEX_UV);
                        stateStack.push_back(ObjParsingState::READ_FLOAT);
                        stateStack.push_back(ObjParsingState::GOBBLE_SPACE);
                        stateStack.push_back(ObjParsingState::READ_FLOAT); 
                        stateStack.push_back(ObjParsingState::GOBBLE_SPACE);
                    } else if (*(charStream + 1) == 'n') {  // Read vertex normal
                        charStream += 2;
                        stateStack.push_back(ObjParsingState::GOBBLE_LINE);
                        stateStack.push_back(ObjParsingState::READ_VERTEX_NORMAL);
                        stateStack.push_back(ObjParsingState::READ_FLOAT);
                        stateStack.push_back(ObjParsingState::GOBBLE_SPACE);
                        stateStack.push_back(ObjParsingState::READ_FLOAT); 
                        stateStack.push_back(ObjParsingState::GOBBLE_SPACE);
                        stateStack.push_back(ObjParsingState::READ_FLOAT);
                        stateStack.push_back(ObjParsingState::GOBBLE_SPACE);
                    } else if (*(charStream + 1) == ' ') {  // Read vertex Position
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

                case '#':   // Comment
                default:    // Unsupported keyword
                    stateStack.push_back(ObjParsingState::GOBBLE_LINE);
                    break;
                }
                break;
            case ObjParsingState::GOBBLE_SPACE: 
                if(currentSymbol == ' ') { stateStack.push_back(ObjParsingState::GOBBLE_SPACE); charStream++; }
                break;
            case ObjParsingState::GOBBLE_LINE: 
                if(currentSymbol != '\n') { stateStack.push_back(ObjParsingState::GOBBLE_LINE); charStream++; }
                else { stateStack.push_back(ObjParsingState::BEGIN_LINE), charStream++; }
                break;
            case ObjParsingState::READ_FLOAT: 
                if(isDigit(currentSymbol) || currentSymbol == '.' || currentSymbol == '-') {
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
                if(isDigit(currentSymbol) || currentSymbol == '-') {
                    stateStack.push_back(ObjParsingState::READ_INT);
                    charBuffer.push_back(currentSymbol);
                    charStream++;
                } else {
                    charBuffer.push_back(0);
                    intBuffer.push_back(std::atoi(charBuffer.data()));
                    charBuffer = {};
                }
                break;
            case ObjParsingState::READ_VERTEX_POSITION:     // floatBuffer should contain exactly 3 floats now
                vertexPositions.push_back(Maths::Vector3({ floatBuffer[0], floatBuffer[1], floatBuffer[2] }));
                floatBuffer = {};
                break;
            case ObjParsingState::READ_VERTEX_NORMAL:       // floatBuffer should contain exactly 3 floats now
                vertexNormals.push_back(Maths::Vector3({ floatBuffer[0], floatBuffer[1], floatBuffer[2] }));
                floatBuffer = {};
                break;
            case ObjParsingState::READ_VERTEX_UV:           // floatBuffer should contain exactly 2 floats now
                vertexUVs.push_back(Maths::Vector2({ floatBuffer[0], floatBuffer[1] }));
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
                indexTriple = { intBuffer[0], intBuffer[1], intBuffer[2] };
                if(vertexIndices.find(indexTriple) == vertexIndices.end()) {
                    vertexIndices.emplace(indexTriple, resultMesh.vertices.size());
                    resultMesh.vertices.push_back(Graphics::Mesh::VertexFormat {
                        .position = vertexPositions[indexTriple.pos - 1],
                        .uv_x = vertexUVs[indexTriple.uv - 1].x(),
                        .normal = vertexNormals[indexTriple.normal - 1],
                        .uv_y = vertexUVs[indexTriple.uv - 1].y(),
                    });
                }
                resultMesh.indices.push_back(vertexIndices.at(indexTriple));
                intBuffer = {};
                break;
            case ObjParsingState::GOBBLE_CHARACTER: 
                charStream++;
                break;
            default:
                break;
            }
        }

        return resultMesh;
    }

    Graphics::Mesh AssetManager::LoadMeshFromOBJ(char const *meshName)
    {
        char dirPath[] = CONSTRUCT_PATHS(RESOURCE_PATH, MESH_PATH);
        char * filePath = static_cast<char *>(malloc(strlen(meshName) * sizeof(char) + sizeof(dirPath)));
        strcpy(filePath, dirPath);
        strcat(filePath, meshName); 
        auto meshData = Util::FileIO::ReadFile(filePath);
        
        return ParseOBJ(meshData.data());
        return Graphics::Mesh();
    }

} // namespace Engine