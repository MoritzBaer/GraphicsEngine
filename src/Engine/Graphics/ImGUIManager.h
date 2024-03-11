#pragma once

#include "Util/Macros.h"
#include "Window.h"
#include "CommandQueue.h"

namespace Engine::Graphics
{
    class ImGUIManager
    {
        _SINGLETON(ImGUIManager, Window const * window)

        VkDescriptorPool imGUIPool;

    public:
        static void BeginFrame();

        static class ImGUIDrawCommand : public Command {
                VkImageView targetView;
                VkExtent2D targetExtent;
            public:
                ImGUIDrawCommand(VkImageView const & targetView, VkExtent2D const & targetExtent) : targetView(targetView), targetExtent(targetExtent) { }
                void QueueExecution(VkCommandBuffer const & queue) const;
        } DrawFrameCommand(VkImageView const & targetView, VkExtent2D const & targetExtent) { return ImGUIDrawCommand(targetView, targetExtent); }
    };
    
} // namespace Engine::Util
