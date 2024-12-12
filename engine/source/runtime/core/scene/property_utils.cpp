/**************************************************************************/
/*  property_utils.cpp                                                    */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "property_utils.h"

#include "core/engine/engine.h"
#include "core/templates/local_vector.h"

#ifdef TOOLS_ENABLED
#include "editor/editor_GObject.h"
#endif // TOOLS_ENABLED
using namespace lain;
bool PropertyUtils::is_property_value_different(const Object *p_object, const Variant &p_a, const Variant &p_b) {
	if (p_a.get_type() == Variant::FLOAT && p_b.get_type() == Variant::FLOAT) {
		// This must be done because, as some scenes save as text, there might be a tiny difference in floats due to numerical error.
		return !Math::is_equal_approx((float)p_a, (float)p_b);
	} else if (p_a.get_type() == Variant::GOBJECT_PATH && p_b.get_type() == Variant::OBJECT) {
		// With properties of type GObject, left side is GObjectPath, while right side is GObject.
		const GObject *base_GObject = Object::cast_to<GObject>(p_object);
		const GObject *target_GObject = Object::cast_to<GObject>(p_b);
		if (base_GObject && target_GObject) {
			return p_a != base_GObject->get_path_to(target_GObject);
		}
	}

	if (p_a.get_type() == Variant::ARRAY && p_b.get_type() == Variant::ARRAY) {
		const GObject *base_GObject = Object::cast_to<GObject>(p_object);
		Array array1 = p_a;
		Array array2 = p_b;

		if (base_GObject && !array1.is_empty() && array2.size() == array1.size() && array1[0].get_type() == Variant::GOBJECT_PATH && array2[0].get_type() == Variant::OBJECT) {
			// Like above, but GObjectPaths/GObjects are inside arrays.
			for (int i = 0; i < array1.size(); i++) {
				const GObject *target_GObject = Object::cast_to<GObject>(array2[i]);
				if (!target_GObject || array1[i] != base_GObject->get_path_to(target_GObject)) {
					return true;
				}
			}
			return false;
		}
	}

	// For our purposes, treating null object as NIL is the right thing to do
	const Variant &a = p_a.get_type() == Variant::OBJECT && (Object *)p_a == nullptr ? Variant() : p_a;
	const Variant &b = p_b.get_type() == Variant::OBJECT && (Object *)p_b == nullptr ? Variant() : p_b;
	return !(a == b);
}

bool lain::PropertyUtils::is_property_value_different(const Ref<Resource> p_object, const Variant& p_a, const Variant& p_b) {
	if (p_a.get_type() == Variant::FLOAT && p_b.get_type() == Variant::FLOAT) {
		// This must be done because, as some scenes save as text, there might be a tiny difference in floats due to numerical error.
		return !Math::is_equal_approx((float)p_a, (float)p_b);
	}
		// For our purposes, treating null object as NIL is the right thing to do
	const Variant &a = p_a.get_type() == Variant::OBJECT && (Object *)p_a == nullptr ? Variant() : p_a;
	const Variant &b = p_b.get_type() == Variant::OBJECT && (Object *)p_b == nullptr ? Variant() : p_b;

		return !(a == b);
}

Variant PropertyUtils::get_property_default_value(const Object *p_object, const StringName &p_property, bool *r_is_valid, const Vector<SceneState::PackState> *p_states_stack_cache, bool p_update_exports, const GObject *p_owner, bool *r_is_class_default) {
	PropertyInfo value;
	Variant ret;
	bool ok = ClassDB::get_property_info(p_object->get_class_name(), p_property, &value);
	if(!ok){
		if (r_is_valid) {
			*r_is_valid = false;
		}
		return Variant();
	}
	VariantInternal::initialize(&ret, value.type);
	if (r_is_valid) {
		*r_is_valid = true;
	}
	return ret;
}


Variant PropertyUtils::get_property_default_value(const Ref<Resource> p_object, const StringName &p_property, bool *r_is_valid, const Vector<SceneState::PackState> *p_states_stack_cache, bool p_update_exports, const GObject *p_owner, bool *r_is_class_default) {
	PropertyInfo value;
	Variant ret;
	bool ok = ClassDB::get_property_info(p_object->get_class_name(), p_property, &value);
	if(!ok){
		if (r_is_valid) {
			*r_is_valid = false;
		}
		return Variant();
	}
	VariantInternal::initialize(&ret, value.type);
	if (r_is_valid) {
		*r_is_valid = true;
	}
	return ret;
}

// Like SceneState::PackState, but using a raw pointer to avoid the cost of
// updating the reference count during the internal work of the functions below
namespace {
struct _FastPackState {
	SceneState *state = nullptr;
	int GObject = -1;
};
} // namespace

static bool _collect_inheritance_chain(const Ref<SceneState> &p_state, const GObjectPath &p_path, LocalVector<_FastPackState> &r_states_stack) {
	bool found = false;

	LocalVector<_FastPackState> inheritance_states;

	Ref<SceneState> state = p_state;
	while (state.is_valid()) {
		int GObject = state->find_gobject_by_path(p_path);
		if (GObject >= 0) {
			// This one has state for this GObject
			inheritance_states.push_back({ state.ptr(), GObject });
			found = true;
		}
		state = state->get_base_scene_state();
	}

	if (inheritance_states.size() > 0) {
		for (int i = inheritance_states.size() - 1; i >= 0; --i) {
			r_states_stack.push_back(inheritance_states[i]);
		}
	}

	return found;
}

Vector<SceneState::PackState> PropertyUtils::get_node_states_stack(const GObject *p_GObject, const GObject *p_owner, bool *r_instantiated_by_owner) {
	if (r_instantiated_by_owner) {
		*r_instantiated_by_owner = true;
	}

	LocalVector<_FastPackState> states_stack;
	{
		const GObject *owner = p_owner;
#ifdef TOOLS_ENABLED
		if (!p_owner && Engine::get_singleton()->is_editor_hint()) {
			owner = EditorGObject::get_singleton()->get_edited_scene();
		}
#endif

		const GObject *n = p_GObject;
		while (n) {
			if (n == owner) {
				const Ref<SceneState> &state = n->get_scene_inherited_state();
				if (_collect_inheritance_chain(state, n->get_path_to(p_GObject), states_stack)) {
					if (r_instantiated_by_owner) {
						*r_instantiated_by_owner = false;
					}
				}
				break;
			} else if (!n->get_scene_file_path().is_empty()) {
				const Ref<SceneState> &state = n->get_scene_instance_state();
				_collect_inheritance_chain(state, n->get_path_to(p_GObject), states_stack);
			}
			n = n->get_owner();
		}
	}

	// Convert to the proper type for returning, inverting the vector on the go
	// (it was more convenient to fill the vector in reverse order)
	Vector<SceneState::PackState> states_stack_ret;
	{
		states_stack_ret.resize(states_stack.size());
		_FastPackState *ps = states_stack.ptr();
		if (states_stack.size() > 0) {
			for (int i = states_stack.size() - 1; i >= 0; --i) {
				states_stack_ret.write[i].state.reference_ptr(ps->state);
				states_stack_ret.write[i].gobject = ps->GObject;
				++ps;
			}
		}
	}
	return states_stack_ret;
}
