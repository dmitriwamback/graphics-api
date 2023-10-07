namespace Mi::Memory {

    void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType               = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType           = VK_IMAGE_TYPE_2D;
        imageCreateInfo.extent.width        = width;
        imageCreateInfo.extent.height       = height;
        imageCreateInfo.extent.depth        = 1;
        imageCreateInfo.mipLevels           = 1;
        imageCreateInfo.arrayLayers         = 1;
        imageCreateInfo.format              = format;
        imageCreateInfo.tiling              = tiling;
        imageCreateInfo.initialLayout       = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.usage               = usage;
        imageCreateInfo.samples             = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.sharingMode         = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(Mi::Internal::mi_vulkancontext->device, &imageCreateInfo, nullptr, &image) != VK_SUCCESS) MI_ASSERT("Failed to create an Image");

        VkMemoryRequirements mem;
        vkGetImageMemoryRequirements(Mi::Internal::mi_vulkancontext->device, image, &mem);

        VkMemoryAllocateInfo allocate{};
        allocate.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocate.allocationSize = mem.size;
        allocate.memoryTypeIndex = Mi::Memory::FindMemoryType(mem.memoryTypeBits, properties);

        if (vkAllocateMemory(Mi::Internal::mi_vulkancontext->device, &allocate, nullptr, &imageMemory) != VK_SUCCESS) MI_ASSERT("Failed to allocate image memory");
        vkBindImageMemory(Mi::Internal::mi_vulkancontext->device, image, imageMemory, 0);
    }

    /*
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    */

    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags flags) {

        VkImageViewCreateInfo viewCreateInfo{};
        viewCreateInfo.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewCreateInfo.image            = image;
        viewCreateInfo.viewType         = VK_IMAGE_VIEW_TYPE_2D;
        viewCreateInfo.format           = format;
        viewCreateInfo.subresourceRange.aspectMask      = flags;
        viewCreateInfo.subresourceRange.baseMipLevel    = 0;
        viewCreateInfo.subresourceRange.baseArrayLayer  = 0;
        viewCreateInfo.subresourceRange.levelCount      = 1;
        viewCreateInfo.subresourceRange.layerCount      = 1;

        VkImageView view;
        if (vkCreateImageView(Mi::Internal::mi_vulkancontext->device, &viewCreateInfo, nullptr, &view) != VK_SUCCESS) MI_ASSERT("Failed to create ImageView");

        return view;
    }
    
    /*
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    */

    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags flags) {

        for (VkFormat format : candidates) {
            VkFormatProperties properties;
            vkGetPhysicalDeviceFormatProperties(Mi::Internal::mi_vulkancontext->physicalDevice, format, &properties);

            if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & flags) == flags) {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & flags) == flags) {
                return format;
            }
        }
        MI_ASSERT("Failed to find a supported format");
    }

    /*
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    */

    VkFormat FindDepthFormat() {

        return FindSupportedFormat({
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT
        },  VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    /*
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    */

    bool HasStencilComponent(VkFormat format) {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    /*
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    */

    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(Mi::Internal::mi_vulkancontext->device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) MI_ASSERT("Failed to create buffer");

        VkMemoryRequirements mem;
        vkGetBufferMemoryRequirements(Mi::Internal::mi_vulkancontext->device, buffer, &mem);

        VkMemoryAllocateInfo allocate{};
        allocate.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocate.allocationSize = mem.size;
        allocate.memoryTypeIndex = Mi::Memory::FindMemoryType(mem.memoryTypeBits, properties);
        if (vkAllocateMemory(Mi::Internal::mi_vulkancontext->device, &allocate, nullptr, &bufferMemory) != VK_SUCCESS) MI_ASSERT("Could not allocate memory");

        vkBindBufferMemory(Mi::Internal::mi_vulkancontext->device, buffer, bufferMemory, 0);
    }

    /*
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    */
   
    void CopyBuffer(VkBuffer source, VkBuffer dst, VkDeviceSize size, VkCommandPool commandPool) {

        VkCommandBufferAllocateInfo allocate{};
        allocate.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocate.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocate.commandPool = commandPool;
        allocate.commandBufferCount = 1;
        
        VkCommandBuffer cmdbuf;
        vkAllocateCommandBuffers(Mi::Internal::mi_vulkancontext->device, &allocate, &cmdbuf);

        VkCommandBufferBeginInfo icmd{};
        icmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        icmd.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(cmdbuf, &icmd);
        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(cmdbuf, source, dst, 1, &copyRegion);
        vkEndCommandBuffer(cmdbuf);

        VkSubmitInfo submit{};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &cmdbuf;

        vkQueueSubmit(Mi::Internal::mi_vulkancontext->graphicsQueue, 1, &submit, VK_NULL_HANDLE);
        vkQueueWaitIdle(Mi::Internal::mi_vulkancontext->graphicsQueue);
        vkFreeCommandBuffers(Mi::Internal::mi_vulkancontext->device, commandPool, 1, &cmdbuf);
    }
}