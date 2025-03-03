/**************************************************************************/
/*  input_event.h                                                         */
/**************************************************************************/

#ifndef INPUT_EVENT_H
#define INPUT_EVENT_H

#include "core/input/input_enums.h"
#include "core/io/resource.h"
#include "core/math/transform2d.h"
#include "core/input/keyboard.h"
#include "core/string/ustring.h"
#include "core/typedefs.h"
namespace lain{

/**
 * Input Event classes. These are used in the main loop.
 * The events are pretty obvious.
 */

class Shortcut;

/**
 * Input Modifier Status
 * for keyboard/mouse events.
 */

class InputEvent : public Resource {
	LCLASS(InputEvent, Resource);

	int device = 0;

protected:
	bool canceled = false;
	bool pressed = false;

	static void _bind_methods();

public:
	static const int DEVICE_ID_EMULATION;
	static const int DEVICE_ID_INTERNAL;

	void set_device(int p_device);
	int get_device() const;

	bool is_action(const StringName &p_action, bool p_exact_match = false) const;
	bool is_action_pressed(const StringName &p_action, bool p_allow_echo = false, bool p_exact_match = false) const;
	bool is_action_released(const StringName &p_action, bool p_exact_match = false) const;
	float get_action_strength(const StringName &p_action, bool p_exact_match = false) const;
	float get_action_raw_strength(const StringName &p_action, bool p_exact_match = false) const;

	bool is_canceled() const;
	bool is_pressed() const;
	bool is_released() const;
	virtual bool is_echo() const;

	virtual String as_text() const = 0;

	virtual Ref<InputEvent> xformed_by(const Transform2D &p_xform, const Vector2 &p_local_ofs = Vector2()) const;

	virtual bool action_match(const Ref<InputEvent> &p_event, bool p_exact_match, float p_deadzone, bool *r_pressed, float *r_strength, float *r_raw_strength) const;
	virtual bool is_match(const Ref<InputEvent> &p_event, bool p_exact_match = true) const;

	virtual bool is_action_type() const;

	virtual bool accumulate(const Ref<InputEvent> &p_event) { return false; }

	InputEvent() {}
};

class InputEventFromWindow : public InputEvent {
	LCLASS(InputEventFromWindow, InputEvent);

	int64_t window_id = 0;

protected:
	static void _bind_methods();

public:
	void set_window_id(int64_t p_id);
	int64_t get_window_id() const;

	InputEventFromWindow() {}
};

class InputEventWithModifiers : public InputEventFromWindow {
	LCLASS(InputEventWithModifiers, InputEventFromWindow);

	bool command_or_control_autoremap = false;

	bool shift_pressed = false;
	bool alt_pressed = false;
	bool meta_pressed = false; // "Command" on macOS, "Meta/Win" key on other platforms.
	bool ctrl_pressed = false;

protected:
	static void _bind_methods();
	void _validate_property(PropertyInfo &p_property) const;

public:
	void set_command_or_control_autoremap(bool p_enabled);
	bool is_command_or_control_autoremap() const;

	bool is_command_or_control_pressed() const;

	void set_shift_pressed(bool p_pressed);
	bool is_shift_pressed() const;

	void set_alt_pressed(bool p_pressed);
	bool is_alt_pressed() const;

	void set_ctrl_pressed(bool p_pressed);
	bool is_ctrl_pressed() const;

	void set_meta_pressed(bool p_pressed);
	bool is_meta_pressed() const;

	void set_modifiers_from_event(const InputEventWithModifiers *event);

	BitField<KeyModifierMask> get_modifiers_mask() const;

	virtual String as_text() const override;
	virtual String to_string() override;

	InputEventWithModifiers() {}
};

class InputEventKey : public InputEventWithModifiers {
	LCLASS(InputEventKey, InputEventWithModifiers);

	Key keycode = Key::NONE; // Key enum, without modifier masks.
	Key physical_keycode = Key::NONE;
	Key key_label = Key::NONE;
	uint32_t unicode = 0; ///unicode
	KeyLocation location = KeyLocation::UNSPECIFIED;

	bool echo = false; /// true if this is an echo key

protected:
	static void _bind_methods();

public:
	void set_pressed(bool p_pressed);

	void set_keycode(Key p_keycode);
	Key get_keycode() const;

	void set_physical_keycode(Key p_keycode);
	Key get_physical_keycode() const;

	void set_key_label(Key p_key_label);
	Key get_key_label() const;

	void set_unicode(char32_t p_unicode);
	char32_t get_unicode() const;

	void set_location(KeyLocation p_key_location);
	KeyLocation get_location() const;

	void set_echo(bool p_enable);
	virtual bool is_echo() const override;

	Key get_keycode_with_modifiers() const;
	Key get_physical_keycode_with_modifiers() const;
	Key get_key_label_with_modifiers() const;

	virtual bool action_match(const Ref<InputEvent> &p_event, bool p_exact_match, float p_deadzone, bool *r_pressed, float *r_strength, float *r_raw_strength) const override;
	virtual bool is_match(const Ref<InputEvent> &p_event, bool p_exact_match = true) const override;

	virtual bool is_action_type() const override { return true; }

	virtual String as_text_physical_keycode() const;
	virtual String as_text_keycode() const;
	virtual String as_text_key_label() const;
	virtual String as_text_location() const;
	virtual String as_text() const override;
	virtual String to_string() override;

	static Ref<InputEventKey> create_reference(Key p_keycode_with_modifier_masks, bool p_physical = false);

	InputEventKey() {}
};

class InputEventMouse : public InputEventWithModifiers {
	LCLASS(InputEventMouse, InputEventWithModifiers);

	BitField<MouseButtonMask> button_mask;

	Vector2 pos;
	Vector2 global_pos;

protected:
	static void _bind_methods();

public:
	void set_button_mask(BitField<MouseButtonMask> p_mask);
	BitField<MouseButtonMask> get_button_mask() const;

	void set_position(const Vector2 &p_pos);
	Vector2 get_position() const;

	void set_global_position(const Vector2 &p_global_pos);
	Vector2 get_global_position() const;

	InputEventMouse() {}
};

class InputEventMouseButton : public InputEventMouse {
	LCLASS(InputEventMouseButton, InputEventMouse);

	float factor = 1;
	MouseButton button_index = MouseButton::NONE;
	bool double_click = false; //last even less than double click time

protected:
	static void _bind_methods();

public:
	void set_factor(float p_factor);
	float get_factor() const;

	void set_button_index(MouseButton p_index);
	MouseButton get_button_index() const;

	void set_pressed(bool p_pressed);
	void set_canceled(bool p_canceled);

	void set_double_click(bool p_double_click);
	bool is_double_click() const;

	virtual Ref<InputEvent> xformed_by(const Transform2D &p_xform, const Vector2 &p_local_ofs = Vector2()) const override;

	virtual bool action_match(const Ref<InputEvent> &p_event, bool p_exact_match, float p_deadzone, bool *r_pressed, float *r_strength, float *r_raw_strength) const override;
	virtual bool is_match(const Ref<InputEvent> &p_event, bool p_exact_match = true) const override;

	virtual bool is_action_type() const override { return true; }
	virtual String as_text() const override;
	virtual String to_string() override;

	InputEventMouseButton() {}
};

class InputEventMouseMotion : public InputEventMouse {
	LCLASS(InputEventMouseMotion, InputEventMouse);

	Vector2 tilt;
	float pressure = 0;
	Vector2 relative;
	Vector2 screen_relative;
	Vector2 velocity;
	Vector2 screen_velocity;
	bool pen_inverted = false;

protected:
	static void _bind_methods();

public:
	void set_tilt(const Vector2 &p_tilt);
	Vector2 get_tilt() const;

	void set_pressure(float p_pressure);
	float get_pressure() const;

	void set_pen_inverted(bool p_inverted);
	bool get_pen_inverted() const;

	void set_relative(const Vector2 &p_relative);
	Vector2 get_relative() const;

	void set_relative_screen_position(const Vector2 &p_relative);
	Vector2 get_relative_screen_position() const;

	void set_velocity(const Vector2 &p_velocity);
	Vector2 get_velocity() const;

	void set_screen_velocity(const Vector2 &p_velocity);
	Vector2 get_screen_velocity() const;

	virtual Ref<InputEvent> xformed_by(const Transform2D &p_xform, const Vector2 &p_local_ofs = Vector2()) const override;
	virtual String as_text() const override;
	virtual String to_string() override;

	virtual bool accumulate(const Ref<InputEvent> &p_event) override;

	InputEventMouseMotion() {}
};

class InputEventJoypadMotion : public InputEvent {
	LCLASS(InputEventJoypadMotion, InputEvent);
	JoyAxis axis = (JoyAxis)0; ///< Joypad axis
	float axis_value = 0; ///< -1 to 1

protected:
	static void _bind_methods();

public:
	void set_axis(JoyAxis p_axis);
	JoyAxis get_axis() const;

	void set_axis_value(float p_value);
	float get_axis_value() const;

	virtual bool action_match(const Ref<InputEvent> &p_event, bool p_exact_match, float p_deadzone, bool *r_pressed, float *r_strength, float *r_raw_strength) const override;
	virtual bool is_match(const Ref<InputEvent> &p_event, bool p_exact_match = true) const override;

	virtual bool is_action_type() const override { return true; }
	virtual String as_text() const override;
	virtual String to_string() override;

	static Ref<InputEventJoypadMotion> create_reference(JoyAxis p_axis, float p_value);

	InputEventJoypadMotion() {}
};

class InputEventJoypadButton : public InputEvent {
	LCLASS(InputEventJoypadButton, InputEvent);

	JoyButton button_index = (JoyButton)0;
	float pressure = 0; //0 to 1
protected:
	static void _bind_methods();

public:
	void set_button_index(JoyButton p_index);
	JoyButton get_button_index() const;

	void set_pressed(bool p_pressed);

	void set_pressure(float p_pressure);
	float get_pressure() const;

	virtual bool action_match(const Ref<InputEvent> &p_event, bool p_exact_match, float p_deadzone, bool *r_pressed, float *r_strength, float *r_raw_strength) const override;
	virtual bool is_match(const Ref<InputEvent> &p_event, bool p_exact_match = true) const override;

	virtual bool is_action_type() const override { return true; }

	virtual String as_text() const override;
	virtual String to_string() override;

	static Ref<InputEventJoypadButton> create_reference(JoyButton p_btn_index);

	InputEventJoypadButton() {}
};

class InputEventScreenTouch : public InputEventFromWindow {
	LCLASS(InputEventScreenTouch, InputEventFromWindow);
	int index = 0;
	Vector2 pos;
	bool double_tap = false;

protected:
	static void _bind_methods();

public:
	void set_index(int p_index);
	int get_index() const;

	void set_position(const Vector2 &p_pos);
	Vector2 get_position() const;

	void set_pressed(bool p_pressed);
	void set_canceled(bool p_canceled);

	void set_double_tap(bool p_double_tap);
	bool is_double_tap() const;

	virtual Ref<InputEvent> xformed_by(const Transform2D &p_xform, const Vector2 &p_local_ofs = Vector2()) const override;
	virtual String as_text() const override;
	virtual String to_string() override;

	InputEventScreenTouch() {}
};

class InputEventScreenDrag : public InputEventFromWindow {
	LCLASS(InputEventScreenDrag, InputEventFromWindow);
	int index = 0;
	Vector2 pos;
	Vector2 relative;
	Vector2 screen_relative;
	Vector2 velocity;
	Vector2 screen_velocity;
	Vector2 tilt;
	float pressure = 0;
	bool pen_inverted = false;

protected:
	static void _bind_methods();

public:
	void set_index(int p_index);
	int get_index() const;

	void set_tilt(const Vector2 &p_tilt);
	Vector2 get_tilt() const;

	void set_pressure(float p_pressure);
	float get_pressure() const;

	void set_pen_inverted(bool p_inverted);
	bool get_pen_inverted() const;

	void set_position(const Vector2 &p_pos);
	Vector2 get_position() const;

	void set_relative(const Vector2 &p_relative);
	Vector2 get_relative() const;

	void set_relative_screen_position(const Vector2 &p_relative);
	Vector2 get_relative_screen_position() const;

	void set_velocity(const Vector2 &p_velocity);
	Vector2 get_velocity() const;

	void set_screen_velocity(const Vector2 &p_velocity);
	Vector2 get_screen_velocity() const;

	virtual Ref<InputEvent> xformed_by(const Transform2D &p_xform, const Vector2 &p_local_ofs = Vector2()) const override;
	virtual String as_text() const override;
	virtual String to_string() override;

	virtual bool accumulate(const Ref<InputEvent> &p_event) override;

	InputEventScreenDrag() {}
};

class InputEventAction : public InputEvent {
	LCLASS(InputEventAction, InputEvent);

	StringName action;
	float strength = 1.0f;
	int event_index = -1;

protected:
	static void _bind_methods();

public:
	void set_action(const StringName &p_action);
	StringName get_action() const;

	void set_pressed(bool p_pressed);

	void set_strength(float p_strength);
	float get_strength() const;

	void set_event_index(int p_index);
	int get_event_index() const;

	virtual bool is_action(const StringName &p_action) const;

	virtual bool action_match(const Ref<InputEvent> &p_event, bool p_exact_match, float p_deadzone, bool *r_pressed, float *r_strength, float *r_raw_strength) const override;
	virtual bool is_match(const Ref<InputEvent> &p_event, bool p_exact_match = true) const override;

	virtual bool is_action_type() const override { return true; }

	virtual String as_text() const override;
	virtual String to_string() override;

	InputEventAction() {}
};
}

#endif // INPUT_EVENT_H
