#pragma once
#ifndef __Node_H__
#define __Node_H__
#include "core/object/object.h"
#include "core/templates/vset.h"
#include "core/variant/typed_array.h"
#include "core/io/resource.h"
#include "core/string/string_name.h"
#include "node_res.h"
#include "node_path.h"
namespace lain {
    
    /// Node : Game Object base class
    class Node : Object
    {
        typedef VSet<StringName> TypeNameSet;

    public:
        Node() {}
        virtual ~Node();

        virtual void tick(float delta_time);

        bool load(const NodeInstanceRes& node_instance_res);
        void save(NodeInstanceRes& out_node_instance_res);

        void               setName(StringName name) { m_name = name; }
        const String& getName() const { return m_name; }

        bool hasComponent(const StringName& compenent_type_name) const;

        Vector<Reflection::ReflectionPtr<Component>> getComponents() { return m_components; }

        template<typename TComponent>
        TComponent* tryGetComponent(const String& compenent_type_name)
        {
            for (auto& component : m_components)
            {
                if (component.getTypeName() == compenent_type_name)
                {
                    return static_cast<TComponent*>(component.operator->());
                }
            }

            return nullptr;
        }

        template<typename TComponent>
        const TComponent* tryGetComponentConst(const String& compenent_type_name) const
        {
            for (const auto& component : m_components)
            {
                if (component.getTypeName() == compenent_type_name)
                {
                    return static_cast<const TComponent*>(component.operator->());
                }
            }
            return nullptr;
        }
        /// <summary>
        /// especially before or especially after other nodes
        /// </summary>
        enum InternalMode {
            INTERNAL_MODE_DISABLED,
            INTERNAL_MODE_FRONT,
            INTERNAL_MODE_BACK,
        };
#define tryGetComponent(COMPONENT_TYPE) tryGetComponent<COMPONENT_TYPE>(#COMPONENT_TYPE)
#define tryGetComponentConst(COMPONENT_TYPE) tryGetComponentConst<const COMPONENT_TYPE>(#COMPONENT_TYPE)

        /* NODE/TREE */

        StringName get_name() const;
        void set_name(const String& p_name);

        void add_child(Node* p_child, bool p_force_readable_name = false, InternalMode p_internal = INTERNAL_MODE_DISABLED);
        void add_sibling(Node* p_sibling, bool p_force_readable_name = false);
        void remove_child(Node* p_child);

        int get_child_count(bool p_include_internal = true) const;
        Node* get_child(int p_index, bool p_include_internal = true) const;
        TypedArray<Node> get_children(bool p_include_internal = true) const;
        bool has_node(const NodePath& p_path) const;
        Node* get_node(const NodePath& p_path) const;
        Node* get_node_or_null(const NodePath& p_path) const;
        Node* find_child(const String& p_pattern, bool p_recursive = true, bool p_owned = true) const;
        TypedArray<Node> find_children(const String& p_pattern, const String& p_type = "", bool p_recursive = true, bool p_owned = true) const;
        bool has_node_and_resource(const NodePath& p_path) const;
        Node* get_node_and_resource(const NodePath& p_path, Ref<Resource>& r_res, Vector<StringName>& r_leftover_subpath, bool p_last_is_property = true) const;

        virtual void reparent(Node* p_parent, bool p_keep_global_transform = true);
        Node* get_parent() const;
        Node* find_parent(const String& p_pattern) const;

        //Window* get_window() const;
        //Window* get_last_exclusive_window() const;



    protected:
        StringName m_name;
        String m_definition_url;

        // we have to use the ReflectionPtr due to that the components need to be reflected 
        // in editor, and it's polymorphism
        Vector<Reflection::ReflectionPtr<Component>> m_components;

    private:
        NodePath m_import_path;
        struct Data {
            Node* parent = nullptr;
            Node* owner = nullptr;
            HashMap<StringName, Node*> children;
        };


    };
}


#endif // !__NODE_H__
