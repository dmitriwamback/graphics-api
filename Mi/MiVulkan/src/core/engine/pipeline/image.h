namespace Mi::Core::Engine {

    std::map<std::string, VkSampler> uniqueSamplers;

    void CreateDefaultTextureSampler() {

        VkSamplerCreateInfo samplerInfo{};

        VkPhysicalDeviceProperties physicalDeviceProperties{};
        vkGetPhysicalDeviceProperties(Mi::Internal::mi_vulkancontext->physicalDevice, &physicalDeviceProperties);

        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;

        samplerInfo.addressModeU        = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV        = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW        = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable    = VK_TRUE;

        samplerInfo.maxAnisotropy               = physicalDeviceProperties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor                 = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates     = VK_FALSE;
        samplerInfo.compareEnable               = VK_FALSE;
        samplerInfo.compareOp                   = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode          = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias          = 0;
        samplerInfo.minLod              = 0;
        samplerInfo.maxLod              = 0;

        if (vkCreateSampler(Mi::Internal::mi_vulkancontext->device, &samplerInfo, nullptr, &Mi::GlobalUtility::defaultTextureSampler) != VK_SUCCESS) MI_ASSERT("Failed to create default texture sampler");
    }
}