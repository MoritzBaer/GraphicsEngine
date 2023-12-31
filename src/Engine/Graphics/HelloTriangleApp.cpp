#include "HelloTriangleApp.h"

#include <stdexcept>
#include <iostream>
#include <set>
#include <limits>
#include <algorithm>
#include <fstream>

static std::vector<char> ReadFile(const std::string& filename) {
    std::cout << "Trying to open " << filename << "\n";
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
    
    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

VkShaderModule HelloTriangleApp::CreateShaderModule(std::vector<char> const & code) const {
    VkShaderModuleCreateInfo shaderInfo{};
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = code.size();
    shaderInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
    
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(graphicsHandler, &shaderInfo, nullptr, &shaderModule) != VK_SUCCESS) { throw std::runtime_error("failed to create shader module!"); }

    return shaderModule;
}

void HelloTriangleApp::CreateRenderPass()
{
    VkAttachmentDescription colourAttachment{};
    colourAttachment.format = swapchainImageFormat;
    colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colourAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colourAttachmentReference{};
    colourAttachmentReference.attachment = 0;
    colourAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription{};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colourAttachmentReference;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;    
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;  
    
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colourAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDescription;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if(vkCreateRenderPass(graphicsHandler, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) { throw std::runtime_error("Failed to create render pass!"); }
}

void HelloTriangleApp::CreateFramebuffers()
{
    swapchainFramebuffers.resize(swapchainImageViews.size());

    for (size_t i = 0; i < swapchainImageViews.size(); i++) {
        VkImageView attachments[] = {
            swapchainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapchainExtent.width;
        framebufferInfo.height = swapchainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(graphicsHandler, &framebufferInfo, nullptr, &swapchainFramebuffers[i]) != VK_SUCCESS) { throw std::runtime_error("failed to create framebuffer!"); }
    }
}

void HelloTriangleApp::CreateCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(physicalGPU);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if(vkCreateCommandPool(graphicsHandler, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) { throw std::runtime_error("Failed to create command buffer!"); }
}

void HelloTriangleApp::CreateCommandBuffers()
{
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    if (vkAllocateCommandBuffers(graphicsHandler, &allocInfo, commandBuffers.data()) != VK_SUCCESS) { throw std::runtime_error("failed to allocate command buffers!"); }
}

void HelloTriangleApp::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) { throw std::runtime_error("failed to begin recording command buffer!"); }

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapchainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = swapchainExtent;
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    // Vulkan does EVERYTHING with structs, why tf is there a state machine here??!!
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);      
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchainExtent.width);
    viewport.height = static_cast<float>(swapchainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdDraw(commandBuffer, 3, 1, 0, 0);                   // Finally!!!
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) { throw std::runtime_error("failed to record command buffer!"); }
}

void HelloTriangleApp::Draw()
{
    vkWaitForFences(graphicsHandler, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(graphicsHandler, 1, &inFlightFences[currentFrame]);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(graphicsHandler, swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    vkResetCommandBuffer(commandBuffers[currentFrame], 0);
    RecordCommandBuffer(commandBuffers[currentFrame], imageIndex);
    
    VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;  

    if(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) { throw std::runtime_error("Failed to submit command buffer for draw command!"); }
    
    VkSwapchainKHR swapchains[] = { swapchain };
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(presentationQueue, &presentInfo);

    (++currentFrame) %= MAX_FRAMES_IN_FLIGHT;
}

void HelloTriangleApp::CreateSyncObjects()
{
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if( vkCreateSemaphore(graphicsHandler, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS
        ||  vkCreateSemaphore(graphicsHandler, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS
        ||  vkCreateFence(graphicsHandler, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
        { throw std::runtime_error("Failed to create synchronization objects for frame " + i + '!'); }
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApp::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

std::vector<const char *> HelloTriangleApp::GetRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void HelloTriangleApp::SetupDebugMessenger()
{
    if(!enableValidationLayers) { return; }

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    PopulateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(vulkanInstance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) { throw std::runtime_error("failed to set up debug messenger!"); }
}

void HelloTriangleApp::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

VkSurfaceFormatKHR HelloTriangleApp::ChooseSwapchainFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) const {
    auto it = std::find_if(availableFormats.begin(), availableFormats.end(), [](VkSurfaceFormatKHR const &format) { return 
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    &&  format.format == VK_FORMAT_B8G8R8A8_SRGB; });
    if(it != availableFormats.end()) { return *it; }
    return availableFormats[0];
}

void HelloTriangleApp::CreateSwapchain()
{
    auto details = QuerySwapchainSupport(physicalGPU);
    auto format = ChooseSwapchainFormat(details.surfaceFormats);
    auto presentMode = ChooseSwapchainPresentMode(details.presentModes);
    auto extent = ChooseSwapExtent(details.surfaceCapabilities);

    uint32_t imageCount = details.surfaceCapabilities.minImageCount + 1;
    if(details.surfaceCapabilities.maxImageCount) { imageCount = std::min(imageCount, details.surfaceCapabilities.maxImageCount); }

    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = surface;
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = format.format;
    swapchainCreateInfo.imageColorSpace = format.colorSpace;
    swapchainCreateInfo.imageExtent = extent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = FindQueueFamilies(physicalGPU);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    swapchainCreateInfo.preTransform = details.surfaceCapabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;

    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(graphicsHandler, &swapchainCreateInfo, nullptr, &swapchain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }


    vkGetSwapchainImagesKHR(graphicsHandler, swapchain, &imageCount, nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(graphicsHandler, swapchain, &imageCount, swapchainImages.data());

    swapchainImageFormat = format.format;
    swapchainExtent = extent;
}

void HelloTriangleApp::CreateImageViews()
{
    swapchainImageViews.resize(swapchainImages.size());

    for(int i = 0; i < swapchainImages.size(); i++) {
        VkImageViewCreateInfo imageViewInfo{};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.image = swapchainImages[i];
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format = swapchainImageFormat;
        imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewInfo.subresourceRange.baseMipLevel = 0;
        imageViewInfo.subresourceRange.levelCount = 1;
        imageViewInfo.subresourceRange.baseArrayLayer = 0;
        imageViewInfo.subresourceRange.layerCount = 1;

        if(vkCreateImageView(graphicsHandler, &imageViewInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS) { throw std::runtime_error("Failed to create image view for image " + i); }
    }
}

void HelloTriangleApp::CreateGraphicsPipeline()
{
    auto vertexShaderCode = ReadFile("../../shaders/compiled/vulkan-tutorial/vert.spv");
    auto fragmentShaderCode = ReadFile("../../shaders/compiled/vulkan-tutorial/frag.spv");

    auto vertexShader = CreateShaderModule(vertexShaderCode);
    auto fragmentShader = CreateShaderModule(fragmentShaderCode);

    VkPipelineShaderStageCreateInfo vertexStageInfo{};
    vertexStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexStageInfo.module = vertexShader;
    vertexStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentStageInfo{};
    fragmentStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentStageInfo.module = fragmentShader;
    fragmentStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertexStageInfo, fragmentStageInfo};
    
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    VkPipelineColorBlendStateCreateInfo colourBlending{};
    colourBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colourBlending.logicOpEnable = VK_FALSE;
    colourBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colourBlending.attachmentCount = 1;
    colourBlending.pAttachments = &colorBlendAttachment;
    colourBlending.blendConstants[0] = 0.0f; // Optional
    colourBlending.blendConstants[1] = 0.0f; // Optional
    colourBlending.blendConstants[2] = 0.0f; // Optional
    colourBlending.blendConstants[3] = 0.0f; // Optional

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0; // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(graphicsHandler, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) { throw std::runtime_error("failed to create pipeline layout!"); }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colourBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if(vkCreateGraphicsPipelines(graphicsHandler, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) { throw std::runtime_error("Failed to create graphics pipeline!"); }

    vkDestroyShaderModule(graphicsHandler, vertexShader, nullptr);
    vkDestroyShaderModule(graphicsHandler, fragmentShader, nullptr);
}

VkPresentModeKHR HelloTriangleApp::ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR> & availableModes) const {
    auto it = std::find(availableModes.begin(), availableModes.end(), VK_PRESENT_MODE_MAILBOX_KHR);
    if(it != availableModes.end()) { return *it; }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D HelloTriangleApp::ChooseSwapExtent(VkSurfaceCapabilitiesKHR const & capabilities) const {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void HelloTriangleApp::InitWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello Triangle", nullptr, nullptr);
}

void HelloTriangleApp::InitVulkan()
{
    CreateVKInstance();
    SetupDebugMessenger();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapchain();
    CreateImageViews();
    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateFramebuffers();
    CreateCommandPool();
    CreateCommandBuffers();
    CreateSyncObjects();
}

void HelloTriangleApp::MainLoop()
{
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        Draw();
    }

    vkDeviceWaitIdle(graphicsHandler);
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

void HelloTriangleApp::Cleanup()
{
    glfwDestroyWindow(window);
    glfwTerminate();

    for(auto sem : imageAvailableSemaphores) { vkDestroySemaphore(graphicsHandler, sem, nullptr); }
    for(auto sem : renderFinishedSemaphores) { vkDestroySemaphore(graphicsHandler, sem, nullptr); }
    for(auto fence : inFlightFences) { vkDestroyFence(graphicsHandler, fence, nullptr); }
    for(auto framebuffer : swapchainFramebuffers) { vkDestroyFramebuffer(graphicsHandler, framebuffer, nullptr); }
    vkDestroyPipeline(graphicsHandler, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(graphicsHandler, pipelineLayout, nullptr);
    vkDestroyRenderPass(graphicsHandler, renderPass, nullptr);
    for(auto imageView : swapchainImageViews) { vkDestroyImageView(graphicsHandler, imageView, nullptr); }
    vkDestroySwapchainKHR(graphicsHandler, swapchain, nullptr);
    vkDestroyDevice(graphicsHandler, nullptr);
    vkDestroySurfaceKHR(vulkanInstance, surface, nullptr);
    if(enableValidationLayers) { DestroyDebugUtilsMessengerEXT(vulkanInstance, debugMessenger, nullptr); }
    vkDestroyInstance(vulkanInstance, nullptr);
}
    
bool HelloTriangleApp::CheckValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for(const char* validationLayer : requiredValidationLayers) {
        if(std::find_if(
                availableLayers.begin(), 
                availableLayers.end(), 
                [validationLayer](const VkLayerProperties &prop) { return (strcmp(validationLayer, prop.layerName)) != 0; }) 
            == availableLayers.end()) { return false; }
    }

    return true;
}

HelloTriangleApp::QueueFamilyIndices HelloTriangleApp::FindQueueFamilies(VkPhysicalDevice const &device) const {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for(int i = 0; i < queueFamilies.size(); i++) {
        if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) { indices.graphicsFamily = i; }

        VkBool32 presentationSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentationSupport);
        if(presentationSupport) { indices.presentFamily = i; }
    }


    return indices;
}

uint32_t HelloTriangleApp::PhysicalDeviceScore(VkPhysicalDevice const &device) const { 
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    auto queueFamilies = FindQueueFamilies(device);
    auto swapchainDetails = QuerySwapchainSupport(device);

    bool deviceHasNecessaryCapabilities = deviceFeatures.geometryShader
        && queueFamilies.graphicsFamily.has_value()
        && queueFamilies.presentFamily.has_value()
        && CheckDeviceExtensionSupport(device)
        && !swapchainDetails.surfaceFormats.empty()
        && !swapchainDetails.presentModes.empty();

    uint32_t score = ( 1000 * (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) + 1 )
        * deviceHasNecessaryCapabilities;

    return score;
}

void HelloTriangleApp::PickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, nullptr);
    if(deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, devices.data());

    auto it = std::max_element(devices.begin(), devices.end(), [this](VkPhysicalDevice const &a, VkPhysicalDevice const &b) { return PhysicalDeviceScore(a) < PhysicalDeviceScore(b); });
    if(it == devices.end() || PhysicalDeviceScore(*it) == 0) { throw std::runtime_error("Failed to find a suitable GPU!"); }
    physicalGPU = *it;

#ifdef NDEBUG
#else
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalGPU, &deviceProperties);
    std::cout << "Device '" << deviceProperties.deviceName << "' selected for executing graphics commands\n";
#endif
}

void HelloTriangleApp::CreateLogicalDevice()
{
    QueueFamilyIndices indices = FindQueueFamilies(physicalGPU);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures features{};

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pEnabledFeatures = &features;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();

    // Only here for compatibility with older Vulkan implementations
    if (enableValidationLayers) {
        deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(requiredValidationLayers.size());
        deviceCreateInfo.ppEnabledLayerNames = requiredValidationLayers.data();
    } else {
        deviceCreateInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalGPU, &deviceCreateInfo, nullptr, &graphicsHandler) != VK_SUCCESS) { throw std::runtime_error("failed to create logical device!"); }

    vkGetDeviceQueue(graphicsHandler, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(graphicsHandler, indices.presentFamily.value(), 0, &presentationQueue);
}

void HelloTriangleApp::CreateSurface()
{
    if (glfwCreateWindowSurface(vulkanInstance, window, nullptr, &surface) != VK_SUCCESS) { throw std::runtime_error("failed to create window surface!"); }
}

bool HelloTriangleApp::CheckDeviceExtensionSupport(VkPhysicalDevice const &device) const
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(requiredDeviceExtensions.begin(), requiredDeviceExtensions.end());
    for(auto e : availableExtensions) { requiredExtensions.erase(e.extensionName); }
    return requiredExtensions.empty();
}

HelloTriangleApp::SwapchainSupportDetails HelloTriangleApp::QuerySwapchainSupport(VkPhysicalDevice const &device) const
{
    SwapchainSupportDetails details{};

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.surfaceCapabilities);
    
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    details.surfaceFormats.resize(formatCount);
    if(formatCount) { vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.surfaceFormats.data()); }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    details.presentModes.resize(presentModeCount);
    if(formatCount) { vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &formatCount, details.presentModes.data()); }

    return details;
}

void HelloTriangleApp::CreateVKInstance()
{
    if (enableValidationLayers && !CheckValidationLayerSupport()) { throw std::runtime_error("validation layers requested, but not available!"); }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Vulkan Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if(enableValidationLayers) {
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(requiredValidationLayers.size());
        instanceCreateInfo.ppEnabledLayerNames = requiredValidationLayers.data();

        PopulateDebugMessengerCreateInfo(debugCreateInfo);
        instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;

    } else { instanceCreateInfo.enabledLayerCount = 0; }

    auto extensions = GetRequiredExtensions();

    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = extensions.data();
    instanceCreateInfo.enabledLayerCount = 0;

    VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &vulkanInstance);

    if (vkCreateInstance(&instanceCreateInfo, nullptr, &vulkanInstance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensionProperties(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProperties.data());
}

void HelloTriangleApp::Run()
{
    InitWindow();
    InitVulkan();
    MainLoop();
    Cleanup();
}