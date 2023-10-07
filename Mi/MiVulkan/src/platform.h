#pragma once

#include <iostream>
#include <string>

#include <vector>
#include <array>
#include <optional>
#include <set>
#include <map>
#include <fstream>

#define MI_OPTION_DEBUG_MODE        "_debug"
#define MI_OPTION_RELEASE_MODE      "_releaseMode" // Release mode for the game engine
#define MI_OPTION_GAME_RELEASE_MODE "_game_ReleaseMode" // Release mode for the built game (exported game)
#define MI_MAX_IN_FLIGHT_FRAMES 10

#if defined(__APPLE__) || defined(__MACH__)
    #define MI_PLATFORM_OSX
    #define VK_USE_PLATFORM_METAL_EXT
    #define VK_USE_PLATFORM_MACOS_MVK
    #include <QuartzCore/QuartzCore.h>
    #include <OpenAL/OpenAL.h>
    #include <OpenCL/opencl.h>
#elif defined(_WIN32)
    #if defined(_WIN64)
        #define MI_PLATFORM_WINDOWS
        #define VK_USE_PLATFORM_WIN32_KHR
    #endif
#endif

#define LOG(x) std::cout << x << '\n'
#define MI_ASSERT(x) throw std::runtime_error(x)


const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation",
    }, deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#if defined(MI_PLATFORM_OSX)
        "VK_KHR_portability_subset"
#endif
    };

#if defined(NDEBUG)
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif




bool CheckValidationSupport() {

    std::vector<VkLayerProperties> layerProperties;
    uint32_t vLayerCount;
    vkEnumerateInstanceLayerProperties(&vLayerCount, nullptr);

    layerProperties.resize(vLayerCount);
    vkEnumerateInstanceLayerProperties(&vLayerCount, layerProperties.data());


    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerp : layerProperties) 
            if (strcmp(layerName, layerp.layerName) == 0) { layerFound = true; break; }


        if (!layerFound) return false;
    }
    return true;
}