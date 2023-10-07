namespace Mi::GlobalUtility {

    VkImage             depthImage;
    VkDeviceMemory      depthImageMemory;
    VkImageView         depthImageView;
    VkFormat            swapchainFormat;
    VkSampler           defaultTextureSampler;
    VkRenderPass        defaultRenderpass;
}