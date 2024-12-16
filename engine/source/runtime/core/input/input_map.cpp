
#include "input_map.h"
using namespace lain;

int InputMap::ALL_DEVICES = -1;
InputMap* InputMap::singleton = nullptr;
bool InputMap::event_is_action(const Ref<InputEvent> &p_event, const StringName &p_action, bool p_exact_match) const {
	return event_get_action_status(p_event, p_action, p_exact_match);
}

bool lain::InputMap::event_get_action_status(const Ref<InputEvent>& p_event, const StringName& p_action, bool p_exact_match, bool* r_pressed, float* r_strength,
                                             float* r_raw_strength, int* r_event_index) const {
  HashMap<StringName, Action>::Iterator E = input_map.find(p_action);
	// ERR_FAIL_COND_V_MSG(!E, false, suggest_actions(p_action));

	Ref<InputEventAction> input_event_action = p_event;
	if (input_event_action.is_valid()) {
		const bool pressed = input_event_action->is_pressed();
		if (r_pressed != nullptr) {
			*r_pressed = pressed;
		}
		const float strength = pressed ? input_event_action->get_strength() : 0.0f;
		if (r_strength != nullptr) {
			*r_strength = strength;
		}
		if (r_raw_strength != nullptr) {
			*r_raw_strength = strength;
		}
		if (r_event_index) {
			if (input_event_action->get_event_index() >= 0) {
				*r_event_index = input_event_action->get_event_index();
			} else {
				*r_event_index = E->value.inputs.size();
			}
		}
		return input_event_action->get_action() == p_action;
	}

	List<Ref<InputEvent>>::Element *event = _find_event(E->value, p_event, p_exact_match, r_pressed, r_strength, r_raw_strength, r_event_index);
	return event != nullptr;
}

void InputMap::add_action(const StringName &p_action, float p_deadzone) {
	ERR_FAIL_COND_MSG(input_map.has(p_action), "InputMap already has action \"" + String(p_action) + "\".");
	input_map[p_action] = Action();
	static int last_id = 1;
	input_map[p_action].id = last_id;
	input_map[p_action].deadzone = p_deadzone;
	last_id++;
}

float InputMap::action_get_deadzone(const StringName &p_action) {
	ERR_FAIL_COND_V_MSG(!input_map.has(p_action), 0.0f, suggest_actions(p_action));

	return input_map[p_action].deadzone;
}

void InputMap::erase_action(const StringName &p_action) {
	ERR_FAIL_COND_MSG(!input_map.has(p_action), suggest_actions(p_action));

	input_map.erase(p_action);
}
bool InputMap::has_action(const StringName &p_action) const {
	return input_map.has(p_action);
}
void lain::InputMap::action_add_event(const StringName& p_action, const Ref<InputEvent>& p_event) {
  	ERR_FAIL_COND_MSG(p_event.is_null(), "It's not a reference to a valid InputEvent object.");
	ERR_FAIL_COND_MSG(!input_map.has(p_action), suggest_actions(p_action));
	if (_find_event(input_map[p_action], p_event, true)) {
		return; // Already added.
	}

	input_map[p_action].inputs.push_back(p_event);
}
List<StringName> InputMap::get_actions() const {
  List<StringName> actions = List<StringName>();
  if (input_map.is_empty()) {
    return actions;
  }

  for (const KeyValue<StringName, Action>& E : input_map) {
    actions.push_back(E.key);
  }

  return actions;
}

lain::InputMap::InputMap() {
	singleton = this;

}

String lain::InputMap::suggest_actions(const StringName& p_action) const {
List<StringName> actions = get_actions();
	StringName closest_action;
	float closest_similarity = 0.0;

	// Find the most action with the most similar name.
	for (const StringName &action : actions) {
		const float similarity = String(action).similarity(p_action);

		if (similarity > closest_similarity) {
			closest_action = action;
			closest_similarity = similarity;
		}
	}

	String error_message = vformat("The InputMap action \"%s\" doesn't exist.", p_action);

	if (closest_similarity >= 0.4) {
		// Only include a suggestion in the error message if it's similar enough.
		error_message += vformat(" Did you mean \"%s\"?", closest_action);
	}
	return error_message;
}

const List<Ref<InputEvent>> *InputMap::action_get_events(const StringName &p_action) {
	HashMap<StringName, Action>::Iterator E = input_map.find(p_action);
	if (!E) {
		return nullptr;
	}

	return &E->value.inputs;
}

List<Ref<InputEvent>>::Element* InputMap::_find_event(Action& p_action, const Ref<InputEvent>& p_event, bool p_exact_match, bool* r_pressed, float* r_strength,
                                                      float* r_raw_strength, int* r_event_index) const {
  ERR_FAIL_COND_V(!p_event.is_valid(), nullptr);

  int i = 0;
  for (List<Ref<InputEvent>>::Element* E = p_action.inputs.front(); E; E = E->next()) {
    int device = E->get()->get_device();
    if (device == ALL_DEVICES || device == p_event->get_device()) {
      if (E->get()->action_match(p_event, p_exact_match, p_action.deadzone, r_pressed, r_strength, r_raw_strength)) {
        if (r_event_index) {
          *r_event_index = i;
        }
        return E;
      }
    }
    i++;
  }

  return nullptr;
}