#include "gobject.h"
#include "core/scene/scene_stringnames.h"
#include "core/os/thread.h"
#include "core/templates/hash_set.h"
#include "core/scene/packed_scene.h"
#include "core/scene/object/viewport_object.h"
#include "core/scene/component/component.h"
namespace lain {
	
	GObject* GObject::find_parent(const String& p_pattern) const {
		//ERR_THREAD_GUARD_V(nullptr);
		GObject* p = data.parent;
		while (p) {
			if (p->data.name.operator String().match(p_pattern)) {
				return p;
			}
			p = p->data.parent;
		}

		return nullptr;
	}
	void GObject::set_name(const String& p_name) {
		ERR_FAIL_COND_MSG(data.inside_tree && !Thread::is_main_thread(), "Changing the name to nodes inside the SceneTree is only allowed from the main thread. Use `set_name.call_deferred(new_name)`.");
		String name = p_name.validate_node_name();

		ERR_FAIL_COND(name.is_empty());

		if (data.unique_name_in_owner && data.owner) {
			_release_unique_name_in_owner();
		}
		String old_name = data.name;
		data.name = name;

		if (data.parent) {
			data.parent->_validate_child_name(this, true);
			bool success = data.parent->data.children.replace_key(old_name, data.name);
			ERR_FAIL_COND_MSG(!success, "Renaming child in hashtable failed, this is a bug.");
		}

		if (data.unique_name_in_owner && data.owner) {
			_acquire_unique_name_in_owner();
		}

		/*propagate_notification(NOTIFICATION_PATH_RENAMED);

		if (is_inside_tree()) {
			emit_signal(SNAME("renamed"));
			get_tree()->node_renamed(this);
			get_tree()->tree_changed();
		}*/
	}

	// 解析gobjectpath
	GObject* GObject::get_gobject_or_null(const GObjectPath& p_path) const {
		//ERR_THREAD_GUARD_V(nullptr);
		if (p_path.is_empty()) {
			return nullptr;
		}

		ERR_FAIL_COND_V_MSG(!data.inside_tree && p_path.is_absolute(), nullptr, "Can't use get_GObject() with absolute paths from outside the active scene tree.");

		GObject* current = nullptr;
		GObject* root = nullptr;

		if (!p_path.is_absolute()) {
			current = const_cast<GObject*>(this); //start from this
		}
		else {
			root = const_cast<GObject*>(this);
			while (root->data.parent) {
				root = root->data.parent; //start from root
			}
		}

		for (int i = 0; i < p_path.get_name_count(); i++) {
			StringName name = p_path.get_name(i);
			GObject* next = nullptr;

			if (name == SceneStringNames::get_singleton()->dot) { // .

				next = current;

			}
			else if (name == SceneStringNames::get_singleton()->doubledot) { // ..

				if (current == nullptr || !current->data.parent) {
					return nullptr;
				}

				next = current->data.parent;
			}
			else if (current == nullptr) {
				if (name == root->get_name()) {
					next = root;
				}

			}
			else if (name.is_gobject_unique_name()) {
				GObject** unique = current->data.owned_unique_gobjects.getptr(name);
				if (!unique && current->data.owner) {
					unique = current->data.owner->data.owned_unique_gobjects.getptr(name);
				}
				if (!unique) {
					return nullptr;
				}
				next = *unique;
			}
			else {
				next = nullptr;
				// gobj指针指向一个const指针
				const GObject* const* gobj = current->data.children.getptr(name);
				if (gobj) {
					next = const_cast<GObject*>(*gobj);
				}
				else {
					return nullptr;
				}
			}
			current = next;
		}

		return current;
	}

	// why use this? data.display_folded
	void GObject::set_editable_instance(GObject* p_node, bool p_editable) {
		//ERR_THREAD_GUARD
			ERR_FAIL_NULL(p_node);
		ERR_FAIL_COND(!is_ancestor_of(p_node));
		if (!p_editable) {
			p_node->data.editable_instance = false;
			// Avoid this flag being needlessly saved;
			// also give more visual feedback if editable children are re-enabled
			data.display_folded = false;
		}
		else {
			p_node->data.editable_instance = true;
		}
	}

	bool GObject::has_gobject(const GObjectPath& p_path) const {
		return get_gobject_or_null(p_path) != nullptr;
	}

	bool GObject::is_ancestor_of(const GObject* obj) const {
		ERR_FAIL_COND_V(!obj, false);
		GObject* temp = obj->data.parent;
		while (temp) {
			if (temp == this) { return true; }
			temp = temp->data.parent;
		}
		return false;
	}

	GObject* GObject::find_child(const String& p_pattern, bool p_recursive, bool p_owned) const {
		//ERR_THREAD_GUARD_V(nullptr);
		ERR_FAIL_COND_V(p_pattern.is_empty(), nullptr);
		_update_children_cache();
		GObject* const* cptr = data.children_cache.ptr();
		int ccount = data.children_cache.size();
		for (int i = 0; i < ccount; i++) {
			if (p_owned && !cptr[i]->data.owner) {
				continue;
			}
			if (cptr[i]->data.name.operator String().match(p_pattern)) {
				return cptr[i];
			}

			if (!p_recursive) {
				continue;
			}

			GObject* ret = cptr[i]->find_child(p_pattern, true, p_owned);
			if (ret) {
				return ret;
			}
		}
		return nullptr;
	}

	Component* GObject::get_component(const StringName& type_name) const {
		_update_components_cache();
		const Component * const * cmpt = data.components.getptr(type_name);
		if (cmpt) {
			return const_cast<Component*>(*cmpt);
		}
		else return nullptr;

	}

	Component* GObject::get_component(int p_index) const {
		ERR_FAIL_INDEX_V(p_index, (int)data.components_cache.size(), nullptr);
		_update_components_cache();
		return data.components_cache[p_index];
	}
	Vector<Component*> GObject::get_components() const {
		Vector<Component*> arr;
		int cc = get_component_count();
		arr.resize(cc);
		Component** p = arr.ptrw();
		for (int i = 0; i < cc; i++) {
			p[i] = get_component(i);
		}
		return arr;
	}
	// 在children修改后需要
	void GObject::_update_children_cache_impl() const {
		// Assign children
		data.children_cache.resize(data.children.size());
		int idx = 0;
		for (const KeyValue<StringName, GObject*>& K : data.children) {
			data.children_cache[idx] = K.value;
			idx++;
		}
		// Sort them
		data.children_cache.sort_custom<ComparatorByIndex>();
		// Update indices
		data.external_children_count_cache = 0;
		data.internal_children_back_count_cache = 0;
		data.internal_children_front_count_cache = 0;

		for (uint32_t i = 0; i < data.children_cache.size(); i++) {
			switch (data.children_cache[i]->data.internal_mode) {
			case INTERNAL_MODE_DISABLED: {
				data.children_cache[i]->data.index = data.external_children_count_cache++;
			} break;
			case INTERNAL_MODE_FRONT: {
				data.children_cache[i]->data.index = data.internal_children_front_count_cache++;
			} break;
			case INTERNAL_MODE_BACK: {
				data.children_cache[i]->data.index = data.internal_children_back_count_cache++;
			} break;
			}
		}
		data.children_cache_dirty = false;
	}
	int GObject::get_component_count() const {
		_update_components_cache();
		return data.components_cache.size();
	}

	void GObject::_update_components_cache_impl() const {
		data.components_cache.resize(data.components.size());
		int idx = 0;
		for (const KeyValue<StringName, Component*>& K : data.components) {
			data.components_cache[idx] = K.value;
			idx++;
		}
		data.components_cache.sort_custom<Component::ComparatorByIndexCompt>();
		data.components_cache_dirty = false;

	}
	void GObject::add_component(Component* p_component) {
		ERR_FAIL_COND_MSG(data.components.has(p_component->get_class_name()), vformat("Already have component type '%s'.", p_component->get_class_name()));
		ERR_FAIL_COND(p_component == nullptr);
		ERR_FAIL_COND_MSG(data.components[p_component->get_class_name()] == p_component, vformat("Already have component instance '%d'.", p_component));
		_add_component_nocheck(p_component);

	}
	void GObject::_add_component_nocheck(Component* p_component) {
		data.components.insert(p_component->get_class_name(), p_component);
		p_component->m_parent = this;
		// cache
		if (!data.components_cache_dirty) {
			// Special case, also add to the cached children array since its cheap.
			data.components_cache.push_back(p_component);
		}
		else {
			data.components_cache_dirty = true;
		}
		p_component->notification(NOTIFICATION_PARENTED);
		if(data.tree)
			p_component->notification(NOTIFICATION_ENTER_TREE);

	}


	void GObject::add_child(GObject* p_child, bool p_force_readable_name, InternalMode p_internal) {
		// check:
		ERR_FAIL_COND_MSG(data.inside_tree && !Thread::is_main_thread(), "Adding children to a node inside the SceneTree is only allowed from the main thread. Use call_deferred(\"add_child\",node).");
		ERR_FAIL_NULL(p_child);
		ERR_FAIL_COND_MSG(p_child == this, vformat("Can't add child '%s' to itself.", p_child->get_name())); // adding to itself!
		ERR_FAIL_COND_MSG(p_child->data.parent, vformat("Can't add child '%s' to '%s', already has a parent '%s'.", p_child->get_name(), get_name(), p_child->data.parent->get_name())); //Fail if node has a parent
#ifdef DEBUG_ENABLED
		ERR_FAIL_COND_MSG(p_child->is_ancestor_of(this), vformat("Can't add child '%s' to '%s' as it would result in a cyclic dependency since '%s' is already a parent of '%s'.", p_child->get_name(), get_name(), p_child->get_name(), get_name()));
#endif
		ERR_FAIL_COND_MSG(data.blocked > 0, "Parent node is busy setting up children, `add_child()` failed. Consider using `add_child.call_deferred(child)` instead.");

		_validate_child_name(p_child, p_force_readable_name);

#ifdef DEBUG_ENABLED
		if (p_child->data.owner && !p_child->data.owner->is_ancestor_of(p_child)) {
			// Owner of p_child should be ancestor of p_child.
			WARN_PRINT(vformat("Adding '%s' as child to '%s' will make owner '%s' inconsistent. Consider unsetting the owner beforehand.", p_child->get_name(), get_name(), p_child->data.owner->get_name()));
		}
#endif // DEBUG_ENABLED

		_add_child_nocheck(p_child, p_child->data.name, p_internal);
	}

	static SafeRefCount gobj_hrcr_count;

	void GObject::init_gobj_hrcr() {
		gobj_hrcr_count.init(1);
	}

	void GObject::_validate_child_name(GObject* p_child, bool p_force_human_readable) {
			bool unique = true;

			if (p_child->data.name == StringName()) {
				//new unique name must be assigned
				unique = false;
			}
			else {
				const GObject* const* existing = data.children.getptr(p_child->data.name);
				unique = !existing || *existing == p_child;
			}

			if (!unique) {
				ERR_FAIL_COND(!gobj_hrcr_count.ref());
				// Optimized version of the code below:
				 String name = "@" + String(p_child->get_name()) + "@" + itos(gobj_hrcr_count.get());
			}
	}


	void GObject::_add_child_nocheck(GObject* p_child, const StringName& p_name, InternalMode p_internal_mode) {
		p_child->data.name = p_name;
		data.children.insert(p_name, p_child);

		p_child->data.internal_mode = p_internal_mode;
		switch (p_internal_mode) {
		case INTERNAL_MODE_FRONT: {
			p_child->data.index = data.internal_children_front_count_cache++;
		} break;
		case INTERNAL_MODE_BACK: {
			p_child->data.index = data.internal_children_back_count_cache++;
		} break;
		case INTERNAL_MODE_DISABLED: {
			p_child->data.index = data.external_children_count_cache++;
		} break;
		}

		p_child->data.parent = this;
		// cache
		if (!data.children_cache_dirty && p_internal_mode == INTERNAL_MODE_DISABLED && data.internal_children_back_count_cache == 0) {
			// Special case, also add to the cached children array since its cheap.
			data.children_cache.push_back(p_child);
		}
		else {
			data.children_cache_dirty = true;
		}

		p_child->notification(NOTIFICATION_PARENTED);

		if (data.tree) {
			p_child->_set_tree(data.tree);
		}
		
		p_child->data.parent_owned = data.in_constructor; // ???
		add_child_notify(p_child);
		notification(NOTIFICATION_CHILD_ORDER_CHANGED);
		/*emit_signal(SNAME("child_order_changed"));*/
	}

	void GObject::_set_tree(SceneTree* p_tree) {
		SceneTree* tree_changed_a = nullptr;
		SceneTree* tree_changed_b = nullptr;

		//ERR_FAIL_COND(p_scene && data.parent && !data.parent->data.scene); //nobug if both are null

		if (data.tree) {
			_propagate_exit_tree(); //沿着树传播退出树的消息

			tree_changed_a = data.tree;
		}

		data.tree = p_tree;

		if (data.tree) {
			_propagate_enter_tree(); //沿着树传播进入树的消息
			if (!data.parent || data.parent->data.ready_notified) { // no parent (root) or parent ready
				_propagate_ready(); //reverse_notification(notification_ready);
			}

			tree_changed_b = data.tree;
		}

		//if (tree_changed_a) {
		//	tree_changed_a->tree_changed();
		//}
		//if (tree_changed_b) {
		//	tree_changed_b->tree_changed();
		//}

	}
	


	void GObject::_propagate_exit_tree(){
		data.blocked++;
		// children first
		for (HashMap<StringName, GObject*>::Iterator I = data.children.last(); I; --I) {
			I->value->_propagate_exit_tree();
		}

		data.blocked--;
		notification(NOTIFICATION_EXIT_TREE, true);
		if (data.tree) {
			data.tree->node_removed(this);
		}
		for (HashMap<StringName, Component*>::Iterator I = data.components.last(); I; --I) {
			I->value->notification(NOTIFICATION_EXIT_TREE, true); // @TODO 虚函数链的调用顺序
			I->value->set_inside_tree(false);
		}
		data.viewport = nullptr;
		data.ready_notified = false;
		data.tree = nullptr;
		data.depth = -1;
	}

	void GObject::_propagate_enter_tree() {
		// this needs to happen to all children before any enter_tree

		if (data.parent) {
			data.tree = data.parent->data.tree;
			data.depth = data.parent->data.depth + 1;
		}
		else {
			data.depth = 1;
		}
		// 如果这是viewport，直接用
		//data.viewport = dynamic_cast<ViewPort*>(this);
		if (!data.viewport && data.parent) {
			data.viewport = data.parent->data.viewport;
		}

		data.inside_tree = true;

		for (KeyValue<StringName, GroupData>& E : data.grouped) {
			E.value.group = data.tree->add_to_group(E.key, this);
		}
		
		notification(NOTIFICATION_ENTER_TREE);

		/*GDVIRTUAL_CALL(_enter_tree);

		emit_signal(SceneStringNames::get_singleton()->tree_entered);

		data.tree->gobj_added(this);

		if (data.parent) {
			Variant c = this;
			const Variant* cptr = &c;
			data.parent->emit_signalp(SNAME("child_entered_tree"), &cptr, 1);
		}*/
		for (KeyValue<StringName, Component*>& K : data.components) {
			if (!K.value->is_inside_tree()) { // could have been added in enter_tree
				K.value->set_inside_tree(true);
				K.value->notification(NOTIFICATION_ENTER_TREE);
			}
		}

		data.blocked++;
		//block while adding children

		for (KeyValue<StringName, GObject*>& K : data.children) {
			if (!K.value->is_inside_tree()) { // could have been added in enter_tree
				K.value->_propagate_enter_tree();
			}
		}

		data.blocked--;

//#ifdef DEBUG_ENABLED
//		SceneDebugger::add_to_cache(data.scene_file_path, this);
//#endif
		// enter groups
	}
	void GObject::add_to_group(const StringName& p_identifier, bool p_persistent) {
		//ERR_THREAD_GUARD
			ERR_FAIL_COND(!p_identifier.operator String().length());

		if (data.grouped.has(p_identifier)) {
			return;
		}

		GroupData gd;

		if (data.tree) {
			gd.group = data.tree->add_to_group(p_identifier, this);
		}
		else {
			gd.group = nullptr;
		}

		gd.persistent = p_persistent;

		data.grouped[p_identifier] = gd;
	}

	int GObject::get_child_count(bool p_include_internal) const {
		//ERR_THREAD_GUARD_V(0);
		_update_children_cache();

		if (p_include_internal) {
			return data.children_cache.size();
		}
		else {
			return data.children_cache.size() - data.internal_children_front_count_cache - data.internal_children_back_count_cache;
		}
	}

	void GObject::_set_name_nocheck(const StringName& p_name) {
		data.name = p_name;
	}

	void GObject::move_child(GObject* p_child, int p_index) {
		ERR_FAIL_COND_MSG(data.inside_tree && !Thread::is_main_thread(), "Moving child node positions inside the SceneTree is only allowed from the main thread. Use call_deferred(\"move_child\",child,index).");
		ERR_FAIL_NULL(p_child);
		ERR_FAIL_COND_MSG(p_child->data.parent != this, "Child is not a child of this node.");

		_update_children_cache();
		// We need to check whether node is internal and move it only in the relevant node range.
		if (p_child->data.internal_mode == INTERNAL_MODE_FRONT) {
			if (p_index < 0) {
				p_index += data.internal_children_front_count_cache;
			}
			ERR_FAIL_INDEX_MSG(p_index, data.internal_children_front_count_cache, vformat("Invalid new child index: %d. Child is internal.", p_index).ascii().get_data());
			_move_child(p_child, p_index);
		}
		else if (p_child->data.internal_mode == INTERNAL_MODE_BACK) {
			if (p_index < 0) {
				p_index += data.internal_children_back_count_cache;
			}
			ERR_FAIL_INDEX_MSG(p_index, data.internal_children_back_count_cache, vformat("Invalid new child index: %d. Child is internal.", p_index).ascii().get_data());
			_move_child(p_child, (int)data.children_cache.size() - data.internal_children_back_count_cache + p_index);
		}
		else {
			if (p_index < 0) {
				p_index += get_child_count(false);
			}
			ERR_FAIL_INDEX_MSG(p_index, (int)data.children_cache.size() + 1 - data.internal_children_front_count_cache - data.internal_children_back_count_cache, vformat("Invalid new child index: %d.", p_index).ascii().get_data());
			_move_child(p_child, p_index + data.internal_children_front_count_cache);
		}
	}

	void GObject::_move_child(GObject* p_child, int p_index, bool p_ignore_end) {
		ERR_FAIL_COND_MSG(data.blocked > 0, "Parent node is busy setting up children, `move_child()` failed. Consider using `move_child.call_deferred(child, index)` instead (or `popup.call_deferred()` if this is from a popup).");

		// Specifying one place beyond the end
		// means the same as moving to the last index
		if (!p_ignore_end) { // p_ignore_end is a little hack to make back internal children work properly.
			if (p_child->data.internal_mode == INTERNAL_MODE_FRONT) {
				if (p_index == data.internal_children_front_count_cache) {
					p_index--;
				}
			}
			else if (p_child->data.internal_mode == INTERNAL_MODE_BACK) {
				if (p_index == (int)data.children_cache.size()) {
					p_index--;
				}
			}
			else {
				if (p_index == (int)data.children_cache.size() - data.internal_children_back_count_cache) {
					p_index--;
				}
			}
		}

		int child_index = p_child->get_index();

		if (child_index == p_index) {
			return; //do nothing
		}

		int motion_from = MIN(p_index, child_index);
		int motion_to = MAX(p_index, child_index);

		data.children_cache.remove_at(child_index);
		data.children_cache.insert(p_index, p_child);

		if (data.tree) {
			data.tree->tree_changed();
		}

		data.blocked++;
		//new pos first
		for (int i = motion_from; i <= motion_to; i++) {
			if (data.children_cache[i]->data.internal_mode == INTERNAL_MODE_DISABLED) {
				data.children_cache[i]->data.index = i - data.internal_children_front_count_cache;
			}
			else if (data.children_cache[i]->data.internal_mode == INTERNAL_MODE_BACK) {
				data.children_cache[i]->data.index = i - data.internal_children_front_count_cache - data.external_children_count_cache;
			}
			else {
				data.children_cache[i]->data.index = i;
			}
		}
		// notification second
		move_child_notify(p_child);
		notification(NOTIFICATION_CHILD_ORDER_CHANGED);
		/*emit_signal(SNAME("child_order_changed")); */
		p_child->_propagate_groups_dirty();

		data.blocked--;
	}
	void GObject::_release_unique_name_in_owner() {
		ERR_FAIL_NULL(data.owner); // Safety check.
		StringName key = StringName(UNIQUE_NODE_PREFIX + data.name.operator String());
		GObject** which = data.owner->data.owned_unique_gobjects.getptr(key);
		if (which == nullptr || *which != this) {
			return; // Ignore.
		}
		data.owner->data.owned_unique_gobjects.erase(key);
	}
	void GObject::_acquire_unique_name_in_owner() {
		ERR_FAIL_NULL(data.owner); // Safety check.
		StringName key = StringName(UNIQUE_NODE_PREFIX + data.name.operator String());
		GObject** which = data.owner->data.owned_unique_gobjects.getptr(key);
		if (which != nullptr && *which != this) {
			String which_path = is_inside_tree() ? (*which)->get_path() : data.owner->get_path_to(*which);
			WARN_PRINT(vformat("Setting node name '%s' to be unique within scene for '%s', but it's already claimed by '%s'.\n'%s' is no longer set as having a unique name.",
				get_name(), is_inside_tree() ? get_path() : data.owner->get_path_to(this), which_path, which_path));
			data.unique_name_in_owner = false;
			return;
		}
		data.owner->data.owned_unique_gobjects[key] = this;
	}

	GObjectPath GObject::get_path() const {
		ERR_FAIL_COND_V_MSG(!data.inside_tree, GObjectPath(), "Cannot get path of node as it is not in a scene tree.");

		if (data.path_cache) {
			return *data.path_cache;
		}

		const GObject* n = this;

		Vector<StringName> path;

		while (n) {
			path.push_back(n->get_name());
			n = n->data.parent;
		}

		path.reverse();

		data.path_cache = memnew(GObjectPath(path, true));

		return *data.path_cache;
	}

	GObjectPath GObject::get_path_to(const GObject* p_node, bool p_use_unique_path) const {
		ERR_FAIL_NULL_V(p_node, GObjectPath());

		if (this == p_node) {
			return GObjectPath(".");
		}

		HashSet<const GObject*> visited;

		const GObject* n = this;

		while (n) {
			visited.insert(n);
			n = n->data.parent;
		}

		const GObject* common_parent = p_node; // 共同祖先

		while (common_parent) {
			if (visited.has(common_parent)) {
				break;
			}
			common_parent = common_parent->data.parent;
		}

		ERR_FAIL_NULL_V(common_parent, GObjectPath()); //nodes not in the same tree

		visited.clear();

		Vector<StringName> path;
		StringName up = String("..");

		if (p_use_unique_path) {
			n = p_node;

			bool is_detected = false;
			while (n != common_parent) {
				if (n->is_unique_name_in_owner() && n->get_owner() == get_owner()) {
					path.push_back(UNIQUE_NODE_PREFIX + String(n->get_name()));
					is_detected = true;
					break;
				}
				path.push_back(n->get_name());
				n = n->data.parent;
			}

			if (!is_detected) {
				n = this;

				String detected_name;
				int up_count = 0;
				while (n != common_parent) {
					if (n->is_unique_name_in_owner() && n->get_owner() == get_owner()) {
						detected_name = n->get_name();
						up_count = 0;
					}
					up_count++;
					n = n->data.parent;
				}

				for (int i = 0; i < up_count; i++) {
					path.push_back(up);
				}

				if (!detected_name.is_empty()) {
					path.push_back(UNIQUE_NODE_PREFIX + detected_name);
				}
			}
		}
		else { // 从这个到commonparent再到那个
			n = p_node;

			while (n != common_parent) {
				path.push_back(n->get_name());
				n = n->data.parent;
			}

			n = this;

			while (n != common_parent) {
				path.push_back(up);
				n = n->data.parent;
			}
		}

		path.reverse();

		return GObjectPath(path, false);
	}

	bool GObject::is_unique_name_in_owner() const {
		return data.unique_name_in_owner;
	}
	bool GObject::is_editable_instance(const GObject* p_obj) const {
		if (!p_obj) {
			return false; // Easier, null is never editable. :)
		}
		ERR_FAIL_COND_V(!is_ancestor_of(p_obj), false);
		return p_obj->data.editable_instance;
	}


	// 复用
	void GObject::add_sibling(GObject* p_sibling, bool p_force_readable_name) {
		ERR_FAIL_COND_MSG(data.inside_tree && !Thread::is_main_thread(), "Adding a sibling to a node inside the SceneTree is only allowed from the main thread. Use call_deferred(\"add_sibling\",node).");
		ERR_FAIL_NULL(p_sibling);
		ERR_FAIL_COND_MSG(p_sibling == this, vformat("Can't add sibling '%s' to itself.", p_sibling->get_name())); // adding to itself!
		ERR_FAIL_NULL(data.parent);
		ERR_FAIL_COND_MSG(data.parent->data.blocked > 0, "Parent node is busy setting up children, `add_sibling()` failed. Consider using `add_sibling.call_deferred(sibling)` instead.");

		data.parent->add_child(p_sibling, p_force_readable_name, data.internal_mode);
		data.parent->_update_children_cache();
		data.parent->_move_child(p_sibling, get_index() + 1);
	}


	void GObject::_propagate_groups_dirty() {
		for (const KeyValue<StringName, GroupData>& E : data.grouped) {
			if (E.value.group) {
				E.value.group->changed = true;
			}
		}

		for (KeyValue<StringName, GObject*>& K : data.children) {
			K.value->_propagate_groups_dirty();
		}
	}

	// 这需要遍历两边子树，这感觉不合理

	void GObject::remove_child(GObject* p_child) {
		ERR_FAIL_COND_MSG(data.inside_tree && !Thread::is_main_thread(), "Removing children from a node inside the SceneTree is only allowed from the main thread. Use call_deferred(\"remove_child\",node).");
		ERR_FAIL_NULL(p_child);
		ERR_FAIL_COND_MSG(data.blocked > 0, "Parent node is busy adding/removing children, `remove_child()` can't be called at this time. Consider using `remove_child.call_deferred(child)` instead.");
		ERR_FAIL_COND(p_child->data.parent != this);

		/**
		 *  Do not change the data.internal_children*cache counters here.
		 *  Because if nodes are re-added, the indices can remain
		 *  greater-than-everything indices and children added remain
		 *  properly ordered.
		 *
		 *  All children indices and counters will be updated next time the
		 *  cache is re-generated.
		 */

		data.blocked++;
		p_child->_set_tree(nullptr);
		//}

		//remove_child_notify(p_child);
		p_child->notification(NOTIFICATION_UNPARENTED);

		data.blocked--;

		data.children_cache_dirty = true;
		bool success = data.children.erase(p_child->data.name);
		ERR_FAIL_COND_MSG(!success, "Children name does not match parent name in hashtable, this is a bug.");

		p_child->data.parent = nullptr;
		p_child->data.index = -1;

		notification(NOTIFICATION_CHILD_ORDER_CHANGED);
		//emit_signal(SNAME("child_order_changed"));

		if (data.inside_tree) {
			p_child->_propagate_after_exit_tree(); // 带着孩子也要离开树
		}
	}

	void GObject::_propagate_after_exit_tree() {
		// Clear owner if it was not part of the pruned branch
		// why?
		if (data.owner) {
			bool found = false;
			GObject* parent = data.parent;

			while (parent) {
				if (parent == data.owner) {
					found = true;
					break;
				}

				parent = parent->data.parent;
			}

			if (!found) { // owner不在这棵树
				_clean_up_owner();
			}
		}

		data.blocked++; // 这个机制很有意思啊

		for (HashMap<StringName, GObject*>::Iterator I = data.children.last(); I; --I) {
			I->value->_propagate_after_exit_tree();
		}

		data.blocked--;

		//emit_signal(SceneStringNames::get_singleton()->tree_exited);
	}


	void GObject::_clean_up_owner() {
		ERR_FAIL_NULL(data.owner); // Safety check.

		if (data.unique_name_in_owner) {
			_release_unique_name_in_owner();
		}
		data.owner->data.owned.erase(data.OW);
		data.owner = nullptr;
		data.OW = nullptr;
	}

	void GObject::set_owner(GObject* p_owner) {
		//ERR_MAIN_THREAD_GUARD
			if (data.owner) {
				_clean_up_owner();
			}

		ERR_FAIL_COND(p_owner == this);

		if (!p_owner) {
			return;
		}

		GObject* check = get_parent();
		bool owner_valid = false;

		while (check) {
			if (check == p_owner) {
				owner_valid = true;
				break;
			}

			check = check->data.parent;
		}

		ERR_FAIL_COND_MSG(!owner_valid, "Invalid owner. Owner must be an ancestor in the tree.");

		_set_owner_nocheck(p_owner);

		if (data.unique_name_in_owner) {
			_acquire_unique_name_in_owner();
		}
	}
	void GObject::set_scene_inherited_state(Ref<SceneState> p_scene_state) {
		data.inherited_state = p_scene_state;

	}

	void GObject::_set_owner_nocheck(GObject* p_owner) {
		if (data.owner == p_owner) {
			return;
		}

		ERR_FAIL_COND(data.owner);
		data.owner = p_owner;
		data.owner->data.owned.push_back(this);
		data.OW = data.owner->data.owned.back();

		//owner_changed_notify();
	}
	GObject* GObject::_get_child_by_name(const StringName& p_name) const {
		const GObject* const* node = data.children.getptr(p_name);
		if (node) {
			return const_cast<GObject*>(*node);
		}
		else {
			return nullptr;
		}
	}

	void GObject::_propagate_ready() {
		data.ready_notified = true;
		data.blocked++;
		for (KeyValue<StringName, GObject*>& K : data.children) {
			K.value->_propagate_ready();
		}

		data.blocked--;

		notification(NOTIFICATION_POST_ENTER_TREE);

		if (data.ready_first) {
			data.ready_first = false;
			notification(NOTIFICATION_READY);
			//emit_signal(SceneStringNames::get_singleton()->ready);
		}
	}

	GObject* GObject::get_child(int p_index, bool p_include_internal) const {
		//ERR_THREAD_GUARD_V(nullptr);
		_update_children_cache();

		if (p_include_internal) {
			if (p_index < 0) {
				p_index += data.children_cache.size();
			}
			ERR_FAIL_INDEX_V(p_index, (int)data.children_cache.size(), nullptr);
			return data.children_cache[p_index];
		}
		else {
			if (p_index < 0) {
				p_index += (int)data.children_cache.size() - data.internal_children_front_count_cache - data.internal_children_back_count_cache;
			}
			ERR_FAIL_INDEX_V(p_index, (int)data.children_cache.size() - data.internal_children_front_count_cache - data.internal_children_back_count_cache, nullptr);
			p_index += data.internal_children_front_count_cache;
			return data.children_cache[p_index];
		}
	}



	bool GObject::is_greater_than(const GObject* p_node) const {
		ERR_FAIL_NULL_V(p_node, false);
		ERR_FAIL_COND_V(!data.inside_tree, false);
		ERR_FAIL_COND_V(!p_node->data.inside_tree, false);

		ERR_FAIL_COND_V(data.depth < 0, false);
		ERR_FAIL_COND_V(p_node->data.depth < 0, false);

		_update_children_cache();

		int* this_stack = (int*)alloca(sizeof(int) * data.depth);
		int* that_stack = (int*)alloca(sizeof(int) * p_node->data.depth);

		const GObject* n = this;

		int idx = data.depth - 1;
		while (n) {
			ERR_FAIL_INDEX_V(idx, data.depth, false);
			this_stack[idx--] = n->get_index();
			n = n->data.parent;
		}

		ERR_FAIL_COND_V(idx != -1, false);
		n = p_node;
		idx = p_node->data.depth - 1;
		while (n) {
			ERR_FAIL_INDEX_V(idx, p_node->data.depth, false);
			that_stack[idx--] = n->get_index();

			n = n->data.parent;
		}
		ERR_FAIL_COND_V(idx != -1, false);
		idx = 0;

		bool res;
		while (true) {
			// using -2 since out-of-tree or nonroot nodes have -1
			int this_idx = (idx >= data.depth) ? -2 : this_stack[idx];
			int that_idx = (idx >= p_node->data.depth) ? -2 : that_stack[idx];

			if (this_idx > that_idx) {
				res = true;
				break;
			}
			else if (this_idx < that_idx) {
				res = false;
				break;
			}
			else if (this_idx == -2) {
				res = false; // equal
				break;
			}
			idx++;
		}

		return res;
	}

	void GObject::_add_all_components_to_ptg() {
		_update_components_cache();
		for (auto& component : data.components_cache) {
			ERR_FAIL_COND(component->m_parent != this); // 不是你的你还要设计
			component->_add_to_process_thread_group();
		}
	}
	void GObject::_remove_all_components_from_ptg() {
		_update_components_cache();
		for (auto& component : data.components_cache) {
			ERR_FAIL_COND(component->m_parent != this); // 不是你的你还要设计
			component->_remove_from_process_thread_group();
		}
	}
	void GObject::_remove_tree_from_process_thread_group() {
		if (!is_inside_tree()) {
			return; // May not be initialized yet.
		}

		for (KeyValue<StringName, GObject*>& K : data.children) {
			if (K.value->tickdata.process_thread_group != PROCESS_THREAD_GROUP_INHERIT) {
				continue;
			}

			K.value->_remove_tree_from_process_thread_group();
		}

		if (is_any_processing()) {
			_remove_from_process_thread_group();
		}
	}

	void GObject::_add_tree_to_process_thread_group(GObject* p_owner) {
		if (is_any_processing()) {
			_add_to_process_thread_group();
		}

		tickdata.process_thread_group_owner = p_owner;
		if (p_owner != nullptr) {
			tickdata.process_group = p_owner->tickdata.process_group;
		}
		else {
			tickdata.process_group = &data.tree->default_process_group;
		}
		for (KeyValue<StringName, Component*>& K : data.components) {
			if (K.value->tickdata.process_thread_group != PROCESS_THREAD_GROUP_INHERIT) {
				continue;
			}
			K.value->set_ptg_owner(p_owner);
			K.value->_add_to_process_thread_group();
			
		}
		data.blocked++;

		for (KeyValue<StringName, GObject*>& K : data.children) {
			if (K.value->tickdata.process_thread_group != PROCESS_THREAD_GROUP_INHERIT) {
				continue;
			}

			K.value->_add_tree_to_process_thread_group(p_owner);
		}
		data.blocked--;
	}

	int GObject::orphan_node_count = 0;
	thread_local TickObject* GObject::current_process_thread_group = nullptr;

	GObject::GObject() {
			
			orphan_node_count++;

			// Default member initializer for bitfield is a C++20 extension, so:

			tickdata.process_mode = PROCESS_MODE_INHERIT;
			tickdata.physics_process = false;
			tickdata.process = false;
			tickdata.physics_process_internal = false;
			tickdata.process_internal = false;

			data.input = false;
			data.shortcut_input = false;
			data.unhandled_input = false;
			data.unhandled_key_input = false;

			/*data.physics_interpolation_mode = PHYSICS_INTERPOLATION_MODE_INHERIT;
			data.physics_interpolated = false;*/

			data.parent_owned = false;
			data.in_constructor = true;
			data.use_placeholder = false;

			//data.display_folded = false;
			data.editable_instance = false;

			data.inside_tree = false;
			data.ready_notified = false; // This is a small hack, so if a node is added during _ready() to the tree, it correctly gets the _ready() notification.
			data.ready_first = true;
	}

	GObject::~GObject() {
		data.grouped.clear();
		data.owned.clear();
		data.children.clear();
		data.children_cache.clear();

		ERR_FAIL_COND(data.parent);
		ERR_FAIL_COND(data.children_cache.size());

		orphan_node_count--;
	}

	void GObject::_notification(int p_notification) {
		L_PRINT("[Node notification]", "name", CSTR(data.name.operator lain::String()),p_notification);
		switch (p_notification) {
			// 这里去调用脚本了
			// 在unity中，脚本也是component的一部分
		case NOTIFICATION_PROCESS: {
			// call _process in script
		} break;
		case NOTIFICATION_ENTER_TREE: {
			// #like in unreal begin play, adding to tick function set.
			// # 肯定不如unreal的任务图，怎么做到负载均衡？（每个线程分配一个任务）
			// 但是UE的任务图肯定会需要大量的额外时间（但是这部分开销并不在_tick中）
			ERR_FAIL_NULL(get_tree());
			if (tickdata.process_mode == PROCESS_MODE_INHERIT) {
				if (data.parent) {
					tickdata.process_owner = data.parent->tickdata.process_owner;
				}
				else {
					ERR_PRINT("The root node can't be set to Inherit process mode, reverting to Pausable instead.");
					tickdata.process_mode = PROCESS_MODE_PAUSABLE;
					tickdata.process_owner = this;
				}
			}
			else {
				tickdata.process_owner = this;
			}

			{ // Update threaded process mode.
				if (tickdata.process_thread_group == PROCESS_THREAD_GROUP_INHERIT) {
					if (data.parent) {
						tickdata.process_thread_group_owner = data.parent->tickdata.process_thread_group_owner;
					}

					if (tickdata.process_thread_group_owner) {
						tickdata.process_group = tickdata.process_thread_group_owner->tickdata.process_group;
					}
					else {
						tickdata.process_group = &data.tree->default_process_group;
					}
				}
				// use new process_group
				else {
					tickdata.process_thread_group_owner = this;
					_add_process_group();
				}

				if (is_any_processing()) {
					_add_to_process_thread_group();
				}
			}

			//if (data.physics_interpolation_mode == PHYSICS_INTERPOLATION_MODE_INHERIT) {
			//	bool interpolate = true; // Root node default is for interpolation to be on.
			//	if (data.parent) {
			//		interpolate = data.parent->is_physics_interpolated();
			//	}
			//	_propagate_physics_interpolated(interpolate);
			//}
			// input group
			
			get_tree()->nodes_in_tree_count++;
			orphan_node_count--;

		} break;
		case NOTIFICATION_EXIT_TREE: {
			ERR_FAIL_NULL(get_tree());
			get_tree()->nodes_in_tree_count--;
			orphan_node_count++;
			// Remove from processing first.
			if (is_any_processing()) {
				_remove_from_process_thread_group();
			}
			// Remove the process group.
			if (tickdata.process_thread_group_owner == this) {
				_remove_process_group();
			}
			tickdata.process_thread_group_owner = nullptr;
			tickdata.process_owner = nullptr;

			if (data.path_cache) {
				memdelete(data.path_cache);
				data.path_cache = nullptr;
			}
			break;
		}
		default:
			return;
		}
	}

}