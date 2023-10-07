namespace Mi::Core::Engine {

    class AttributeFunction {
    public:
        virtual void Render();
        virtual void Update();   
    };

    class RendererAttribute: public AttributeFunction {
    public:
        Material material;
        std::vector<Vertex> vertices;
    };

    class ObjectAttribute: public AttributeFunction {
    public:

    };
}