#include "core/input/input_event.h"
namespace lain {

class InputMap : public Object {
  LCLASS(InputMap, Object);

 private:
  static InputMap* singleton;

 public:
  /**
	 * A special value used to signify that a given Action can be triggered by any device
	 */
  static int ALL_DEVICES;

  struct Action {
    int id;
    float deadzone;
    List<Ref<InputEvent>> inputs;
  };
  static InputMap* get_singleton() { return singleton; }
  mutable HashMap<StringName, Action> input_map; 

	bool event_is_action(const Ref<InputEvent> &p_event, const StringName &p_action, bool p_exact_match = false) const;
	bool event_get_action_status(const Ref<InputEvent> &p_event, const StringName &p_action, bool p_exact_match = false, bool *r_pressed = nullptr, float *r_strength = nullptr, float *r_raw_strength = nullptr, int *r_event_index = nullptr) const;
	void add_action(const StringName &p_action, float p_deadzone = 0.5);
	void erase_action(const StringName &p_action);
  float action_get_deadzone(const StringName& p_action) ;
  bool has_action(const StringName &p_action) const;
	void action_add_event(const StringName &p_action, const Ref<InputEvent> &p_event);
	String suggest_actions(const StringName &p_action) const;
  const HashMap<StringName, InputMap::Action> get_action_map() const {
  	return input_map; 
  }
  int event_get_index(const Ref<InputEvent> &p_event, const StringName &p_action, bool p_exact_match = false) const {
    int index = -1;
    event_get_action_status(p_event, p_action, p_exact_match, nullptr, nullptr, nullptr, &index);
    return index;
  }
  const List<Ref<InputEvent>> *action_get_events(const StringName &p_action);
  List<StringName> get_actions() const;
	InputMap();
	~InputMap() {singleton = nullptr;}
private:
	List<Ref<InputEvent>>::Element *_find_event(Action &p_action, const Ref<InputEvent> &p_event, bool p_exact_match = false, bool *r_pressed = nullptr, float *r_strength = nullptr, float *r_raw_strength = nullptr, int *r_event_index = nullptr) const;
};
}  // namespace lain
