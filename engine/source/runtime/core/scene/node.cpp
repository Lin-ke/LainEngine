#include "node.h"
namespace lain {
	bool Node::hasComponent(const StringName& compenent_type_name) const {
        for (const auto& component : m_components)
        {
            if (component.getTypeName() == compenent_type_name)
                return true;
        }

        return false;
	}
    bool Node::load(const NodeInstanceRes& Node_instance_res)
    {
        // clear old components
        m_components.clear();

        setName(Node_instance_res.m_name);

        // load Node instanced components
        m_components = Node_instance_res.m_instanced_components;
        for (auto component : m_components)
        {
            if (component)
            {
                //component->postLoadResource(weak_from_this());
            }
        }

        // load Node definition components
        m_definition_url = Node_instance_res.m_definition;

        NodeDefinitionRes definition_res;

        const bool is_loaded_success = g_runtime_global_context.m_asset_manager->loadAsset(m_definition_url, definition_res);
        if (!is_loaded_success)
            return false;

        for (auto loaded_component : definition_res.m_components)
        {
            const std::string type_name = loaded_component.getTypeName();
            // don't create component if it has been instanced
            if (hasComponent(type_name))
                continue;

            loaded_component->postLoadResource(weak_from_this());

            m_components.push_back(loaded_component);
        }

        return true;
    }

}