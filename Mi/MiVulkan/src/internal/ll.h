namespace Mi::Internal::LowLevel {


    Mi::Memory::mi_queuefamily mi_family;

    
    /*
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        DEBUG | VALIDATION
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    */

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
                                                        void* userData) {
        
        std::cerr << "validation layer: " << callbackData->pMessage << '\n';
        return VK_FALSE;
    }


    /*
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        QUEUE FAMILY
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    */
    Mi::Memory::mi_queuefamily FindQueueFamily(VkPhysicalDevice device) {
        Mi::Memory::mi_queuefamily indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {

            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphicsFamily = i;

            VkBool32 present = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mi_vulkancontext->surface, &present);

            if (present) indices.presentQueue = i;
            if (indices.graphicsFamily.has_value() && indices.presentQueue.has_value()) return indices;
            i++;
        }

        return indices;
    }

    /*
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        SWAPCHAIN | FRAMEBUFFERS
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    */


    Mi::Memory::mi_swapchaindetails QuerySwapchainSupport(VkPhysicalDevice device) {
        Mi::Memory::mi_swapchaindetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mi_vulkancontext->surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, mi_vulkancontext->surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, mi_vulkancontext->surface, &formatCount, details.formats.data());
        }
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, mi_vulkancontext->surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, mi_vulkancontext->surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }


    VkSurfaceFormatKHR ChooseSwapchainSurface(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }


    VkPresentModeKHR ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {

        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) return availablePresentMode;
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }


    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) return capabilities.currentExtent;

        int width, height;
        glfwGetFramebufferSize(mi_vulkancontext->window, &width, &height);

        VkExtent2D extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
        extent.width  = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return extent;
    }
    

    void CreateSwapchain() {

        Mi::Memory::mi_swapchaindetails details = QuerySwapchainSupport(mi_vulkancontext->physicalDevice);
        VkSurfaceFormatKHR sformat              = ChooseSwapchainSurface(details.formats);
        VkPresentModeKHR pMode                  = ChooseSwapchainPresentMode(details.presentModes);
        VkExtent2D extent                       = ChooseSwapExtent(details.capabilities);
        
        mi_vulkancontext->extent = extent;
        mi_vulkancontext->format = sformat.format;

        Mi::GlobalUtility::swapchainFormat = sformat.format;

        uint32_t imageCount = details.capabilities.minImageCount + 1;
        if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount) imageCount = details.capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface          = mi_vulkancontext->surface;
        createInfo.minImageCount    = imageCount;
        createInfo.imageFormat      = sformat.format;
        createInfo.imageColorSpace  = sformat.colorSpace;
        createInfo.imageExtent      = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        Mi::Memory::mi_queuefamily indices = FindQueueFamily(mi_vulkancontext->physicalDevice);
        if (indices.graphicsFamily != indices.presentQueue) {
            uint32_t q[] = {indices.graphicsFamily.value(), indices.presentQueue.value()};
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = q;
        }
        else createInfo.imageSharingMode    = VK_SHARING_MODE_EXCLUSIVE;

        createInfo.pQueueFamilyIndices      = nullptr;
        createInfo.preTransform             = details.capabilities.currentTransform;
        createInfo.compositeAlpha           = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode              = pMode;
        createInfo.clipped                  = VK_TRUE;
        createInfo.oldSwapchain             = VK_NULL_HANDLE;
        if (vkCreateSwapchainKHR(mi_vulkancontext->device, &createInfo, nullptr, &mi_vulkancontext->swapchain) != VK_SUCCESS) MI_ASSERT("Failed to create Swapchain");

        vkGetSwapchainImagesKHR(mi_vulkancontext->device, mi_vulkancontext->swapchain, &imageCount, nullptr);
        swapchainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(mi_vulkancontext->device, mi_vulkancontext->swapchain, &imageCount, swapchainImages.data());

        swapchainImageViews.resize(swapchainImages.size());
        for (size_t i = 0; i < swapchainImages.size(); i++) 
        {
            VkImageViewCreateInfo imageCreateInfo{};
            imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageCreateInfo.image = swapchainImages[i];
            imageCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageCreateInfo.format = Mi::Internal::mi_vulkancontext->format;

            imageCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            imageCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageCreateInfo.subresourceRange.baseMipLevel = 0;
            imageCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageCreateInfo.subresourceRange.levelCount = 1;
            imageCreateInfo.subresourceRange.layerCount = 1;
            
            if (vkCreateImageView(Mi::Internal::mi_vulkancontext->device, &imageCreateInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS) MI_ASSERT("Failed to create Swapchain Image Views");
        }
    }



    void CreateFramebuffers() {

        Mi::Internal::framebuffers.resize(Mi::Internal::swapchainImageViews.size());

        for (size_t i = 0; i < Mi::Internal::swapchainImageViews.size(); i++) {

            std::array<VkImageView, 2> attachments = {
                Mi::Internal::swapchainImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = Mi::GlobalUtility::defaultRenderpass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width  = Mi::Internal::mi_vulkancontext->extent.width;
            framebufferInfo.height = Mi::Internal::mi_vulkancontext->extent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(Mi::Internal::mi_vulkancontext->device, 
                                   &framebufferInfo, nullptr, 
                                   &Mi::Internal::framebuffers[i]) != VK_SUCCESS) MI_ASSERT("Framebuffer");
        }
    }


    /*
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        PHYSICAL DEVICE  
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    */

    bool CheckDeviceExtensions(VkPhysicalDevice device) {

        uint32_t deviceExtenionsCount;
        std::vector<VkExtensionProperties> availableExtensions;
        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        vkEnumerateDeviceExtensionProperties(device, nullptr, &deviceExtenionsCount, nullptr);

        availableExtensions.resize(deviceExtenionsCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &deviceExtenionsCount, availableExtensions.data());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }
        
        return requiredExtensions.empty();
    }

    bool IsPhysicalDeviceAdequate(VkPhysicalDevice device) {

        VkPhysicalDeviceProperties  deviceProperties;
        VkPhysicalDeviceFeatures    features,
                                    supportedFeatures;

        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &features);
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        Mi::Memory::mi_swapchaindetails details = QuerySwapchainSupport(device);

        bool adequate = !details.formats.empty() && !details.presentModes.empty();
        mi_family = FindQueueFamily(device);
        return mi_family.graphicsFamily.has_value() && CheckDeviceExtensions(device) && adequate && supportedFeatures.samplerAnisotropy;
    }

    VkPhysicalDevice QueryPhysicalDevices() {

        uint32_t physicalDeviceCount;
        std::vector<VkPhysicalDevice> devices;
        vkEnumeratePhysicalDevices(mi_vulkancontext->instance, &physicalDeviceCount, nullptr);

        devices.resize(physicalDeviceCount);
        vkEnumeratePhysicalDevices(mi_vulkancontext->instance, &physicalDeviceCount, devices.data());

        for (VkPhysicalDevice device : devices) 
            if (IsPhysicalDeviceAdequate(device)) return device;

        MI_ASSERT("Couldn't find an adequate physical device");
        return VK_NULL_HANDLE;
    }

    void CreateDevice() {

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        for (uint32_t qFamily : std::set<uint32_t>{mi_family.graphicsFamily.value(), mi_family.presentQueue.value()})
        {
            float queuePriority = 1.0f;

            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = qFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        VkDeviceCreateInfo deviceCreateInfo{};

        deviceCreateInfo.sType                      = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pQueueCreateInfos          = queueCreateInfos.data();
        deviceCreateInfo.queueCreateInfoCount       = 1;
        deviceCreateInfo.pEnabledFeatures           = &deviceFeatures;
        deviceCreateInfo.ppEnabledExtensionNames    = deviceExtensions.data();
        deviceCreateInfo.enabledExtensionCount      = deviceExtensions.size();

        if (vkCreateDevice(mi_vulkancontext->physicalDevice, &deviceCreateInfo, nullptr, &mi_vulkancontext->device) != VK_SUCCESS) MI_ASSERT("Failed to create the Logical Device");
        vkGetDeviceQueue(mi_vulkancontext->device, mi_family.graphicsFamily.value(), 0, &mi_vulkancontext->graphicsQueue);
        vkGetDeviceQueue(mi_vulkancontext->device, mi_family.presentQueue.value(), 0, &mi_vulkancontext->presentQueue);
    }
}