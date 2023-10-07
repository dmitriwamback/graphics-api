#include "buffer/subpass_dependencies.h"
#include "buffer/render_pass.h"

int MI_ENGINE_CURRENT_FRAME = 0;

#include "pipeline/image.h"
#include "pipeline/pipeline.h"

using Mi::Internal::mi_vulkancontext;

namespace Mi::Core::Engine {


    std::vector<Mi::Core::Engine::RenderPipeline> pipelines = std::vector<Mi::Core::Engine::RenderPipeline>();
    Mi::Core::Engine::RenderPipeline defaultPipeline;



    void Free() {

        /*
            DEFAULT PIPELINE
        */
        vkDestroyPipeline(              mi_vulkancontext->device, defaultPipeline.pipeline->pipeline,                      nullptr);
        vkDestroyPipelineLayout(        mi_vulkancontext->device, defaultPipeline.pipeline->content->layout,               nullptr);
        vkDestroyRenderPass(            mi_vulkancontext->device, defaultPipeline.pipeline->content->renderPass,           nullptr);
        vkDestroyDescriptorSetLayout(   mi_vulkancontext->device, defaultPipeline.pipeline->content->setLayoutDescriptor,  nullptr);
        //vkDestroyBuffer(                mi_vulkancontext->device, defaultPipeline.pipeline->context->vertexBuffer,         nullptr);
        //vkFreeMemory(                   mi_vulkancontext->device, defaultPipeline.pipeline->context->vertexBufferMemory,   nullptr);
        vkDestroyCommandPool(           mi_vulkancontext->device, defaultPipeline.commandPool,                             nullptr);
        
        for (int i = 0; i < MI_MAX_IN_FLIGHT_FRAMES; i++)
        {
            vkDestroySemaphore(mi_vulkancontext->device, defaultPipeline.imageSemaphores[i],    nullptr);
            vkDestroySemaphore(mi_vulkancontext->device, defaultPipeline.renderSemaphores[i],   nullptr);
            vkDestroyFence(mi_vulkancontext->device,     defaultPipeline.inFlightFence[i],          nullptr);
        }
        delete defaultPipeline.pipeline->content;
        delete defaultPipeline.pipeline->data;
        delete defaultPipeline.pipeline->context;


        /*
            OTHER PIPELINES
        */
        for (Mi::Core::Engine::RenderPipeline pipeline : pipelines)
        {
            vkDestroyPipeline(              mi_vulkancontext->device, pipeline.pipeline->pipeline,                      nullptr);
            vkDestroyPipelineLayout(        mi_vulkancontext->device, pipeline.pipeline->content->layout,               nullptr);
            vkDestroyRenderPass(            mi_vulkancontext->device, pipeline.pipeline->content->renderPass,           nullptr);
            vkDestroyDescriptorSetLayout(   mi_vulkancontext->device, pipeline.pipeline->content->setLayoutDescriptor,  nullptr);
            //vkDestroyBuffer(                mi_vulkancontext->device, pipeline.pipeline->context->vertexBuffer,         nullptr);
            //vkFreeMemory(                   mi_vulkancontext->device, pipeline.pipeline->context->vertexBufferMemory,   nullptr);
            vkDestroyCommandPool(           mi_vulkancontext->device, pipeline.commandPool,                             nullptr);
            
            for (int i = 0; i < MI_MAX_IN_FLIGHT_FRAMES; i++)
            {
                vkDestroySemaphore(mi_vulkancontext->device, pipeline.imageSemaphores[i],    nullptr);
                vkDestroySemaphore(mi_vulkancontext->device, pipeline.renderSemaphores[i],   nullptr);
                vkDestroyFence(mi_vulkancontext->device, pipeline.inFlightFence[i],          nullptr);
            }

            delete pipeline.pipeline->content;
            delete pipeline.pipeline->data;
            delete pipeline.pipeline->context;
        }
        for (VkFramebuffer fb : Mi::Internal::framebuffers) 
        {
            vkDestroyFramebuffer(mi_vulkancontext->device, fb, nullptr);
        }

        vkDestroySwapchainKHR(  mi_vulkancontext->device,       mi_vulkancontext->swapchain,                nullptr);
        vkDestroySampler(       mi_vulkancontext->device,       Mi::GlobalUtility::defaultTextureSampler,   nullptr);
        vkDestroyRenderPass(    mi_vulkancontext->device,       Mi::GlobalUtility::defaultRenderpass,       nullptr);
        vkDestroySurfaceKHR(    mi_vulkancontext->instance,     mi_vulkancontext->surface,                  nullptr);

        glfwDestroyWindow(mi_vulkancontext->window);
        glfwTerminate();
        delete mi_vulkancontext;
    }


    void MiMainLoop() {

        CreateDefaultTextureSampler();

        VkRenderPass renderpass = CreateDefaultRenderpass();
        Mi::GlobalUtility::defaultRenderpass = renderpass;
        Mi::Internal::LowLevel::CreateFramebuffers();

        Mi::Memory::uniformtest1 test{};
        test.unrelatedvariable = 100.f;
        
        defaultPipeline = RenderPipeline::Create("triangle");
        defaultPipeline.SetUniform(&test);

        do {
            defaultPipeline.Bind();
            MI_ENGINE_CURRENT_FRAME = (MI_ENGINE_CURRENT_FRAME + 1) & MI_MAX_IN_FLIGHT_FRAMES;
            glfwPollEvents();
        }
        while (!glfwWindowShouldClose(Mi::Internal::mi_vulkancontext->window));
        vkDeviceWaitIdle(Mi::Internal::mi_vulkancontext->device);
        Free();
    }
}