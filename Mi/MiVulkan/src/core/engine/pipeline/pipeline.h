namespace Mi::Core::Engine {


    /*
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        SHADERS
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    */

    std::vector<char> LoadShaderSource(std::string shaderPath) 
    {
        std::ifstream file(shaderPath, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            LOG("Cannot open file");
            return std::vector<char>();
        }

        size_t fileContentSize = (size_t)file.tellg();
        std::vector<char> sourcebuf(fileContentSize);

        file.seekg(0);      file.read(sourcebuf.data(), fileContentSize);      file.close();
        return sourcebuf;

    }

    VkShaderModule CreateShaderModule(std::vector<char> shaderSource) 
    {
        VkShaderModule shader;
        VkShaderModuleCreateInfo shaderCreateInfo{};
        shaderCreateInfo.sType      = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderCreateInfo.codeSize   = shaderSource.size();
        shaderCreateInfo.pCode      = reinterpret_cast<const uint32_t*>(shaderSource.data());

        if (vkCreateShaderModule(Mi::Internal::mi_vulkancontext->device, &shaderCreateInfo, nullptr, &shader) != VK_SUCCESS) MI_ASSERT("Failed to compile shader");

        return shader;
    }

    Mi::Memory::mi_shadermodule* CreateShaderModule(std::string shaderFolder) 
    {
        std::string vertexShaderPath    = "res/shaders/compiled/" + shaderFolder + "/vMain.spv";
        std::string fragmentShaderPath  = "res/shaders/compiled/" + shaderFolder + "/fMain.spv";

        std::vector<char> vertexShaderSource    = LoadShaderSource(vertexShaderPath),
                          fragmentShaderSource  = LoadShaderSource(fragmentShaderPath);

        VkShaderModule vertex = CreateShaderModule(vertexShaderSource), 
                       fragment = CreateShaderModule(fragmentShaderSource);

        VkPipelineShaderStageCreateInfo vertexInfo{}, fragmentInfo{};

        vertexInfo.sType    = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertexInfo.stage    = VK_SHADER_STAGE_VERTEX_BIT;
        vertexInfo.module   = vertex;
        vertexInfo.pName    = "main";

        fragmentInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragmentInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentInfo.module = fragment;
        fragmentInfo.pName  = "main";

        Mi::Memory::mi_shadermodule* module = static_cast<Mi::Memory::mi_shadermodule*>(malloc(sizeof(Mi::Memory::mi_shadermodule)));
        module->mVertex     = vertex;
        module->mFragment   = fragment;
        module->sVertex     = vertexInfo;
        module->sFragment   = fragmentInfo;

        return module;
    }




    /*
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        PIPELINE INITIALIZERS
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    */

    Mi::Memory::mi_pipelinecontent* CreatePipelineContent() {

        Mi::Memory::mi_pipelinecontent* content = static_cast<Mi::Memory::mi_pipelinecontent*>(malloc(sizeof(Mi::Memory::mi_pipelinecontent)));

        VkRenderPass renderpass = Mi::Core::Engine::CreateDefaultRenderpass();
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};

        content->renderPass = renderpass;

        content->uniformBufferLayoutBinding.binding                 = 0;
        content->uniformBufferLayoutBinding.descriptorCount         = 1;
        content->uniformBufferLayoutBinding.descriptorType          = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        content->uniformBufferLayoutBinding.pImmutableSamplers      = nullptr;
        content->uniformBufferLayoutBinding.stageFlags              = VK_SHADER_STAGE_VERTEX_BIT;

        content->layoutInfo.sType           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        content->layoutInfo.bindingCount    = 1;
        content->layoutInfo.pBindings       = &content->uniformBufferLayoutBinding;
        content->layoutInfo.flags           = 0;
        content->layoutInfo.pNext           = nullptr;

        content->scissor.offset = {0, 0};
        content->scissor.extent = Mi::Internal::mi_vulkancontext->extent;

        if (vkCreateDescriptorSetLayout(Mi::Internal::mi_vulkancontext->device, &content->layoutInfo, nullptr, &content->setLayoutDescriptor) != VK_SUCCESS) MI_ASSERT("Failed to create VkDescriptorSetLayout");

        pipelineLayoutCreateInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts    = &content->setLayoutDescriptor;
        
        if (vkCreatePipelineLayout(Mi::Internal::mi_vulkancontext->device, &pipelineLayoutCreateInfo, nullptr, &content->layout) != VK_SUCCESS) MI_ASSERT("Failed to create VkPipelineLayout");

        return content;
    }

    Mi::Memory::mi_pipelinecreateinfo* CreatePipelineInfo() {

        Mi::Memory::mi_pipelinecreateinfo* info = static_cast<Mi::Memory::mi_pipelinecreateinfo*>(malloc(sizeof(Mi::Memory::mi_pipelinecreateinfo)));

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width  = (float)Mi::Internal::mi_vulkancontext->extent.width;
        viewport.height = (float)Mi::Internal::mi_vulkancontext->extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = Mi::Internal::mi_vulkancontext->extent;

        VkPipelineColorBlendAttachmentState color{};
        color.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        color.blendEnable = VK_FALSE;


        info->dynamicState.sType                                = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        info->dynamicState.dynamicStateCount                    = (uint32_t)(dynamicStates.size());
        info->dynamicState.pDynamicStates                       = &dynamicStates[0];
        info->dynamicState.flags                                = 0;
        info->dynamicState.pNext                                = nullptr;

        info->vertexInput.sType                                 = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        info->vertexInput.vertexBindingDescriptionCount         = 0;
        info->vertexInput.vertexAttributeDescriptionCount       = 0;
        info->vertexInput.pVertexAttributeDescriptions          = nullptr;
        info->vertexInput.pVertexBindingDescriptions            = nullptr;
        info->vertexInput.flags                                 = 0;
        info->vertexInput.pNext                                 = nullptr;

        info->inputAssembly.sType                               = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        info->inputAssembly.topology                            = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        info->inputAssembly.primitiveRestartEnable              = VK_FALSE;
        info->inputAssembly.flags                               = 0;
        info->inputAssembly.pNext                               = nullptr;

        info->viewportState.sType                               = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        info->viewportState.viewportCount                       = 1;
        info->viewportState.scissorCount                        = 1;
        info->viewportState.pViewports                          = &viewport;
        info->viewportState.pScissors                           = &scissor;
        info->viewportState.flags                               = 0;
        info->viewportState.pNext                               = nullptr;

        info->rasterizer.sType                                  = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        info->rasterizer.depthClampEnable                       = VK_FALSE;
        info->rasterizer.rasterizerDiscardEnable                = VK_FALSE;
        info->rasterizer.polygonMode                            = VK_POLYGON_MODE_FILL;
        info->rasterizer.lineWidth                              = 1;
        info->rasterizer.cullMode                               = VK_CULL_MODE_BACK_BIT;
        info->rasterizer.frontFace                              = VK_FRONT_FACE_CLOCKWISE;
        info->rasterizer.depthBiasEnable                        = VK_FALSE;
        info->rasterizer.depthBiasConstantFactor                = 0;
        info->rasterizer.depthBiasClamp                         = 0;
        info->rasterizer.depthBiasSlopeFactor                   = 0;
        info->rasterizer.flags                                  = 0;
        info->rasterizer.pNext                                  = nullptr;

        info->colorBlending.sType                               = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        info->colorBlending.logicOpEnable                       = VK_FALSE;
        info->colorBlending.logicOp                             = VK_LOGIC_OP_COPY;
        info->colorBlending.attachmentCount                     = 1;
        info->colorBlending.pAttachments                        = &color;
        info->colorBlending.blendConstants[0]                   = 0.0f;
        info->colorBlending.blendConstants[1]                   = 0.0f;
        info->colorBlending.blendConstants[2]                   = 0.0f;
        info->colorBlending.blendConstants[3]                   = 0.0f;
        info->colorBlending.flags                               = 0;
        info->colorBlending.pNext                               = nullptr;

        return info;
    }









    /*
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        PIPELINE
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    */

    class RenderPipeline {
    public:
        Mi::Memory::mi_pipeline*            pipeline;
        Mi::Memory::mi_shadermodule         shader;
        Mi::Memory::mi_uniformbuffermemory  uniformbuf;

        void* rawuniformbuffer;

        VkQueue presentQueue;
        VkCommandPool commandPool;
        static RenderPipeline Create(std::string shaderFolder);

        void AddRenderable();
        void Bind();
        void SetUniform(void* uniform);
        void UpdateUniformBuffer(int image);

        std::vector<VkCommandBuffer>    commandBuffers      = std::vector<VkCommandBuffer>();
        std::vector<VkSemaphore>        imageSemaphores     = std::vector<VkSemaphore>();
        std::vector<VkSemaphore>        renderSemaphores    = std::vector<VkSemaphore>();
        std::vector<VkFence>            inFlightFence       = std::vector<VkFence>();
    };




    RenderPipeline RenderPipeline::Create(std::string shaderFolder) 
    {

        RenderPipeline pipeline = RenderPipeline();
        pipeline.uniformbuf = {};

        Mi::Memory::mi_pipeline* p_core = static_cast<Mi::Memory::mi_pipeline*>(malloc(sizeof(Mi::Memory::mi_pipeline)));
        p_core->content = CreatePipelineContent();
        p_core->data    = CreatePipelineInfo();
        p_core->shader  = CreateShaderModule(shaderFolder);
        p_core->context = static_cast<Mi::Memory::mi_pipelinecontextbuffer*>(malloc(sizeof(Mi::Memory::mi_pipelinecontextbuffer)));

        /*
        ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        */

        pipeline.renderSemaphores.resize(MI_MAX_IN_FLIGHT_FRAMES);
        pipeline.imageSemaphores .resize(MI_MAX_IN_FLIGHT_FRAMES);
        pipeline.inFlightFence   .resize(MI_MAX_IN_FLIGHT_FRAMES);
        pipeline.commandBuffers  .resize(MI_MAX_IN_FLIGHT_FRAMES);

        Mi::Memory::mi_queuefamily family = Mi::Internal::LowLevel::FindQueueFamily(Internal::mi_vulkancontext->physicalDevice);
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = family.graphicsFamily.value();

        if (vkCreateCommandPool(Mi::Internal::mi_vulkancontext->device, &poolInfo, nullptr, &pipeline.commandPool) != VK_SUCCESS) MI_ASSERT("Failed to create VkCommandPool");

        VkCommandBufferAllocateInfo commandBufferAlloc{};
        commandBufferAlloc.sType                = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAlloc.commandBufferCount   = MI_MAX_IN_FLIGHT_FRAMES;
        commandBufferAlloc.level                = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAlloc.commandPool          = pipeline.commandPool;

        if (vkAllocateCommandBuffers(Mi::Internal::mi_vulkancontext->device, &commandBufferAlloc, pipeline.commandBuffers.data()) != VK_SUCCESS) MI_ASSERT("Failed to allocate VkCommandBuffer(s)");

        VkFenceCreateInfo fenceCreateInfo{};

        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkSemaphoreCreateInfo semaphoreCreateInfo{};

        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        for (int i = 0; i < MI_MAX_IN_FLIGHT_FRAMES; i++)
        {
            if (vkCreateSemaphore(Mi::Internal::mi_vulkancontext->device, &semaphoreCreateInfo, nullptr, &pipeline.imageSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(Mi::Internal::mi_vulkancontext->device, &semaphoreCreateInfo, nullptr, &pipeline.renderSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(Mi::Internal::mi_vulkancontext->device, &fenceCreateInfo, nullptr, &pipeline.inFlightFence[i]) != VK_SUCCESS)
            {
                MI_ASSERT("Failed to create VkSemaphore or VkFence");
            }
        }

        /*
        ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        */
        VkGraphicsPipelineCreateInfo pipelineInfo{};

        VkPipelineShaderStageCreateInfo stages[] = {
            p_core->shader->sVertex,
            p_core->shader->sFragment
        };

        pipelineInfo.sType                  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount             = 2;
        pipelineInfo.pVertexInputState      = &p_core->data->vertexInput;
        pipelineInfo.pInputAssemblyState    = &p_core->data->inputAssembly;
        pipelineInfo.pViewportState         = &p_core->data->viewportState;
        pipelineInfo.pRasterizationState    = &p_core->data->rasterizer;
        pipelineInfo.pColorBlendState       = &p_core->data->colorBlending;
        pipelineInfo.pDynamicState          = &p_core->data->dynamicState;

        pipelineInfo.layout                 = p_core->content->layout;
        pipelineInfo.renderPass             = p_core->content->renderPass;
        pipelineInfo.subpass                = 0;
        pipelineInfo.basePipelineHandle     = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex      = -1;
        pipelineInfo.pStages                = stages;

        if (vkCreateGraphicsPipelines(Mi::Internal::mi_vulkancontext->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &p_core->pipeline) != VK_SUCCESS) MI_ASSERT("Failed to create GraphicsPipeline [RenderPipeline::Create]");
        pipeline.pipeline = p_core;
        return pipeline;
    }




    void RenderPipeline::Bind() {
        
        vkWaitForFences(Mi::Internal::mi_vulkancontext->device, 1, &inFlightFence[MI_ENGINE_CURRENT_FRAME], VK_TRUE, UINT64_MAX);
        vkResetFences(Mi::Internal::mi_vulkancontext->device, 1, &inFlightFence[MI_ENGINE_CURRENT_FRAME]);

        VkCommandBufferBeginInfo cmdBufferInfo{};
        cmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBufferInfo.flags = 0;
        cmdBufferInfo.pInheritanceInfo = nullptr;

        VkResult acquireNexImage = vkAcquireNextImageKHR(Mi::Internal::mi_vulkancontext->device, Mi::Internal::mi_vulkancontext->swapchain, UINT64_MAX, imageSemaphores[MI_ENGINE_CURRENT_FRAME], VK_NULL_HANDLE, &pipeline->data->imageIndex);
        if (acquireNexImage == VK_ERROR_OUT_OF_DATE_KHR)
        {
            
        }
        else if (acquireNexImage != VK_SUCCESS && acquireNexImage != VK_SUBOPTIMAL_KHR) MI_ASSERT("Failed to acquire swapchain");


        vkResetCommandBuffer(commandBuffers[MI_ENGINE_CURRENT_FRAME], 0);
        if (vkBeginCommandBuffer(commandBuffers[MI_ENGINE_CURRENT_FRAME], &cmdBufferInfo) != VK_SUCCESS) MI_ASSERT("Failed to begin command buffer");

        /*
        ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        */

        VkClearValue clearColor = {{0.f, 0.f, 0.f, 1.f}};

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType                = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass           = pipeline->content->renderPass;
        renderPassInfo.framebuffer          = Mi::Internal::framebuffers[pipeline->data->imageIndex];
        renderPassInfo.renderArea.offset    = {0,0};
        renderPassInfo.renderArea.extent    = Mi::Internal::mi_vulkancontext->extent;
        renderPassInfo.clearValueCount      = 1;
        renderPassInfo.pClearValues         = &clearColor;

        vkCmdBeginRenderPass(commandBuffers[MI_ENGINE_CURRENT_FRAME], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffers[MI_ENGINE_CURRENT_FRAME], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);

        /*
        ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        */

        VkViewport viewport{};
        viewport.x              = 0.0f;
        viewport.y              = 0.0f;
        viewport.width          = static_cast<float>(Mi::Internal::mi_vulkancontext->extent.width);
        viewport.height         = static_cast<float>(Mi::Internal::mi_vulkancontext->extent.height);
        viewport.minDepth       = 0.0f;
        viewport.maxDepth       = 1.0f;
        vkCmdSetViewport(commandBuffers[MI_ENGINE_CURRENT_FRAME], 0, 1, &viewport);
        vkCmdSetScissor(commandBuffers[MI_ENGINE_CURRENT_FRAME], 0, 1, &pipeline->content->scissor);

        vkCmdEndRenderPass(commandBuffers[MI_ENGINE_CURRENT_FRAME]);
        if (vkEndCommandBuffer(commandBuffers[MI_ENGINE_CURRENT_FRAME]) != VK_SUCCESS) MI_ASSERT("Failed to record VkCommandBuffer");


        /*
        ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        */

       UpdateUniformBuffer(pipeline->data->imageIndex);

        VkSubmitInfo submitInfo{};

        VkSemaphore waitSemaphore[] = { imageSemaphores[MI_ENGINE_CURRENT_FRAME] };
        VkSemaphore signalSemaphore[] = { renderSemaphores[MI_ENGINE_CURRENT_FRAME] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        submitInfo.sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount       = 1;
        submitInfo.pWaitSemaphores          = waitSemaphore;
        submitInfo.pWaitDstStageMask        = waitStages;
        submitInfo.commandBufferCount       = 1;
        submitInfo.pCommandBuffers          = &commandBuffers[MI_ENGINE_CURRENT_FRAME];
        submitInfo.signalSemaphoreCount     = 1;
        submitInfo.pSignalSemaphores        = signalSemaphore;
        
        vkQueueSubmit(Mi::Internal::mi_vulkancontext->graphicsQueue, 1, &submitInfo, inFlightFence[MI_ENGINE_CURRENT_FRAME]);

        /*
        ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        */

        VkPresentInfoKHR presentInfo{};
        VkSwapchainKHR swapChains[] = {Mi::Internal::mi_vulkancontext->swapchain};

        presentInfo.sType                   = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount      = 1;
        presentInfo.pWaitSemaphores         = signalSemaphore;
        presentInfo.swapchainCount          = 1;
        presentInfo.pSwapchains             = swapChains;
        presentInfo.pImageIndices           = &pipeline->data->imageIndex;

        vkQueuePresentKHR(Mi::Internal::mi_vulkancontext->presentQueue, &presentInfo);

        /*
        ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        */
    }


    void RenderPipeline::SetUniform(void* uniform) {

        VkDeviceSize uniformBufferSize = sizeof(uniform);
        rawuniformbuffer = uniform;

        uniformbuf.uniformbuffers    .resize(MI_MAX_IN_FLIGHT_FRAMES);
        uniformbuf.mem_mapped        .resize(MI_MAX_IN_FLIGHT_FRAMES);
        uniformbuf.mem_uniformbuffers.resize(MI_MAX_IN_FLIGHT_FRAMES);

        for (int i = 0; i < MI_MAX_IN_FLIGHT_FRAMES; i++)
        {
            Mi::Memory::CreateBuffer(uniformBufferSize, 
                                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                                     uniformbuf.uniformbuffers[i], 
                                     uniformbuf.mem_uniformbuffers[i]);

            vkMapMemory(Mi::Internal::mi_vulkancontext->device, 
                        uniformbuf.mem_uniformbuffers[i], 
                        0, 
                        uniformBufferSize, 
                        0, 
                        &uniformbuf.mem_mapped[i]);
        }
    }

    void RenderPipeline::UpdateUniformBuffer(int image) {

        Mi::Memory::uniformtest1 test{};
        test.unrelatedvariable = 100.f;

        memcpy(uniformbuf.mem_mapped[image], &test, sizeof(test));
    }
}