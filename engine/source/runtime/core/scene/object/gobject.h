#pragma once
#ifndef __GOBJECT_H__
#define __GOBJECT_H__
#include "core/object/object.h"
#include "gobject_path.h"
#include "core/templates/local_vector.h"
#include "core/scene/scene_tree.h"
namespace lain {
    class Component;
    class Viewport;
    class SceneState;

    class GObject: public Object{
        LCLASS(GObject, Object);
    public:
        enum InternalMode {
            INTERNAL_MODE_DISABLED,
            INTERNAL_MODE_FRONT,
            INTERNAL_MODE_BACK,
        }; // 在siblings强制靠前/靠后
        struct GroupData {
            bool persistent = false;
            SceneTree::Group* group = nullptr;
        };
        struct Data {
            String scene_file_path; // 如果场景是从文件实例化来的
            Ref<SceneState> instance_state;

            int depth = -1;
            StringName name;
            GObject* parent;
            GObject* owner; // 记录谁实例化了什么
            bool is_prefab = false;

            HashMap<StringName, GObject*> children;
            Vector<Component*> components; // Reflection::ReflectionPtr<Component>
            HashMap<StringName, GObject*> owned_unique_gobjects; // 儿子中使用%访问，无需路径
            bool unique_name_in_owner = false;

            // chidrencache，不常用hashmap
            mutable bool children_cache_dirty = true;
            mutable LocalVector<GObject*> children_cache;
            mutable int index = -1; // relative to front, normal or back. 与chidren排序有关
            InternalMode internal_mode = INTERNAL_MODE_DISABLED;
            mutable int internal_children_front_count_cache = 0;
            mutable int internal_children_back_count_cache = 0;
            mutable int external_children_count_cache = 0;

            /// process bools
            bool physics_process : 1;
            bool process : 1;

            bool physics_process_internal : 1;
            bool process_internal : 1;

            bool input : 1;
            bool shortcut_input : 1;
            bool unhandled_input : 1;
            bool unhandled_key_input : 1;
            
            bool inside_tree : 1;
            bool ready_notified : 1;
            bool ready_first : 1;
            mutable GObjectPath* path_cache = nullptr;
            int blocked = 0; // Safeguard that throws an error when attempting to modify the tree in a harmful way while being traversed.
            SceneTree* tree;

            List<GObject*>::Element* OW = nullptr; // Owned element. head?
            List<GObject*> owned; 
            HashMap<StringName, GroupData> grouped; // 节点可以加入到组中，便于管理

            // view
            Viewport* viewport = nullptr;

        } data; // 防止名称冲突

        Vector<Component*> get_components() { return data.components; }
        template<typename TComponent>
        TComponent* try_get_component(const String& compenent_type_name)
        {
            for (auto& component : m_components)
            {
                if (component.get_class_name() == compenent_type_name)
                {
                    return static_cast<TComponent*>(component.operator->());
                }
            }

            return nullptr;
        }
        int get_index() const { return data.index; }
        StringName get_name() const { return data.name; }
        GObject* get_owner() const { return data.owner; }
        GObject* get_parent() const { return data.parent; }
        GObject* find_parent(const String& p_pattern) const;
        bool has_gobject(const GObjectPath& p_path) const;
        GObject* get_gobject_or_null(const GObjectPath& p_path) const;
        GObject* find_child(const String& pattern, bool p_recuresive, bool p_owned) const;
        GObjectPath get_path() const {}
        void set_scene_instance_state(Ref<SceneState> p) { data.instance_state = p; }
        void set_scene_file_path(String p_path) { data.scene_file_path = p_path; }
        bool is_ancestor_of(const GObject*);
        bool is_unique_name_in_owner() const;
        bool is_inside_tree() { return data.inside_tree; }
        void add_child(GObject* p_child, bool p_force_readable_name, InternalMode p_internal);
        void add_sibling(GObject* p_sibling, bool p_force_readable_name);
        void remove_child(GObject* p_child);
        void add_component(Component*); // 还有一套和children对应的组件
        void remove_component(Component* p_child);

        void set_owner(GObject*);
        GObject* get_owner() { return data.owner; }
        GObjectPath get_path_to(const GObject*, bool) const;
        void add_to_group(const StringName&, bool);
    private:
        _FORCE_INLINE_ void _update_children_cache() const {
            if (unlikely(data.children_cache_dirty)) {
                _update_children_cache_impl();
            }
        }

        void _update_children_cache_impl() const;

        void _set_owner_nocheck(GObject*);
        void _add_child_nocheck(GObject* p_child, const StringName& p_name, InternalMode p_internal_mode);
        void _move_child(GObject* p_child, int p_index, bool p_ignore_end = false);
        void _clean_up_owner();
        void _release_unique_name_in_owner();
        void _acquire_unique_name_in_owner();
        void _validate_child_name(GObject* p_child, bool p_force_readable_name);
        void _set_tree(SceneTree* tree);
        // 使用传播函数以完成修改
        void _propagate_enter_tree();
        void _propagate_exit_tree();
        void _propagate_ready();
        void _propagate_groups_dirty();
        void _propagate_after_exit_tree();
        struct ComparatorByIndex {
            bool operator()(const GObject* p_left, const GObject* p_right) const {
                static const uint32_t order[3] = { 1, 0, 2 };
                uint32_t order_left = order[p_left->data.internal_mode];
                uint32_t order_right = order[p_right->data.internal_mode];
                if (order_left == order_right) {
                    return p_left->data.index < p_right->data.index;
                }
                return order_left < order_right;
            }
        };

	};
}

#endif