#include "project_settings.h"
#include "base.h"
#include "config_parser.h"
#include "core/error/error_macros.h"
#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#include "core/os/mutex.h"
#include "core/os/os.h"

#define OSRESDIR OS::GetSingleton()->GetResourceDir()

using namespace lain;
ProjectSettings* ProjectSettings::p_singleton = nullptr;
const String ProjectSettings::PROJECT_DATA_DIR_NAME_SUFFIX = "prjdata";
const String ProjectSettings::PROJECT_FILE_NAME = "prj.txt";
const String ProjectSettings::PROJECT_BINARY_NAME = "prj.data";
const String ProjectSettings::ALL_PROJECTS_FILE_NAME = "prj.config";

Error ProjectSettings::Initialize(const String p_path) {
  Error err = _initialize(p_path);
  if (OK == err) {
    L_CORE_INFO("Successful init projectSettings.");
    project_data_dir_name = PROJECT_DATA_DIR_NAME_SUFFIX;
  }

  return OK;
}
const PackedStringArray ProjectSettings::get_required_features() {
  PackedStringArray features;
  features.append(VERSION_BRANCH);
  return features;
}

lain::ProjectSettings::ProjectSettings() {
  p_singleton = this;
  GLOBAL_DEF_BASIC("application/config/name", "");
	GLOBAL_DEF_BASIC(PropertyInfo(Variant::DICTIONARY, "application/config/name_localized", PROPERTY_HINT_LOCALIZABLE_STRING), Dictionary());
	GLOBAL_DEF_BASIC(PropertyInfo(Variant::STRING, "application/config/description", PROPERTY_HINT_MULTILINE_TEXT), "");
	GLOBAL_DEF_BASIC("application/config/version", "");
	GLOBAL_DEF_INTERNAL(PropertyInfo(Variant::STRING, "application/config/tags"), PackedStringArray());
	GLOBAL_DEF_BASIC(PropertyInfo(Variant::STRING, "application/run/main_scene", PROPERTY_HINT_FILE, "*.tscn,*.scn,*.res"), "");
	GLOBAL_DEF("application/run/disable_stdout", false);
	GLOBAL_DEF("application/run/disable_stderr", false);
	GLOBAL_DEF("application/run/print_header", true);
	GLOBAL_DEF("application/run/enable_alt_space_menu", false);
	GLOBAL_DEF_RST("application/config/use_hidden_project_data_directory", true);
	GLOBAL_DEF("application/config/use_custom_user_dir", false);
	GLOBAL_DEF("application/config/custom_user_dir_name", "");
	GLOBAL_DEF("application/config/project_settings_override", "");

	GLOBAL_DEF("application/run/main_loop_type", "SceneTree");
	GLOBAL_DEF("application/config/auto_accept_quit", true);
	GLOBAL_DEF("application/config/quit_on_go_back", true);

	// The default window size is tuned to:
	// - Have a 16:9 aspect ratio,
	// - Have both dimensions divisible by 8 to better play along with video recording,
	// - Be displayable correctly in windowed mode on a 1366×768 display (tested on Windows 10 with default settings).
	GLOBAL_DEF_BASIC(PropertyInfo(Variant::INT, "display/window/size/viewport_width", PROPERTY_HINT_RANGE, "1,7680,1,or_greater"), 1152); // 8K resolution
	GLOBAL_DEF_BASIC(PropertyInfo(Variant::INT, "display/window/size/viewport_height", PROPERTY_HINT_RANGE, "1,4320,1,or_greater"), 648); // 8K resolution

	GLOBAL_DEF_BASIC(PropertyInfo(Variant::INT, "display/window/size/mode", PROPERTY_HINT_ENUM, "Windowed,Minimized,Maximized,Fullscreen,Exclusive Fullscreen"), 0);

	// Keep the enum values in sync with the `DisplayServer::SCREEN_` enum.
	GLOBAL_DEF_BASIC(PropertyInfo(Variant::INT, "display/window/size/initial_position_type", PROPERTY_HINT_ENUM, "Absolute,Center of Primary Screen,Center of Other Screen,Center of Screen With Mouse Pointer,Center of Screen With Keyboard Focus"), 1);
	GLOBAL_DEF_BASIC(PropertyInfo(Variant::VECTOR2I, "display/window/size/initial_position"), Vector2i());
	GLOBAL_DEF_BASIC(PropertyInfo(Variant::INT, "display/window/size/initial_screen", PROPERTY_HINT_RANGE, "0,64,1,or_greater"), 0);

	GLOBAL_DEF_BASIC("display/window/size/resizable", true);
	GLOBAL_DEF_BASIC("display/window/size/borderless", false);
	GLOBAL_DEF("display/window/size/always_on_top", false);
	GLOBAL_DEF("display/window/size/transparent", false);
	GLOBAL_DEF("display/window/size/extend_to_title", false);
	GLOBAL_DEF("display/window/size/no_focus", false);

	GLOBAL_DEF(PropertyInfo(Variant::INT, "display/window/size/window_width_override", PROPERTY_HINT_RANGE, "0,7680,1,or_greater"), 0); // 8K resolution
	GLOBAL_DEF(PropertyInfo(Variant::INT, "display/window/size/window_height_override", PROPERTY_HINT_RANGE, "0,4320,1,or_greater"), 0); // 8K resolution

	GLOBAL_DEF("display/window/energy_saving/keep_screen_on", true);
#ifdef TOOLS_ENABLED
	GLOBAL_DEF("display/window/energy_saving/keep_screen_on.editor_hint", false);
#endif

	GLOBAL_DEF("animation/warnings/check_invalid_track_paths", true);
	GLOBAL_DEF("animation/warnings/check_angle_interpolation_type_conflicting", true);

	GLOBAL_DEF_BASIC(PropertyInfo(Variant::STRING, "audio/buses/default_bus_layout", PROPERTY_HINT_FILE, "*.tres"), "res://default_bus_layout.tres");
	GLOBAL_DEF(PropertyInfo(Variant::INT, "audio/general/default_playback_type", PROPERTY_HINT_ENUM, "Stream,Sample"), 0);
	GLOBAL_DEF(PropertyInfo(Variant::INT, "audio/general/default_playback_type.web", PROPERTY_HINT_ENUM, "Stream,Sample"), 1);
	GLOBAL_DEF_RST("audio/general/text_to_speech", false);
	GLOBAL_DEF_RST(PropertyInfo(Variant::FLOAT, "audio/general/2d_panning_strength", PROPERTY_HINT_RANGE, "0,2,0.01"), 0.5f);
	GLOBAL_DEF_RST(PropertyInfo(Variant::FLOAT, "audio/general/3d_panning_strength", PROPERTY_HINT_RANGE, "0,2,0.01"), 0.5f);

	GLOBAL_DEF(PropertyInfo(Variant::INT, "audio/general/ios/session_category", PROPERTY_HINT_ENUM, "Ambient,Multi Route,Play and Record,Playback,Record,Solo Ambient"), 0);
	GLOBAL_DEF("audio/general/ios/mix_with_others", false);

	// _add_builtin_input_map();

	// Keep the enum values in sync with the `DisplayServer::ScreenOrientation` enum.
	custom_prop_info["display/window/handheld/orientation"] = PropertyInfo(Variant::INT, "display/window/handheld/orientation", PROPERTY_HINT_ENUM, "Landscape,Portrait,Reverse Landscape,Reverse Portrait,Sensor Landscape,Sensor Portrait,Sensor");
	GLOBAL_DEF("display/window/subwindows/embed_subwindows", true);
	// Keep the enum values in sync with the `DisplayServer::VSyncMode` enum.
	custom_prop_info["display/window/vsync/vsync_mode"] = PropertyInfo(Variant::INT, "display/window/vsync/vsync_mode", PROPERTY_HINT_ENUM, "Disabled,Enabled,Adaptive,Mailbox");
	custom_prop_info["rendering/driver/threads/thread_model"] = PropertyInfo(Variant::INT, "rendering/driver/threads/thread_model", PROPERTY_HINT_ENUM, "Single-Unsafe,Single-Safe,Multi-Threaded");
	GLOBAL_DEF("physics/2d/run_on_separate_thread", false);
	GLOBAL_DEF("physics/3d/run_on_separate_thread", false);

	GLOBAL_DEF_BASIC(PropertyInfo(Variant::STRING, "display/window/stretch/mode", PROPERTY_HINT_ENUM, "disabled,canvas_items,viewport"), "disabled");
	GLOBAL_DEF_BASIC(PropertyInfo(Variant::STRING, "display/window/stretch/aspect", PROPERTY_HINT_ENUM, "ignore,keep,keep_width,keep_height,expand"), "keep");
	GLOBAL_DEF_BASIC(PropertyInfo(Variant::FLOAT, "display/window/stretch/scale", PROPERTY_HINT_RANGE, "0.5,8.0,0.01"), 1.0);
	GLOBAL_DEF_BASIC(PropertyInfo(Variant::STRING, "display/window/stretch/scale_mode", PROPERTY_HINT_ENUM, "fractional,integer"), "fractional");

	GLOBAL_DEF(PropertyInfo(Variant::INT, "debug/settings/profiler/max_functions", PROPERTY_HINT_RANGE, "128,65535,1"), 16384);
	GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "debug/settings/profiler/max_timestamp_query_elements", PROPERTY_HINT_RANGE, "256,65535,1"), 256);

	GLOBAL_DEF(PropertyInfo(Variant::BOOL, "compression/formats/zstd/long_distance_matching"), Compression::zstd_long_distance_matching);
	GLOBAL_DEF(PropertyInfo(Variant::INT, "compression/formats/zstd/compression_level", PROPERTY_HINT_RANGE, "1,22,1"), Compression::zstd_level);
	GLOBAL_DEF(PropertyInfo(Variant::INT, "compression/formats/zstd/window_log_size", PROPERTY_HINT_RANGE, "10,30,1"), Compression::zstd_window_log_size);
	GLOBAL_DEF(PropertyInfo(Variant::INT, "compression/formats/zlib/compression_level", PROPERTY_HINT_RANGE, "-1,9,1"), Compression::zlib_level);
	GLOBAL_DEF(PropertyInfo(Variant::INT, "compression/formats/gzip/compression_level", PROPERTY_HINT_RANGE, "-1,9,1"), Compression::gzip_level);

	GLOBAL_DEF("debug/settings/crash_handler/message",
			String("Please include this when reporting the bug to the project developer."));
	GLOBAL_DEF("debug/settings/crash_handler/message.editor",
			String("Please include this when reporting the bug on: https://github.com/godotengine/godot/issues"));
	GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "rendering/occlusion_culling/bvh_build_quality", PROPERTY_HINT_ENUM, "Low,Medium,High"), 2);
	GLOBAL_DEF_RST("rendering/occlusion_culling/jitter_projection", true);

	GLOBAL_DEF_RST("internationalization/rendering/force_right_to_left_layout_direction", false);
	GLOBAL_DEF_BASIC(PropertyInfo(Variant::INT, "internationalization/rendering/root_node_layout_direction", PROPERTY_HINT_ENUM, "Based on Application Locale,Left-to-Right,Right-to-Left,Based on System Locale"), 0);
	GLOBAL_DEF_BASIC("internationalization/rendering/root_node_auto_translate", true);

	GLOBAL_DEF(PropertyInfo(Variant::INT, "gui/timers/incremental_search_max_interval_msec", PROPERTY_HINT_RANGE, "0,10000,1,or_greater"), 2000);
	GLOBAL_DEF(PropertyInfo(Variant::FLOAT, "gui/timers/tooltip_delay_sec", PROPERTY_HINT_RANGE, "0,5,0.01,or_greater"), 0.5);
#ifdef TOOLS_ENABLED
	GLOBAL_DEF("gui/timers/tooltip_delay_sec.editor_hint", 0.5);
#endif

	GLOBAL_DEF_BASIC("gui/common/snap_controls_to_pixels", true);
	GLOBAL_DEF_BASIC("gui/fonts/dynamic_fonts/use_oversampling", true);

	GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "rendering/rendering_device/vsync/frame_queue_size", PROPERTY_HINT_RANGE, "2,3,1"), 2);
	GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "rendering/rendering_device/vsync/swapchain_image_count", PROPERTY_HINT_RANGE, "2,4,1"), 3);
	GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/rendering_device/staging_buffer/block_size_kb", PROPERTY_HINT_RANGE, "4,2048,1,or_greater"), 256);
	GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/rendering_device/staging_buffer/max_size_mb", PROPERTY_HINT_RANGE, "1,1024,1,or_greater"), 128);
	GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/rendering_device/staging_buffer/texture_upload_region_size_px", PROPERTY_HINT_RANGE, "1,256,1,or_greater"), 64);
	GLOBAL_DEF_RST(PropertyInfo(Variant::BOOL, "rendering/rendering_device/pipeline_cache/enable"), true);
	GLOBAL_DEF(PropertyInfo(Variant::FLOAT, "rendering/rendering_device/pipeline_cache/save_chunk_size_mb", PROPERTY_HINT_RANGE, "0.000001,64.0,0.001,or_greater"), 3.0);
	GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/rendering_device/vulkan/max_descriptors_per_pool", PROPERTY_HINT_RANGE, "1,256,1,or_greater"), 64);

	GLOBAL_DEF_RST("rendering/rendering_device/d3d12/max_resource_descriptors_per_frame", 16384);
	custom_prop_info["rendering/rendering_device/d3d12/max_resource_descriptors_per_frame"] = PropertyInfo(Variant::INT, "rendering/rendering_device/d3d12/max_resource_descriptors_per_frame", PROPERTY_HINT_RANGE, "512,262144");
	GLOBAL_DEF_RST("rendering/rendering_device/d3d12/max_sampler_descriptors_per_frame", 1024);
	custom_prop_info["rendering/rendering_device/d3d12/max_sampler_descriptors_per_frame"] = PropertyInfo(Variant::INT, "rendering/rendering_device/d3d12/max_sampler_descriptors_per_frame", PROPERTY_HINT_RANGE, "256,2048");
	GLOBAL_DEF_RST("rendering/rendering_device/d3d12/max_misc_descriptors_per_frame", 512);
	custom_prop_info["rendering/rendering_device/d3d12/max_misc_descriptors_per_frame"] = PropertyInfo(Variant::INT, "rendering/rendering_device/d3d12/max_misc_descriptors_per_frame", PROPERTY_HINT_RANGE, "32,4096");

	// The default value must match the minor part of the Agility SDK version
	// installed by the scripts provided in the repository
	// (check `misc/scripts/install_d3d12_sdk_windows.py`).
	// For example, if the script installs 1.613.3, the default value must be 613.
	GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "rendering/rendering_device/d3d12/agility_sdk_version"), 613);

	GLOBAL_DEF_BASIC(PropertyInfo(Variant::INT, "rendering/textures/canvas_textures/default_texture_filter", PROPERTY_HINT_ENUM, "Nearest,Linear,Linear Mipmap,Nearest Mipmap"), 1);
	GLOBAL_DEF_BASIC(PropertyInfo(Variant::INT, "rendering/textures/canvas_textures/default_texture_repeat", PROPERTY_HINT_ENUM, "Disable,Enable,Mirror"), 0);

	GLOBAL_DEF("collada/use_ambient", false);

	// Input settings
	GLOBAL_DEF_BASIC("input_devices/pointing/android/enable_long_press_as_right_click", false);
	GLOBAL_DEF_BASIC("input_devices/pointing/android/enable_pan_and_scale_gestures", false);
	GLOBAL_DEF_BASIC(PropertyInfo(Variant::INT, "input_devices/pointing/android/rotary_input_scroll_axis", PROPERTY_HINT_ENUM, "Horizontal,Vertical"), 1);

	// These properties will not show up in the dialog. If you want to exclude whole groups, use add_hidden_prefix().
	GLOBAL_DEF_INTERNAL("application/config/features", PackedStringArray());
	GLOBAL_DEF_INTERNAL("internationalization/locale/translation_remaps", PackedStringArray());
	GLOBAL_DEF_INTERNAL("internationalization/locale/translations", PackedStringArray());
	GLOBAL_DEF_INTERNAL("internationalization/locale/translations_pot_files", PackedStringArray());
	GLOBAL_DEF_INTERNAL("internationalization/locale/translation_add_builtin_strings_to_pot", false);

}

String ProjectSettings::GlobalizePath(const String& p_path) const {
  if (p_path.begins_with("res://")) {
    if (!resource_path.is_empty()) {
      return p_path.replace("res:/", resource_path);
    }
    return p_path.replace("res://", "");
  } else if (p_path.begins_with("user://")) {
    String data_dir = OS::GetSingleton()->GetUserDataDir();
    if (!data_dir.is_empty()) {
      return p_path.replace("user:/", data_dir);
    }
    return p_path.replace("user://", "");
  }

  return p_path;
}

String ProjectSettings::LocalizePath(const String& p_path) const {
  if (p_path.begins_with("res://"))
    return p_path;
  String path = p_path.simplify_path();

  if (resource_path.is_empty() || (path.is_absolute_path() && !path.begins_with(resource_path))) {
    return path;
  }

  // Check if we have a special path (like res://) or a protocol identifier.
  int p = path.find("://");
  bool found = false;
  if (p > 0) {
    found = true;
    for (int i = 0; i < p; i++) {
      if (!is_ascii_alphanumeric_char(path[i])) {
        found = false;
        break;
      }
    }
  }
  if (found) {
    return path;
  }

  Ref<DirAccess> dir = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);

  if (dir->change_dir(path) == OK) {
    String cwd = dir->get_current_dir();
    cwd = cwd.replace("\\", "/");

    // Ensure that we end with a '/'.
    // This is important to ensure that we do not wrongly localize the resource path
    // in an absolute path that just happens to contain this string but points to a
    // different folder (e.g. "/my/project" as resource_path would be contained in
    // "/my/project_data", even though the latter is not part of res://.
    // `path_join("")` is an easy way to ensure we have a trailing '/'.
    const String res_path = resource_path.path_join("");

    // DirAccess::get_current_dir() is not guaranteed to return a path that with a trailing '/',
    // so we must make sure we have it as well in order to compare with 'res_path'.
    cwd = cwd.path_join("");

    if (!cwd.begins_with(res_path)) {
      return path;
    }

    return cwd.replace_first(res_path, "res://");
  } else {
    int sep = path.rfind("/");
    if (sep == -1) {
      return "res://" + path;
    }

    String parent = path.substr(0, sep);

    String plocal = LocalizePath(parent);
    if (plocal.is_empty()) {
      return "";
    }
    // Only strip the starting '/' from 'path' if its parent ('plocal') ends with '/'
    if (plocal[plocal.length() - 1] == '/') {
      sep += 1;
    }
    return plocal + path.substr(sep, path.size() - sep);
  }
}

/// private
// 没初始化  OSRESDIR = ""

Error ProjectSettings::_initialize(const String p_path, bool p_ignore_override) {
  if (!OSRESDIR.is_empty()) {
    // OS (default) will call ProjectSettings->get_resource_path which will be empty if not overridden!
    // If the OS would rather use a specific location, then it will not be empty.
    resource_path = OSRESDIR.replace("\\", "/");
    if (!resource_path.is_empty() && resource_path[resource_path.length() - 1] == '/') {
      resource_path = resource_path.substr(0, resource_path.length() - 1);  // Chop end.
    }
  }
  // Try to use the filesystem for files, according to OS.
  // (Only Android -when reading from pck- and iOS use this.)
  if (!OSRESDIR.is_empty()) {  // if resdir is set, open it .
    Error err = _load_settings_text_or_binary("res://" + PROJECT_FILE_NAME, "");
    if (err == OK && !p_ignore_override) {
      // Optional, we don't mind if it fails.
      _load_settings_text("res://override.cfg");
    }
    return err;
  }
  // Nothing was found, try to find a project file in provided path (`p_path`)
  // or, if requested (`p_upwards`) in parent directories.
  Ref<DirAccess> d = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
  ERR_FAIL_COND_V_MSG(d.is_null(), ERR_CANT_CREATE, "Cannot create DirAccess for path '" + p_path + "'.");
  d->change_dir(p_path);

  String current_dir = d->get_current_dir();
  bool found = false;
  Error err;
  while (true) {
    // Set the resource path early so things can be resolved when loading.
    resource_path = current_dir;
    resource_path = resource_path.replace("\\", "/");  // Windows path to Unix path just in case.
    err = _load_settings_text_or_binary(current_dir.path_join(PROJECT_FILE_NAME), current_dir.path_join(PROJECT_BINARY_NAME));
    if (err == OK && !p_ignore_override) {
      // Optional, we don't mind if it fails.
      _load_settings_text(current_dir.path_join("override.cfg"));
      found = true;
      break;
    }

    //if (p_upwards) {
    //	// Try to load settings ascending through parent directories
    //	d->change_dir("..");
    //	if (d->get_current_dir() == current_dir) {
    //		break; // not doing anything useful
    //	}
    //	current_dir = d->get_current_dir();
    //}
    else {
      break;
    }
  }

  if (!found) {
    return err;
  }

  if (resource_path.length() && resource_path[resource_path.length() - 1] == '/') {
    resource_path = resource_path.substr(0, resource_path.length() - 1);  // Chop end.
  }

  return OK;
}
Error ProjectSettings::_load_settings_text_or_binary(const String& p_text_path, const String& p_bin_path) {

  Error err = _load_settings_text(p_text_path);
  if (err == OK) {
    return OK;
  } else if (err != ERR_FILE_NOT_FOUND) {
    ERR_PRINT("Couldn't load file '" + p_text_path + "', error code " + itos(err) + ".");
  }

  // Fallback to text-based binary file if binary was not found.
  err = _load_settings_binary(p_bin_path);
  if (err == OK) {
    return OK;
  } else if (err != ERR_FILE_NOT_FOUND) {
    // If the file exists but can't be loaded, we want to know it.
    ERR_PRINT("Couldn't load file '" + p_bin_path + "', error code " + itos(err) + ".");
  }

  return err;
}

Error ProjectSettings::_load_settings_text(const String& p_path) {
  Error err;
  Ref<FileAccess> f = FileAccess::open(p_path, FileAccess::READ, &err);
  auto cp = ConfigFile();

  if (f.is_null()) {
    // FIXME: Above 'err' error code is ERR_FILE_CANT_OPEN if the file is missing
    // This needs to be streamlined if we want decent error reporting
    return ERR_FILE_NOT_FOUND;
  }

  cp.ParseFile(f);
  for (const KeyValue<String, HashMap<String, Variant>>& E : cp.values) {
    String section = E.key;
    for (const KeyValue<String, Variant>& F : E.value) {
      if (section.is_empty() && F.key == "config_version") {
        // config_version
      } else {
        if (section.is_empty()) {
          Set(F.key, F.value);
        } else {
          Set(section + "/" + F.key, F.value);
        }
      }
    }
  }

  return OK;
}
Error ProjectSettings::_load_settings_binary(const String& text_path) {
  return OK;
}

/*
	* TODO
	*/
bool ProjectSettings::Set(const StringName& p_name, const Variant& p_value) {
  _THREAD_SAFE_METHOD_

  if (p_value.get_type() == Variant::NIL) {
    props.erase(p_name);
  } else {
    if (props.has(p_name)) {
      props[p_name].variant = p_value;
    } else {
      props[p_name] = VariantContainer(p_value, last_order++);
    }
  }

  return true;
}
bool ProjectSettings::Get(const StringName& p_name, Variant& r_ret) const {
  _THREAD_SAFE_METHOD_

  if (!props.has(p_name)) {
    WARN_PRINT("Property not found: " + String(p_name));
    return false;
  }
  r_ret = props[p_name].variant;
  return true;
}

bool lain::ProjectSettings::Has(const StringName& p_name) const {
	_THREAD_SAFE_METHOD_
	return props.has(p_name);
}

Variant ProjectSettings::GetSetting(const StringName& p_name) const {
  _THREAD_SAFE_METHOD_
  StringName name = p_name;
  if (!props.has(name)) {
    WARN_PRINT("Property not found: " + String(name));
    return Variant();
  }
  return props[name].variant;
}


void ProjectSettings::set_initial_value(const String &p_name, const Variant &p_value) {
	ERR_FAIL_COND_MSG(!props.has(p_name), "Request for nonexistent project setting: " + p_name + ".");

	// Duplicate so that if value is array or dictionary, changing the setting will not change the stored initial value.
	props[p_name].initial = p_value.duplicate();
}

void ProjectSettings::set_restart_if_changed(const String &p_name, bool p_restart) {
	ERR_FAIL_COND_MSG(!props.has(p_name), "Request for nonexistent project setting: " + p_name + ".");
	props[p_name].restart_if_changed = p_restart;
}

void ProjectSettings::set_as_basic(const String &p_name, bool p_basic) {
	ERR_FAIL_COND_MSG(!props.has(p_name), "Request for nonexistent project setting: " + p_name + ".");
	props[p_name].basic = p_basic;
}

void ProjectSettings::set_as_internal(const String &p_name, bool p_internal) {
	ERR_FAIL_COND_MSG(!props.has(p_name), "Request for nonexistent project setting: " + p_name + ".");
	props[p_name].internal = p_internal;
}

void ProjectSettings::set_ignore_value_in_docs(const String &p_name, bool p_ignore) {
	ERR_FAIL_COND_MSG(!props.has(p_name), "Request for nonexistent project setting: " + p_name + ".");
#ifdef DEBUG_METHODS_ENABLED
	props[p_name].ignore_value_in_docs = p_ignore;
#endif
}
bool ProjectSettings::get_ignore_value_in_docs(const String &p_name) const {
	ERR_FAIL_COND_V_MSG(!props.has(p_name), false, "Request for nonexistent project setting: " + p_name + ".");
#ifdef DEBUG_METHODS_ENABLED
	return props[p_name].ignore_value_in_docs;
#else
	return false;
#endif
}

void ProjectSettings::set_builtin_order(const String &p_name) {
	ERR_FAIL_COND_MSG(!props.has(p_name), "Request for nonexistent project setting: " + p_name + ".");
	if (props[p_name].order >= NO_BUILTIN_ORDER_BASE) {
		props[p_name].order = last_builtin_order++;
	}
}
Variant lain::_GLOBAL_DEF(const String& p_var, const Variant& p_default, bool p_restart_if_changed, bool p_ignore_value_in_docs, bool p_basic, bool p_internal) {
  Variant ret;
	if (!ProjectSettings::GetSingleton()->Has(p_var)) {
		ProjectSettings::GetSingleton()->Set(p_var, p_default);
	}
	ret = GLOBAL_GET(p_var);

	ProjectSettings::GetSingleton()->set_initial_value(p_var, p_default);
	ProjectSettings::GetSingleton()->set_builtin_order(p_var);
	ProjectSettings::GetSingleton()->set_as_basic(p_var, p_basic);
	ProjectSettings::GetSingleton()->set_restart_if_changed(p_var, p_restart_if_changed);
	ProjectSettings::GetSingleton()->set_ignore_value_in_docs(p_var, p_ignore_value_in_docs);
	ProjectSettings::GetSingleton()->set_as_internal(p_var, p_internal);
	return ret;
}

void ProjectSettings::set_custom_property_info(const PropertyInfo &p_info) {
	const String &prop_name = p_info.name;
	ERR_FAIL_COND(!props.has(prop_name));
	custom_prop_info[prop_name] = p_info;
}

Variant lain::_GLOBAL_DEF(const PropertyInfo & p_info, const Variant & p_default, bool p_restart_if_changed, bool p_ignore_value_in_docs, bool p_basic, bool p_internal)
{
	Variant ret = _GLOBAL_DEF(p_info.name, p_default, p_restart_if_changed, p_ignore_value_in_docs, p_basic, p_internal);
  L_PRINT("set: " , p_info.name, p_default.operator lain::String());
	ProjectSettings::GetSingleton()->set_custom_property_info(p_info);
	return ret;
}
# undef OSRESDIR