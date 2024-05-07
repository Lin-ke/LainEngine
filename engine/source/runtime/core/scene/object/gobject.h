#pragma once
#ifndef __GOBJECT_H__
#define __GOBJECT_H__
#include "core/object/object.h"
#include "gobject_path.h"
#include "core/templates/local_vector.h"
#include "core/scene/scene_tree.h"
#include "core/scene/tick_object.h"
#include "core/scene/packed_scene.h"
#include "core/meta/reflection/reflection_marcos.h"
namespace lain {
    class Component;
    class Viewport;
    class GObject;
    class SceneState;
    class Serializer;
    SAFE_FLAG_TYPE_PUN_GUARANTEES
        SAFE_NUMERIC_TYPE_PUN_GUARANTEES(uint32_t)

        // 1. Parent; Children; Sibling������ϵ
        // 2. Owner,���ϵĽڵ㣬�����б������
        // 3. Grouped, ����tag�������ϵ
        // 4. Process ���֣���ProcessGroup��ϵ
    REFLECTION_TYPE(GObject);
    CLASS(GObject: public TickObject, WhiteListFields){
        friend class Serializer;
        LCLASS(GObject, Object);
        friend class SceneState;
        friend class SceneTree;
        friend void register_core_types();
    public:
        enum InternalMode {
            INTERNAL_MODE_DISABLED,
            INTERNAL_MODE_FRONT,
            INTERNAL_MODE_BACK,
        }; // ��siblingsǿ�ƿ�ǰ/����
    
        struct GroupData {
            bool persistent = false;
            SceneTree::Group* group = nullptr;
        };
        struct Data {
            String scene_file_path; // ��������Ǵ��ļ�ʵ��������
            Ref<SceneState> instance_state; // ʵ��������scenestate
            Ref<SceneState> inherited_state; // �̳г�����scenestate


            int depth = -1;
            StringName name;
            GObject* parent = nullptr;
            GObject* owner = nullptr; // ��¼˭ʵ������ʲô
            //bool is_prefab = false; // ��scene_file_pathӦ������һ��
            bool editable_instance : 1; // ��
            bool display_folded : 1;    // ��
            
            bool parent_owned : 1;
            bool in_constructor : 1;
            bool use_placeholder : 1;
            
            HashMap<StringName, GObject*> children;
            HashMap<StringName, Component*> components;  // ÿ��component ���һ��;  ������-components

            HashMap<StringName, GObject*> owned_unique_gobjects; // ������ʹ��%���ʣ�����·��
            bool unique_name_in_owner = false;
            // chidrencache��������hashmap
            mutable bool children_cache_dirty = true;
            mutable bool components_cache_dirty= true;

            mutable LocalVector<GObject*> children_cache;
            mutable LocalVector<Component*> components_cache; // idx get

            mutable int index = -1; // relative to front, normal or back. ��chidren�����й�
            InternalMode internal_mode = INTERNAL_MODE_DISABLED;
            mutable int internal_children_front_count_cache = 0;
            mutable int internal_children_back_count_cache = 0;
            mutable int external_children_count_cache = 0;

          

            bool input : 1;
            bool shortcut_input : 1;
            bool unhandled_input : 1;
            bool unhandled_key_input : 1;
            
            bool inside_tree : 1;
            bool ready_notified : 1;
            bool ready_first : 1;
            mutable GObjectPath* path_cache = nullptr;
            int blocked = 0; // Safeguard that throws an error when attempting to modify the tree in a harmful way while being traversed.
            SceneTree* tree = nullptr;

            List<GObject*>::Element* OW = nullptr; // Owned element. head?
            List<GObject*> owned; 
            HashMap<StringName, GroupData> grouped; // �ڵ���Լ��뵽���У����ڹ���
            // ���groupΪɶ����sceneTree�е�

            // view
            Viewport* viewport = nullptr;
            // ?
            


        } data; // ��ֹ���Ƴ�ͻ
       
        
        GObject* get_child(int p_index, bool p_include_internal = true) const;

        L_INLINE String get_scene_file_path() const { return data.scene_file_path; }
        L_INLINE int get_index() const { return data.index; }
        L_INLINE StringName get_name() const { return data.name; }
        void set_name(const String& p_name);
        L_INLINE GObject* get_owner() const { return data.owner; }
        L_INLINE GObject* get_parent() const { return data.parent; }
        GObject* find_parent(const String& p_pattern) const;
        bool has_gobject(const GObjectPath& p_path) const;
        GObject* get_gobject_or_null(const GObjectPath& p_path) const;
        void set_editable_instance(GObject* p_node, bool p_editable);

        L_INLINE Ref<SceneState> get_scene_inherited_state()const { return data.inherited_state; }
        L_INLINE Ref<SceneState> get_scene_instance_state() const { return data.instance_state; }
        GObject* find_child(const String& pattern, bool p_recuresive, bool p_owned) const;
        GObjectPath get_path() const;
        L_INLINE void set_scene_instance_state(Ref<SceneState> p) { data.instance_state = p; }
        L_INLINE void set_scene_file_path(String p_path) { data.scene_file_path = p_path; }
        bool is_ancestor_of(const GObject*) const;
        bool is_unique_name_in_owner() const;
        virtual bool is_inside_tree() const override { return data.inside_tree; }
        _FORCE_INLINE_ virtual SceneTree* get_tree() const override {
            ERR_FAIL_NULL_V(data.tree, nullptr);
            return data.tree;
        }
        bool is_editable_instance(const GObject*) const;
        bool is_greater_than(const GObject*) const;
        ///@TODO: place holder
        L_INLINE bool get_scene_instance_load_placeholder() const { return false; }
        void add_child(GObject* p_child, bool p_force_readable_name = false, InternalMode p_internal = INTERNAL_MODE_DISABLED);
        void add_sibling(GObject* p_sibling, bool p_force_readable_name = false);
        void remove_child(GObject* p_child);
        template<typename T>
        Component* get_component() const {
            return get_component(*T._get_class_namev());
        }

        Component* get_component(const StringName& type_name) const;
        Component* get_component(int p_index) const;
        Vector<Component*> get_components() const;
        int get_component_count() const;
        void add_component(Component*); // ����һ�׺�children��Ӧ�����
        // @TODO
        void remove_component(Component* p_child);
        void move_component(Component* p_component, int p_index) ;

        void set_owner(GObject*);
        void set_scene_inherited_state(Ref<SceneState> p_scene_state);
        L_INLINE GObject* get_owner() { return data.owner; }
        GObjectPath get_path_to(const GObject*, bool use_unique_path = false) const;
        void add_to_group(const StringName&, bool);
        int get_child_count(bool p_include_internal = true) const;
        void move_child(GObject* p_child, int p_index);
        
       

        static int orphan_node_count;
        // WTODO: Process group management ���������
        
        void _add_all_components_to_ptg();
        void _remove_all_components_from_ptg();

        void _add_components_to_ptg();
        // һ��ָ���һ������ָ��Ĵ�С��һ���İ�

        void _remove_tree_from_process_thread_group();
        void _add_tree_to_process_thread_group(GObject* p_owner);

        static thread_local TickObject* current_process_thread_group;

        // process

        GObject();
        ~GObject();

        // notification����
        // notification���ƿ����������������Ҫ����
        // �������notification��Ϊ���е�notification��
        enum {
            // You can make your own, but don't use the same numbers as other notifications in other nodes.
            NOTIFICATION_ENTER_TREE = 10,
            NOTIFICATION_EXIT_TREE = 11,
            NOTIFICATION_MOVED_IN_PARENT = 12,
            NOTIFICATION_READY = 13,
            NOTIFICATION_PAUSED = 14,
            NOTIFICATION_UNPAUSED = 15,
            NOTIFICATION_PHYSICS_PROCESS = 16,
            NOTIFICATION_PROCESS = 17,
            NOTIFICATION_PARENTED = 18,
            NOTIFICATION_UNPARENTED = 19,
            NOTIFICATION_SCENE_INSTANTIATED = 20,
            NOTIFICATION_DRAG_BEGIN = 21,
            NOTIFICATION_DRAG_END = 22,
            NOTIFICATION_PATH_RENAMED = 23,
            NOTIFICATION_CHILD_ORDER_CHANGED = 24,
            NOTIFICATION_INTERNAL_PROCESS = 25,
            NOTIFICATION_INTERNAL_PHYSICS_PROCESS = 26,
            NOTIFICATION_POST_ENTER_TREE = 27,
            NOTIFICATION_DISABLED = 28,
            NOTIFICATION_ENABLED = 29,
            NOTIFICATION_RESET_PHYSICS_INTERPOLATION = 2001, // A GodotSpace Odyssey.
            // Keep these linked to Node.
            NOTIFICATION_WM_MOUSE_ENTER = 1002,
            NOTIFICATION_WM_MOUSE_EXIT = 1003,
            NOTIFICATION_WM_WINDOW_FOCUS_IN = 1004,
            NOTIFICATION_WM_WINDOW_FOCUS_OUT = 1005,
            NOTIFICATION_WM_CLOSE_REQUEST = 1006,
            NOTIFICATION_WM_GO_BACK_REQUEST = 1007,
            NOTIFICATION_WM_SIZE_CHANGED = 1008,
            NOTIFICATION_WM_DPI_CHANGE = 1009,
            NOTIFICATION_VP_MOUSE_ENTER = 1010,
            NOTIFICATION_VP_MOUSE_EXIT = 1011,

            NOTIFICATION_OS_MEMORY_WARNING = MainLoop::NOTIFICATION_OS_MEMORY_WARNING,
            NOTIFICATION_TRANSLATION_CHANGED = MainLoop::NOTIFICATION_TRANSLATION_CHANGED,
            NOTIFICATION_WM_ABOUT = MainLoop::NOTIFICATION_WM_ABOUT,
            NOTIFICATION_CRASH = MainLoop::NOTIFICATION_CRASH,
            NOTIFICATION_OS_IME_UPDATE = MainLoop::NOTIFICATION_OS_IME_UPDATE,
            NOTIFICATION_APPLICATION_RESUMED = MainLoop::NOTIFICATION_APPLICATION_RESUMED,
            NOTIFICATION_APPLICATION_PAUSED = MainLoop::NOTIFICATION_APPLICATION_PAUSED,
            NOTIFICATION_APPLICATION_FOCUS_IN = MainLoop::NOTIFICATION_APPLICATION_FOCUS_IN,
            NOTIFICATION_APPLICATION_FOCUS_OUT = MainLoop::NOTIFICATION_APPLICATION_FOCUS_OUT,
            NOTIFICATION_TEXT_SERVER_CHANGED = MainLoop::NOTIFICATION_TEXT_SERVER_CHANGED,

            // Editor specific node notifications
            NOTIFICATION_EDITOR_PRE_SAVE = 9001,
            NOTIFICATION_EDITOR_POST_SAVE = 9002,
        };


    private:
        _FORCE_INLINE_ void _update_children_cache() const {
            if (unlikely(data.children_cache_dirty)) {
                _update_children_cache_impl();
            }
        }

        _FORCE_INLINE_ void _update_components_cache() const {
            if (unlikely(data.children_cache_dirty)) {
                _update_components_cache_impl();
            }
        }

        void _move_child(GObject* p_child, int p_index, bool p_ignore_end = false);
        void _update_children_cache_impl() const;
        void _update_components_cache_impl() const;
        GObject* _get_child_by_name(const StringName& p_name) const;
        void _set_name_nocheck(const StringName& name);
        void _set_owner_nocheck(GObject*);
        void _add_child_nocheck(GObject* p_child, const StringName& p_name, InternalMode p_internal_mode = INTERNAL_MODE_DISABLED);
        void _clean_up_owner();
        void _release_unique_name_in_owner();
        void _acquire_unique_name_in_owner();
        void _validate_child_name(GObject* p_child, bool p_force_readable_name);
        void _set_tree(SceneTree* tree);
        // ʹ�ô�������������޸�
        void _propagate_enter_tree();
        void _propagate_exit_tree();
        void _propagate_ready();
        void _propagate_groups_dirty();
        void _propagate_after_exit_tree();

        void _add_component_nocheck(Component*);

        /*LocalVector<Component*> & _get_components() const {
            return data.components_cache;
        }*/
        // ����ӿ�
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
      


        // GObject needed in register
        static void init_gobj_hrcr();


        protected:
            void _block() { data.blocked++; }
            void _unblock() { data.blocked--; }

            void _notification(int p_notification);

            virtual void _physics_interpolated_changed() {}

            virtual void add_child_notify(GObject* p_child) {}
            virtual void remove_child_notify(GObject* p_child) {}
            virtual void move_child_notify(GObject* p_child) {}
            virtual void owner_changed_notify() {}
	};
}

#endif