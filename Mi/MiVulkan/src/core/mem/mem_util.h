namespace Mi::Memory {

    uint32_t FindMemoryType(uint32_t filter, VkMemoryPropertyFlags properties) {

        VkPhysicalDeviceMemoryProperties mem;
        vkGetPhysicalDeviceMemoryProperties(Mi::Internal::mi_vulkancontext->physicalDevice, &mem);

        for (uint32_t i = 0; i < mem.memoryTypeCount; i++) {
            if (filter & (1 << i) && (mem.memoryTypes[i].propertyFlags & properties) == properties) return i;
        }
        MI_ASSERT("Memory Type");
    }
}