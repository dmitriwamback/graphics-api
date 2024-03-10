#define GLFW_INCLUDE_VULKAN

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "platform.h"
/*
>definitions of variables/macros in <platform.h>

    ----------------- DEFINTIONS -----------------
    IN_FLIGHT_FRAMES,               -> Macro (integer)
    PLATFORM_OSX,                   -> Macro (no value)
    PLATFORM_WINDOWS                -> Macro (no value)

    ------------------ FUNCTIONS -----------------
    RTASSERT,                       -> Macro (function)
    RTLOG,                          -> Macro (function)

    ------------------ VARIABLES ------------------
    validationLayers,               -> std::vector
    deviceExtensions                -> std::vector
*/

//------------------------------------------------------------------------------------------------------//

std::vector<VkImage>        swapchainImages;
std::vector<VkImageView>    swapchainImageViews;
std::vector<VkFramebuffer>  framebuffers;

typedef struct vulkan_cxt 
{
    VkInstance                  instance;
    VkPhysicalDevice            physicalDevice;
    VkDevice                    device;
    VkQueue                     graphicsQueue;
    VkSurfaceKHR                surface;
    VkSwapchainKHR              swapchain;
    VkFormat                    format;
    VkExtent2D                  extent;
    VkDebugUtilsMessengerEXT    debug;
    GLFWwindow*                 window;
    VkQueue                     presentQueue;

} vkcxt;

vkcxt* vulkan_context;

//------------------------------------------------------------------------------------------------------//

#include "internal.h"
/*
>definitions of variables/macros/functions in <internal.h>
    ----------------- FUNCTIONS -----------------
    debugCallback,                  -> returns VkBool32,
    QueryQueueFamily                -> returns queue_family,
    CreateSynchronizedObjects       -> no return,
    QuerySwapchainSupport           -> returns swapchaindetails,
    ChooseSwapchainSurface          -> returns VkSurfaceFormatKHR,
    ChooseSwapchainPresentMode      -> returns VkPresentModeKHR,
    ChooseSwapExtent                -> returns VkExtent2D,
    CreateSwapchain                 -> no return,
    CreateFramebuffers              -> no return,
    CheckDeviceExtensions           -> returns bool,
    IsPhysicalDeviceAdequate        -> returns bool,
    QueryPhysicalDevices            -> returns VkPhysicalDevice,
    CreateDevice                    -> no return,

    ----------------- VARIABLES -----------------
    commandBuffers                  -> std::vector
    imageSemaphores                 -> std::vector
    inFlightFence                   -> std::vector
    commandPool                     -> VkCommandPool,
    depthImage                      -> VkImage,
    depthImageMemory                -> VkDeviceMemory,
    depthImageView                  -> VkImageView,
    swapchainFormat                 -> VkFormat,
    defaultTextureSampler           -> VkSampler,
    defaultRenderpass               -> VkRenderPass,
    global_queue_family             -> queuefamily (structure)

    ----------------- STRUCTURES ----------------
    queuefamily                     -> qf       (typedef),
    swapchaindetails                -> sd       (typedef),
    pipelinecontent                 -> pc       (typedef),
    pipelinecontextbuffer           -> pcb      (typedef),
    pipelinecreateinfo              -> pci      (typedef),
    uniformbuffermemory             -> ubm      (typedef),
    pipeline                        -> pl       (typedef)
*/


#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace RenderTools {

    void RTMainLoop() {
        
    }

    

    //------------------------------------------------------------------------------------------------------//
    //------------------------------------------------------------------------------------------------------//



    void Initialize() {

        //------------------------------------------------------------------------------------------------------//

        vulkan_context = static_cast<vkcxt*>(malloc(sizeof(vkcxt)));

        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        vulkan_context->window = glfwCreateWindow(1200, 800, "Render Tools", nullptr, nullptr);

        //------------------------------------------------------------------------------------------------------//

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        
        if (enableValidationLayers) requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        //------------------------------------------------------------------------------------------------------//


        VkApplicationInfo applicationInfo{};
            applicationInfo.sType                       = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            applicationInfo.pApplicationName            = "Render Tools";
            applicationInfo.pEngineName                 = "Render Tools";
            applicationInfo.applicationVersion          = VK_MAKE_VERSION(1, 0, 0);
            applicationInfo.engineVersion               = VK_MAKE_VERSION(1, 0, 0);
            applicationInfo.apiVersion                  = VK_API_VERSION_1_0;            


        VkInstanceCreateInfo instanceCreateInfo{};
            instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            instanceCreateInfo.pApplicationInfo = &applicationInfo;

#if defined(PLATFORM_OSX)
            requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            requiredExtensions.emplace_back("VK_KHR_get_physical_device_properties2");
            instanceCreateInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

            instanceCreateInfo.enabledExtensionCount    = (uint32_t)requiredExtensions.size();
            instanceCreateInfo.ppEnabledExtensionNames  = requiredExtensions.data();
            instanceCreateInfo.enabledLayerCount        = 0;

        if (enableValidationLayers && !CheckValidationSupport()) std::cout << "Validation layers not found!\n";

        if (vkCreateInstance(&instanceCreateInfo, nullptr, &vulkan_context->instance) != VK_SUCCESS) RTASSERT("Couldn't create VkInstance");
        if (glfwCreateWindowSurface(vulkan_context->instance, vulkan_context->window, nullptr, &vulkan_context->surface) != VK_SUCCESS) RTASSERT("Failed to create VkSurface");

        //------------------------------------------------------------------------------------------------------//

        vulkan_context->physicalDevice = QueryPhysicalDevices();
        CreateDevice();
        CreateSwapchain();
        CreateSynchronizedObjects();
        RTMainLoop();

        free(vulkan_context);
    }
}