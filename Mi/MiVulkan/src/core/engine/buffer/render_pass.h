namespace Mi::Core::Engine {

    VkRenderPass CreateDefaultRenderpass() {

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format          = Mi::Internal::mi_vulkancontext->format;
        colorAttachment.samples         = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp          = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp         = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.inputAttachmentCount = 0;
        subpass.pInputAttachments = nullptr;
        subpass.pResolveAttachments = nullptr;
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkRenderPassCreateInfo renderpassInfo{};
        renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderpassInfo.attachmentCount  = 1;
        renderpassInfo.subpassCount     = 1;
        renderpassInfo.pAttachments     = &colorAttachment;
        renderpassInfo.pSubpasses       = &subpass;

        VkRenderPass renderpass;
        vkCreateRenderPass(Mi::Internal::mi_vulkancontext->device, &renderpassInfo, nullptr, &renderpass);
        return renderpass;
    }

    
}