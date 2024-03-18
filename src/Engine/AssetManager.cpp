#include "AssetManager.h"

#include "Util/FileIO.h"
#include "Debug/Profiling.h"
#include "Graphics/Transform.h"
#include "Graphics/MeshRenderer.h"
#include "Graphics/TestMaterial.h"

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
    class PipelineBuilder {
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        
        VkPipelineInputAssemblyStateCreateInfo inputAssembly;
        VkPipelineRasterizationStateCreateInfo rasterizer;
        VkPipelineColorBlendAttachmentState colourBlendAttachment;
        VkPipelineMultisampleStateCreateInfo multisampling;
        VkPipelineLayout pipelineLayout;
        VkPipelineDepthStencilStateCreateInfo depthStencil;
        VkPipelineRenderingCreateInfo renderInfo;
        VkFormat colourAttachmentformat;

    public: 
        PipelineBuilder & Reset();

        PipelineBuilder() { Reset(); }

        inline PipelineBuilder & SetPipelineLayout(VkPipelineLayout const & layout); 
        inline PipelineBuilder & SetShaderStages(Graphics::Shader const & vertexShader, Graphics::Shader const & fragmentShader);
        inline PipelineBuilder & SetInputTopology(VkPrimitiveTopology const & topology);
        inline PipelineBuilder & SetPolygonMode(VkPolygonMode const & polygonMode);
        inline PipelineBuilder & SetCullMode(VkCullModeFlags const & cullMode, VkFrontFace const & frontFace);
        inline PipelineBuilder & SetColourAttachmentFormat(VkFormat const & format);
        inline PipelineBuilder & SetDepthFormat(VkFormat const & format);
        inline PipelineBuilder & DisableDepthTest();

        VkPipeline BuildPipeline() const;
        void BuildPipeline(VkPipeline * pipeline) const;
    };
    

    PipelineBuilder & PipelineBuilder::Reset()
    {
        inputAssembly = { 
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .primitiveRestartEnable = VK_FALSE
        };

        rasterizer = { 
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .lineWidth = 1.0f 
        };

        colourBlendAttachment = { };

        multisampling = { 
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.0f,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE
        };

        colourBlendAttachment = {
            .blendEnable = VK_FALSE,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
        };

        pipelineLayout = { };
        depthStencil = { 
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = VK_TRUE,
	        .depthWriteEnable = VK_TRUE,
	        .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
	        .depthBoundsTestEnable = VK_FALSE,
	        .stencilTestEnable = VK_FALSE,
	        .front = {},
	        .back = {},
	        .minDepthBounds = 0.f,
	        .maxDepthBounds= 1.f
        };

        renderInfo = { 
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &colourAttachmentformat
        };

        shaderStages.clear();

        return *this;
    }

    inline PipelineBuilder & PipelineBuilder::SetPipelineLayout(VkPipelineLayout const &layout)
    {
        pipelineLayout = layout;
        return *this;
    }

    inline PipelineBuilder & PipelineBuilder::SetShaderStages(Graphics::Shader const &vertexShader, Graphics::Shader const &fragmentShader)
    {
        shaderStages.clear();

        shaderStages.push_back(vertexShader.GetStageInfo());
        shaderStages.push_back(fragmentShader.GetStageInfo());
        return *this;
    }

    inline PipelineBuilder & PipelineBuilder::SetInputTopology(VkPrimitiveTopology const & topology)
    {
        inputAssembly.topology = topology;
        return *this;
    }

    inline PipelineBuilder & PipelineBuilder::SetPolygonMode(VkPolygonMode const &polygonMode)
    {
        rasterizer.polygonMode = polygonMode;
        return *this;
    }

    inline PipelineBuilder & PipelineBuilder::SetCullMode(VkCullModeFlags const &cullMode, VkFrontFace const &frontFace)
    {
        rasterizer.cullMode = cullMode;
        rasterizer.frontFace = frontFace;
        return *this;
    }

    inline PipelineBuilder & PipelineBuilder::SetColourAttachmentFormat(VkFormat const &format)
    {
        colourAttachmentformat = format;
        return *this;
    }

    inline PipelineBuilder & PipelineBuilder::SetDepthFormat(VkFormat const &format)
    {
        renderInfo.depthAttachmentFormat = format;
        return *this;
    }

    inline PipelineBuilder & PipelineBuilder::DisableDepthTest()
    {
        depthStencil.depthTestEnable = VK_FALSE;
	    depthStencil.depthWriteEnable = VK_FALSE;
	    depthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
	    depthStencil.depthBoundsTestEnable = VK_FALSE;
	    depthStencil.stencilTestEnable = VK_FALSE;
        return *this;
    }

    VkPipeline PipelineBuilder::BuildPipeline() const
    {
        VkPipeline newPipeline;
        BuildPipeline(&newPipeline);
        return newPipeline;
    }

    void PipelineBuilder::BuildPipeline(VkPipeline *pipeline) const
    {
        VkPipelineViewportStateCreateInfo viewportInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1
        };

        VkPipelineColorBlendStateCreateInfo colourBlendInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .attachmentCount = 1,
            .pAttachments = &colourBlendAttachment
        };

        VkPipelineVertexInputStateCreateInfo vertexInputInfo { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

        VkDynamicState states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynamicStateInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = 2,
            .pDynamicStates = states
        };

        VkGraphicsPipelineCreateInfo pipelineInfo {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &renderInfo,
            .stageCount = static_cast<uint32_t>(shaderStages.size()),
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &inputAssembly,
            .pViewportState = &viewportInfo,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pDepthStencilState = &depthStencil,
            .pColorBlendState = &colourBlendInfo,
            .pDynamicState = &dynamicStateInfo,
            .layout = pipelineLayout
        };

        Graphics::InstanceManager::CreateGraphicsPipeline(pipelineInfo, pipeline);
    }

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
                    vertexIndices.emplace(indexTriple, static_cast<uint32_t>(resultMesh.vertices.size()));
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

    Graphics::Material *AssetManager::LoadMaterial(char const *materialName)
    {
        // Dummy implementation // TODO: implement properly
        Graphics::Shader vertexShader = LoadShader("coloured_triangle_mesh.vert", Graphics::ShaderType::VERTEX);
        Graphics::Shader fragmentShader = LoadShader("coloured_triangle.frag", Graphics::ShaderType::FRAGMENT);
        PipelineBuilder builder = PipelineBuilder();
        
        VkPipelineLayout layout;

        size_t uniformSize = sizeof(VkDeviceAddress) + sizeof(Maths::Matrix4) + sizeof(Maths::Vector3);

        VkPushConstantRange uniformInfo {
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .offset = 0,
            .size = static_cast<uint32_t>(uniformSize)
        };

        VkPipelineLayoutCreateInfo layoutInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pushConstantRangeCount = 1,
            .pPushConstantRanges = &uniformInfo
        };

        Graphics::InstanceManager::CreatePipelineLayout(&layoutInfo, &layout);

        VkPipeline pipeline = builder
                            .SetPipelineLayout(layout)
                            .SetShaderStages(vertexShader, fragmentShader)
                            .SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
                            .SetPolygonMode(VK_POLYGON_MODE_FILL)
                            .SetColourAttachmentFormat(VK_FORMAT_R16G16B16A16_SFLOAT)
                            .SetDepthFormat(VK_FORMAT_UNDEFINED)
                            .BuildPipeline();
        return new Graphics::TestMaterial(layout, pipeline, { 0.7, 0.9, 0.4 });
    }

    Core::Entity AssetManager::LoadPrefab(char const *prefabName)
    {   
        // TODO: Read prefab json to get all properties
        Core::Entity prefab = ENGINE_NEW_ENTITY();
        prefab.AddComponent<Graphics::Transform>()->modelMatrix = Maths::Matrix4::Identity();
        prefab.AddComponent<Graphics::MeshRenderer>()->mesh = LoadMeshFromOBJ(prefabName);
        prefab.GetComponent<Graphics::MeshRenderer>()->mesh.Upload();
        prefab.GetComponent<Graphics::MeshRenderer>()->material = LoadMaterial(prefabName);
        return prefab;
    }

} // namespace Engine