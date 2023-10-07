#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "platform.h"
#include "core/mem/structs.h"
namespace Mi::Internal {

    std::vector<VkImage>        swapchainImages;
    std::vector<VkImageView>    swapchainImageViews;
    std::vector<VkFramebuffer>  framebuffers;
    Mi::Memory::mi_vkcxt*       mi_vulkancontext;
}
#include "util/global.h"
#include "core/mem/mem_util.h"
#include "core/mem/pipeline_memory.h"
#include "internal/ll.h"

#include "core/renderable/vertex.h"
#include "core/renderable/material.h"
#include "types/attrib_func.h"
#include "types/attrib.h"
#include "core/objects/registry.h"
#include "core/renderable/renderable.h"

#include "core/engine/engine.h"

namespace Mi::Internal {


    void Initialize() {

        mi_vulkancontext = static_cast<Mi::Memory::mi_vkcxt*>(malloc(sizeof(Mi::Memory::mi_vkcxt)));

        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        mi_vulkancontext->window = glfwCreateWindow(1200, 800, "Mi Engine <Vulkan>", nullptr, nullptr);


        std::vector<const char*> requiredExtensions;
        uint32_t glfwExtensionCount;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        for (int i = 0; i < glfwExtensionCount; i++) {
            requiredExtensions.emplace_back(glfwExtensions[i]);
        }

        VkApplicationInfo appCreateInfo{};
        appCreateInfo.sType                 = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appCreateInfo.applicationVersion    = VK_MAKE_VERSION(1, 0, 0);
        appCreateInfo.engineVersion         = VK_MAKE_VERSION(1, 0, 0);
        appCreateInfo.apiVersion            = VK_API_VERSION_1_0;
        appCreateInfo.pApplicationName      = "Mi";
        appCreateInfo.pEngineName           = "Mi Engine Vulkan";



        VkInstanceCreateInfo instanceCreateInfo{};
        instanceCreateInfo.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pApplicationInfo     = &appCreateInfo;

#if defined(MI_PLATFORM_OSX)
        requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        requiredExtensions.emplace_back("VK_KHR_get_physical_device_properties2");
        instanceCreateInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif 
        instanceCreateInfo.enabledExtensionCount   = (uint32_t)requiredExtensions.size();
        instanceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();
        instanceCreateInfo.enabledLayerCount = 0;

        if (enableValidationLayers && !CheckValidationSupport()) MI_ASSERT("Validation layers not found");
        if (enableValidationLayers)
        {
            instanceCreateInfo.enabledLayerCount        = static_cast<uint32_t>(validationLayers.size());
            instanceCreateInfo.ppEnabledLayerNames      = validationLayers.data();
        }
        else instanceCreateInfo.enabledLayerCount = 0;
        


        if (vkCreateInstance(&instanceCreateInfo, nullptr, &mi_vulkancontext->instance) != VK_SUCCESS) MI_ASSERT("Failed to initialize VkInstance");
        if (glfwCreateWindowSurface(mi_vulkancontext->instance, mi_vulkancontext->window, nullptr, &mi_vulkancontext->surface) != VK_SUCCESS) MI_ASSERT("Failed to create VkSurface");

        mi_vulkancontext->physicalDevice = Mi::Internal::LowLevel::QueryPhysicalDevices();
        Mi::Internal::LowLevel::CreateDevice();
        Mi::Internal::LowLevel::CreateSwapchain();

        Mi::Core::Engine::MiMainLoop();
    }
}