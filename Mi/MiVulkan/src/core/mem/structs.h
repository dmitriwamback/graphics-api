namespace Mi::Memory {

    typedef struct mi_vkcxt 
    {
        VkInstance                  instance;
        VkPhysicalDevice            physicalDevice;
        VkDevice                    device;
        VkQueue                     graphicsQueue;
        VkSurfaceKHR                surface;
        VkSwapchainKHR              swapchain;
        VkFormat                    format;
        VkExtent2D                  extent;
        VkDebugUtilsMessengerEXT    debug;
        GLFWwindow*                 window;
        VkQueue                     presentQueue;

    } mi_vkcxt;




    typedef struct mi_pipelinecontent 
    {
        VkPipelineLayout    layout;
        VkRenderPass        renderPass;
        VkRect2D            scissor{};
        VkDescriptorSetLayoutBinding        uniformBufferLayoutBinding{};
        VkDescriptorSetLayoutCreateInfo     layoutInfo{};
        VkDescriptorSetLayout               setLayoutDescriptor{};

    } mi_pipelinecontent;

    typedef struct mi_pipelinecreateinfo
    {
        VkPipelineDynamicStateCreateInfo            dynamicState{};
        VkPipelineVertexInputStateCreateInfo        vertexInput{};
        VkPipelineInputAssemblyStateCreateInfo      inputAssembly{};
        VkPipelineViewportStateCreateInfo           viewportState{};
        VkPipelineRasterizationStateCreateInfo      rasterizer{};
        VkPipelineColorBlendStateCreateInfo         colorBlending{};
        uint32_t imageIndex;

    } mi_pipelinecreateinfo;

    typedef struct mi_uniformbuffermemory 
    {
        std::vector<VkBuffer>       uniformbuffers;
        std::vector<VkDeviceMemory> mem_uniformbuffers;
        std::vector<void*>          mem_mapped;

    } mi_uniformbuffermemory;

    typedef struct mi_pipelinecontextbuffer 
    {
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;

    } mi_pipelinecontextbuffer;






    typedef struct uniformtest1 
    {
        float unrelatedvariable;
    } uniformtest1;






    typedef struct mi_swapchaindetails 
    {
        VkSurfaceCapabilitiesKHR        capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR>   presentModes;

    } mi_swapchaindetails;

    typedef struct mi_queuefamily 
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentQueue;

    } mi_queuefamily;

    typedef struct mi_texturesamplerdependencies 
    {
        VkFilter min, mag;
        VkSamplerAddressMode u, v, w;
        VkSamplerMipmapMode mipmapMode;

    } mi_texturesamplerdependencies;


    typedef struct mi_shadermodule
    {
        VkShaderModule mVertex,
                       mFragment;
        VkPipelineShaderStageCreateInfo sVertex,
                                        sFragment;

    } mi_shadermodule;


    typedef struct mi_pipeline 
    {
        VkPipeline pipeline;

        mi_pipelinecontent*         content;
        mi_pipelinecreateinfo*      data;
        mi_uniformbuffermemory*     memory;
        mi_pipelinecontextbuffer*   context;

        mi_shadermodule* shader;
    } mi_pipeline;
}