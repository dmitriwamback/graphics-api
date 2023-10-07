namespace Mi::Core::Engine {

    class Renderable {
    public:
        std::vector<Attribute> attributes;

        void Update();
        void AddAttribute(Attribute attr);
    private:
        bool hasRendererAttribute;
    };

    void Renderable::Update() {

        for (Attribute attr : attributes) 
        {

            if (static_cast<RendererAttribute*>(attr.function) == nullptr) {
                attr.function->Update();
                continue;
            } 

            RendererAttribute* r_attrib = static_cast<RendererAttribute*>(attr.function);
            r_attrib->Render();
        }
    }
    void Renderable::AddAttribute(Attribute attribute) {

        if (attributes.size() + 1 > 10) return;
        if (hasRendererAttribute && static_cast<RendererAttribute*>(attribute.function) != nullptr) return;

        attributes.push_back(attribute);
        hasRendererAttribute = static_cast<RendererAttribute*>(attribute.function) != nullptr ? true : false;
    }
}