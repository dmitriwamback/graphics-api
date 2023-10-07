namespace Mi::Core::Engine {

    class Vertex {
    public:
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;

        static VkVertexInputBindingDescription GetBindingDescription();
        static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescription();
    };

    VkVertexInputBindingDescription Vertex::GetBindingDescription() {

        VkVertexInputBindingDescription binding{};
        binding.binding     = 0;
        binding.stride      = sizeof(Vertex);
        binding.inputRate   = VK_VERTEX_INPUT_RATE_INSTANCE;
        return binding;
    }

    std::array<VkVertexInputAttributeDescription, 3> Vertex::GetAttributeDescription() {

        std::array<VkVertexInputAttributeDescription, 3> attribDesc{};

        attribDesc[0].binding       = 0;
        attribDesc[0].location      = 0;
        attribDesc[0].format        = VK_FORMAT_R32G32B32_SFLOAT;
        attribDesc[0].offset        = offsetof(Vertex, position);

        attribDesc[1].binding       = 0;
        attribDesc[1].location      = 1;
        attribDesc[1].format        = VK_FORMAT_R32G32B32_SFLOAT;
        attribDesc[1].offset        = offsetof(Vertex, normal);

        attribDesc[2].binding       = 0;
        attribDesc[2].location      = 2;
        attribDesc[2].format        = VK_FORMAT_R32G32_SFLOAT;
        attribDesc[2].offset        = offsetof(Vertex, uv);

        return attribDesc;
    }
}