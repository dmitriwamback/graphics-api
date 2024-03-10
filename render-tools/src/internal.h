namespace RenderTools {

    std::vector<VkCommandBuffer>    commandBuffers      = std::vector<VkCommandBuffer>();
    std::vector<VkSemaphore>        imageSemaphores     = std::vector<VkSemaphore>();
    std::vector<VkSemaphore>        renderSemaphores    = std::vector<VkSemaphore>();
    std::vector<VkFence>            inFlightFence       = std::vector<VkFence>();
    VkCommandPool                   commandPool;

    VkImage             depthImage;
    VkDeviceMemory      depthImageMemory;
    VkImageView         depthImageView;
    VkFormat            swapchainFormat;
    VkSampler           defaultTextureSampler;
    VkRenderPass        defaultRenderpass;

    /*
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        STRUCTURES
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    */

    typedef struct qf {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentQueue;
    } queuefamily;

    typedef struct sd {
        VkSurfaceCapabilitiesKHR        capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR>   presentModes;
    } swapchaindetails;



    typedef struct pc
    {
        VkPipelineLayout    layout;
        VkRenderPass        renderPass;
        VkRect2D            scissor{};
        VkDescriptorSetLayoutBinding        uniformBufferLayoutBinding{};
        VkDescriptorSetLayoutCreateInfo     layoutInfo{};
        VkDescriptorSetLayout               setLayoutDescriptor{};

    } pipelinecontent;

    typedef struct pci
    {
        VkPipelineDynamicStateCreateInfo            dynamicState{};
        VkPipelineVertexInputStateCreateInfo        vertexInput{};
        VkPipelineInputAssemblyStateCreateInfo      inputAssembly{};
        VkPipelineViewportStateCreateInfo           viewportState{};
        VkPipelineRasterizationStateCreateInfo      rasterizer{};
        VkPipelineColorBlendStateCreateInfo         colorBlending{};
        uint32_t imageIndex;

    } pipelinecreateinfo;

    typedef struct ubm 
    {
        std::vector<VkBuffer>       uniformbuffers;
        std::vector<VkDeviceMemory> mem_uniformbuffers;
        std::vector<void*>          mem_mapped;

    } uniformbuffermemory;

    typedef struct pcb 
    {
        bool isIndexed;
        VkBuffer vertexBuffer, 
                 indexBuffer;
        VkDeviceMemory vertexBufferMemory, 
                       indexBufferMemory;

    } pipelinecontextbuffer;

    typedef struct pipeline 
    {
        VkPipeline pipeline;

        pipelinecontent*         content;
        pipelinecreateinfo*      data;
        uniformbuffermemory*     memory;
        pipelinecontextbuffer*   context;
    } pl;


    qf global_queue_family;

    /*
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        DEBUG | VALIDATION
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    */

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
                                                        void* userData) 
    {
        
        std::cerr << "validation layer: " << callbackData->pMessage << '\n';
        return VK_FALSE;
    }

    /*
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        QUEUE FAMILY
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    */

    qf QueryQueueFamily(VkPhysicalDevice pDevice) {

        qf indices;
        uint32_t queueFamilyCount;
        std::vector<VkQueueFamilyProperties> familyProperties;

        vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount, nullptr);

        familyProperties.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount, familyProperties.data());

        int index = 0;
        for (const auto& queueFamily: familyProperties) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphicsFamily = index;

            VkBool32 present = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(pDevice, index, vulkan_context->surface, &present);

            if (present) indices.presentQueue = index;
            if (indices.graphicsFamily.has_value() && indices.presentQueue.has_value()) return indices;
            index++;
        }

        return indices;
    }

    /*
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        SWAPCHAIN | FRAMEBUFFERS
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    */

    void CreateSynchronizedObjects() {

        renderSemaphores.resize(IN_FLIGHT_FRAMES);
        imageSemaphores .resize(IN_FLIGHT_FRAMES);
        inFlightFence   .resize(IN_FLIGHT_FRAMES);
        commandBuffers  .resize(IN_FLIGHT_FRAMES);

        
        qf family = QueryQueueFamily(vulkan_context->physicalDevice);
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = family.graphicsFamily.value();

        if (vkCreateCommandPool(vulkan_context->device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) RTASSERT("Failed to create VkCommandPool");

        VkCommandBufferAllocateInfo commandBufferAlloc{};
        commandBufferAlloc.sType                = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAlloc.commandBufferCount   = IN_FLIGHT_FRAMES;
        commandBufferAlloc.level                = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAlloc.commandPool          = commandPool;

        if (vkAllocateCommandBuffers(vulkan_context->device, &commandBufferAlloc, commandBuffers.data()) != VK_SUCCESS) RTASSERT("Failed to allocate VkCommandBuffer(s)");


        VkFenceCreateInfo fenceCreateInfo{};

        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkSemaphoreCreateInfo semaphoreCreateInfo{};

        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        for (int i = 0; i < IN_FLIGHT_FRAMES; i++)
        {
            if (vkCreateSemaphore(vulkan_context->device, &semaphoreCreateInfo, nullptr, &imageSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(vulkan_context->device, &semaphoreCreateInfo, nullptr, &renderSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(vulkan_context->device, &fenceCreateInfo, nullptr, &inFlightFence[i]) != VK_SUCCESS)
            {
                RTASSERT("Failed to create VkSemaphore or VkFence");
            }
        }
    }

    /*
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        PHYSICAL DEVICE  
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    */

    sd QuerySwapchainSupport(VkPhysicalDevice pDevice) {

        sd details;
        uint32_t formatCount;
        uint32_t presentModeCount;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pDevice, vulkan_context->surface, &details.capabilities);

        vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, vulkan_context->surface, &formatCount, nullptr);
        vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, vulkan_context->surface, &presentModeCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, vulkan_context->surface, &formatCount, details.formats.data());
        }
        if (presentModeCount != 0) {
            details.presentModes.resize(formatCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, vulkan_context->surface, &presentModeCount, details.presentModes.data());
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
        glfwGetFramebufferSize(vulkan_context->window, &width, &height);

        VkExtent2D extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
        extent.width  = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return extent;
    }

    void CreateSwapchain() {

        swapchaindetails details                = QuerySwapchainSupport(vulkan_context->physicalDevice);
        VkSurfaceFormatKHR sformat              = ChooseSwapchainSurface(details.formats);
        VkPresentModeKHR pMode                  = ChooseSwapchainPresentMode(details.presentModes);
        VkExtent2D extent                       = ChooseSwapExtent(details.capabilities);
        
        vulkan_context->extent = extent;
        vulkan_context->format = sformat.format;

        swapchainFormat = sformat.format;

        uint32_t imageCount = details.capabilities.minImageCount + 1;
        if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount) imageCount = details.capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface          = vulkan_context->surface;
        createInfo.minImageCount    = imageCount;
        createInfo.imageFormat      = sformat.format;
        createInfo.imageColorSpace  = sformat.colorSpace;
        createInfo.imageExtent      = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        queuefamily indices = QueryQueueFamily(vulkan_context->physicalDevice);
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
        if (vkCreateSwapchainKHR(vulkan_context->device, &createInfo, nullptr, &vulkan_context->swapchain) != VK_SUCCESS) RTASSERT("Failed to create Swapchain");

        vkGetSwapchainImagesKHR(vulkan_context->device, vulkan_context->swapchain, &imageCount, nullptr);
        swapchainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(vulkan_context->device, vulkan_context->swapchain, &imageCount, swapchainImages.data());

        swapchainImageViews.resize(swapchainImages.size());
        for (size_t i = 0; i < swapchainImages.size(); i++) 
        {
            VkImageViewCreateInfo imageCreateInfo{};
                imageCreateInfo.sType           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                imageCreateInfo.image           = swapchainImages[i];
                imageCreateInfo.viewType        = VK_IMAGE_VIEW_TYPE_2D;
                imageCreateInfo.format          = vulkan_context->format;

                imageCreateInfo.components.r    = VK_COMPONENT_SWIZZLE_IDENTITY;
                imageCreateInfo.components.g    = VK_COMPONENT_SWIZZLE_IDENTITY;
                imageCreateInfo.components.b    = VK_COMPONENT_SWIZZLE_IDENTITY;
                imageCreateInfo.components.a    = VK_COMPONENT_SWIZZLE_IDENTITY;

                imageCreateInfo.subresourceRange.aspectMask         = VK_IMAGE_ASPECT_COLOR_BIT;
                imageCreateInfo.subresourceRange.baseMipLevel       = 0;
                imageCreateInfo.subresourceRange.baseArrayLayer     = 0;
                imageCreateInfo.subresourceRange.levelCount         = 1;
                imageCreateInfo.subresourceRange.layerCount         = 1;
            
            if (vkCreateImageView(vulkan_context->device, &imageCreateInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS) RTASSERT("Failed to create Swapchain Image Views");
        }
    }

    void CreateFramebuffers() {

        framebuffers.resize(swapchainImageViews.size());

        for (size_t i = 0; i < swapchainImageViews.size(); i++) {

            std::array<VkImageView, 2> attachments = {
                swapchainImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = defaultRenderpass;
                framebufferInfo.attachmentCount = 1;
                framebufferInfo.pAttachments = attachments.data();
                framebufferInfo.width  = vulkan_context->extent.width;
                framebufferInfo.height = vulkan_context->extent.height;
                framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(vulkan_context->device, 
                                   &framebufferInfo, nullptr, 
                                   &framebuffers[i]) != VK_SUCCESS) RTASSERT("Framebuffer");
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

        swapchaindetails details = QuerySwapchainSupport(device);

        bool adequate = !details.formats.empty() && !details.presentModes.empty();
        global_queue_family = QueryQueueFamily(device);
        return global_queue_family.graphicsFamily.has_value() && CheckDeviceExtensions(device) && adequate && supportedFeatures.samplerAnisotropy;
    }

    VkPhysicalDevice QueryPhysicalDevices() {

        uint32_t physicalDeviceCount;
        std::vector<VkPhysicalDevice> devices;
        vkEnumeratePhysicalDevices(vulkan_context->instance, &physicalDeviceCount, nullptr);

        devices.resize(physicalDeviceCount);
        vkEnumeratePhysicalDevices(vulkan_context->instance, &physicalDeviceCount, devices.data());

        for (VkPhysicalDevice device : devices) 
            if (IsPhysicalDeviceAdequate(device)) return device;

        RTASSERT("Couldn't find an adequate physical device");
        return VK_NULL_HANDLE;
    }

    void CreateDevice() {

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        for (uint32_t qFamily : std::set<uint32_t>{global_queue_family.graphicsFamily.value(), global_queue_family.presentQueue.value()})
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

        if (vkCreateDevice(vulkan_context->physicalDevice, &deviceCreateInfo, nullptr, &vulkan_context->device) != VK_SUCCESS) RTASSERT("Failed to create the Logical Device");
        vkGetDeviceQueue(vulkan_context->device, global_queue_family.graphicsFamily.value(), 0, &vulkan_context->graphicsQueue);
        vkGetDeviceQueue(vulkan_context->device, global_queue_family.presentQueue.value(), 0, &vulkan_context->presentQueue);
    }
}