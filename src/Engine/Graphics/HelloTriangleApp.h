#pragma once

#define GLFW_INCLUDE_VULKAN
#include "glfw3.h"
#include "vulkan/vulkan.h"
#include "shaderc/shaderc.hpp"
#include "../Maths/Matrix.h"
#include <vector>
#include <optional>

using namespace Engine;

class HelloTriangleApp
{
private:

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
    };

    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        std::vector<VkSurfaceFormatKHR> surfaceFormats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    void InitShaderc();
    void InitWindow();
    void InitVulkan();
    void MainLoop();
    void Cleanup();

    void CreateVKInstance();
    bool CheckValidationLayerSupport();
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
    std::vector<const char*> GetRequiredExtensions();
    void SetupDebugMessenger();
    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice const &device) const;
    uint32_t PhysicalDeviceScore(VkPhysicalDevice const &device) const;
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void CreateSurface();
    bool CheckDeviceExtensionSupport(VkPhysicalDevice const &device) const;
    SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice const &device) const;
    VkExtent2D ChooseSwapExtent(VkSurfaceCapabilitiesKHR const & capabilities) const;
    VkPresentModeKHR ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR> & availableModes) const;
    VkSurfaceFormatKHR ChooseSwapchainFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) const;
    void CreateSwapchain();
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect);
    void CreateImageViews();
    void CreateDescriptorSetLayout();
    void CreateDescriptorSets();
    void CreateGraphicsPipeline();
    VkShaderModule CreateShaderModule(std::vector<char> const & code) const;
    VkShaderModule CreateShaderModule(std::vector<uint32_t> const & code) const;
    std::vector<uint32_t> CompileToBytecode(std::string const & source, shaderc_shader_kind type) const;
    void CreateRenderPass();
    void CreateFramebuffers();
    void CreateVertexBuffer();
    void CreateIndexBuffer();
    void CreateUniformBuffers();
    void CreateDescriptorPool();
    void CreateDepthResources();
    void CreateCommandPool();
    void CreateImage(
        uint32_t width, 
        uint32_t height, 
        VkFormat format, 
        VkImageTiling tiling, 
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties, 
        VkImage& image, 
        VkDeviceMemory& imageMemory);
    void CreateTextureImage();
    VkCommandBuffer BeginSingleTimeCommand() const;
    void EndSingleTimeCommand(VkCommandBuffer commandBuffer) const;
    void CreateCommandBuffers();
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void UpdateUniformBuffers(uint32_t imageIndex);
    void Draw();
    void CreateSyncObjects();
    void CleanupSwapchain();
    void RecreateSwapchain();
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties, VkBuffer & buffer, VkDeviceMemory & memory) const;
    void CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size) const;
    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void CreateTextureImageView();
    void CreateTextureSampler();
    VkFormat FindSupportedFormat(std::vector<VkFormat> const & candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat FindDepthFormat();
    bool HasStencilComponent(VkFormat format);
    void LoadModel();
    
    GLFWwindow *window;

    const uint16_t WINDOW_WIDTH = 1600;
    const uint16_t WINDOW_HEIGHT = 900;

    const std::string MODEL_PATH = "../../models/viking_room.obj";
    const std::string TEXTURE_PATH = "../../textures/viking_room.png";

    class ShaderFileIncluder : public shaderc::CompileOptions::IncluderInterface {
        // Handles shaderc_include_resolver_fn callbacks.
        shaderc_include_result* GetInclude(const char* requested_source,
                                               shaderc_include_type type,
                                               const char* requesting_source,
                                               size_t include_depth);

        // Handles shaderc_include_result_release_fn callbacks.
        void ReleaseInclude(shaderc_include_result* data);
    } shaderIncludeResolver;
    shaderc::CompileOptions shaderCompileOptions;
    shaderc::Compiler shaderCompiler;

    struct VertexData;
    std::vector<VertexData> vertices;
    std::vector<uint32_t> indices;

    VkInstance vulkanInstance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalGPU = VK_NULL_HANDLE;
    VkDevice graphicsHandler;
    VkQueue graphicsQueue;
    VkQueue presentationQueue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    std::vector<VkFramebuffer> swapchainFramebuffers;
    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    uint32_t currentFrame = 0;
    bool framebufferResized = false;
    
    const std::vector<const char*> requiredValidationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> requiredDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    static void FramebufferResizeCallback(GLFWwindow* window, int w, int h);

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

public:
    void Run();

    struct VertexData
    {
        Maths::Vector3 position;
        Maths::Vector3 colour;
        Maths::Vector2 uv;

        static VkVertexInputBindingDescription BindingDescription();
        static std::array<VkVertexInputAttributeDescription, 3> AttributeDescription();
        inline bool operator==(const VertexData& other) const { return position == other.position && colour == other.colour && uv == other.uv; }
    };
};

namespace std {
    template<>
    struct hash<HelloTriangleApp::VertexData> {
        size_t operator()(HelloTriangleApp::VertexData const & vertexData) const {
            return ((hash<Engine::Maths::Vector3>()(vertexData.position) ^
                   (hash<Engine::Maths::Vector3>()(vertexData.colour) << 1)) >> 1) ^
                   (hash<Engine::Maths::Vector2>()(vertexData.uv) << 1);
        }
    };
}   // namespace std