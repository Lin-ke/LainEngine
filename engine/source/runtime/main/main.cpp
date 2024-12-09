#include "main.h"
#include "core/config/project_settings.h"
#include "core/engine/engine.h"
#include "core/meta/reflection/reflection_register.h"
#include "core/os/main_loop.h"
#include "core/os/os.h"
#include "core/register_core_types.h"
#include "core/scene/scene_tree.h"
#include "core/thread/worker_thread_pool.h"
#include "editor/editor_paths.h"
#include "editor/project_manager.h"
#include "function/display/window_system.h"
#include "function/render/rendering_system/rendering_system_default.h"
#include "module/register_module_types.h"
#include "scene/register_scene_types.h"
#include "timer_sync.h"
#include "core/io/resource_loader.h"
#include "core/scene/packed_scene.h"
#include "core/scene/scene_tree.h"
#include "core/scene/object/gobject.h"
#include "scene/main/viewport.h"
//  initialization part
namespace lain {

static Engine* engine = nullptr;
static WindowSystem* window_system = nullptr;
static RenderingSystem* render_system = nullptr;
static ProjectSettings* globals = nullptr;
static ProjectManager* pmanager = nullptr;
// Drviers
String display_driver = "";
String rendering_driver = VulkanDriver;
String rendering_method = ForwardRenderingMethodName;


// Main loop vairables
uint64_t Main::last_ticks = 0;
uint32_t Main::frames = 0;
uint32_t Main::frame = 0;

int Main::iterating = 0;

// --- main window
static WindowSystem::WindowMode window_mode = WindowSystem::WINDOW_MODE_WINDOWED;
static WindowSystem::VSyncMode window_vsync_mode = WindowSystem::VSYNC_ENABLED;
static uint32_t window_flags = 0;
static Size2i window_size = Size2i(1152, 648);
static int init_screen = WindowSystem::SCREEN_PRIMARY;
static bool init_fullscreen = false;
static bool init_maximized = false;
static bool init_windowed = false;
static bool init_always_on_top = false;
static bool init_use_custom_pos = false;
static bool init_use_custom_screen = false;
static Vector2 init_custom_pos;


// everything the main loop needs to know about frame timings
static MainTimerSync main_timer_sync;

/// <summary>
/// Main initialization.
/// </summary>
Error Main::Initialize(int argc, char* argv[]) {
  Thread::make_main_thread();
  set_current_thread_safe_for_nodes(true); // main thread is always safe

  String project_path = "";
  // logger
  OS::GetSingleton()->Initialize();
  engine = memnew(Engine);  //

  register_core_types();

  Reflection::TypeMetaRegister::metaRegister();
  Reflection::TypeMetaRegister::EnumMetaRegister();

  globals = memnew(ProjectSettings);
  EditorPaths::create();  // editor需要在global之后，在ProjectManager之前
  //L_PRINT(EditorPaths::GetSingleton()->GetDataDir(), EditorPaths::GetSingleton()->GetConfigDir());
  pmanager = memnew(ProjectManager);
  // parse parameter
  List<String> args;
  for (int i = 0; i < argc; i++) {
    args.push_back(String::utf8(argv[i]));
  }
  // initialize workerthreadpool
  WorkerThreadPool::get_singleton()->init(16, 0.75);

  // change working dir
  String path = argv[0];
  // first time

  if (!FileAccess::exists(path.path_join(ProjectSettings::PROJECT_FILE_NAME))) {
    // mkdir
    if (!DirAccess::exists(path)) {
      Ref<DirAccess> da = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
      Error err = da->make_dir_recursive(path);
    }
    HashMap<String, HashMap<String, Variant>> config;
    config["application"]["config/name"] = String("default_project");
    config["application"]["config/description"] = String("this is a default project");
    //config["application"]["run/main_scene"] = String("this is a default project");
    pmanager->CreateProject(path, config);
  }

  if (OS::GetSingleton()->SetCwd(path) == OK) {
    // path already specified, don't override
  } else {
    project_path = path;
  }
  globals->Initialize(project_path);
  // Initialize user data dir.
  OS::GetSingleton()->EnsureUserDataDir();
  // initialize module
  initialize_modules(MODULE_INITIALIZATION_LEVEL_CORE); // 在其他等级时再初始化别的模块

  Ref<DirAccess> da = DirAccess::create_for_path(project_path);
 

  // note this is the desired rendering driver, it doesn't mean we will get it.
	// TODO - make sure this is updated in the case of fallbacks, so that the user interface
	// shows the correct driver string.
	OS::GetSingleton()->set_current_rendering_driver_name(rendering_driver);
	OS::GetSingleton()->set_current_rendering_method(rendering_method);

	// always convert to lower case for consistency in the code
	rendering_driver = rendering_driver.to_lower();

  /// --- windows related ----

  Vector2i* window_position = nullptr;
  int initial_position_type = GLOBAL_GET("display/window/size/initial_position_type").operator int();
  if (initial_position_type == 0) {  // Absolute.
    if (!init_use_custom_pos) {
      init_custom_pos = GLOBAL_GET("display/window/size/initial_position").operator Vector2i();
      init_use_custom_pos = true;
    }
  } else if (initial_position_type == 1) {  // Center of Primary Screen.
    if (!init_use_custom_screen) {
      init_screen = WindowSystem::SCREEN_PRIMARY;
      init_use_custom_screen = true;
    }
  } else if (initial_position_type == 2) {  // Center of Other Screen.
    if (!init_use_custom_screen) {
      init_screen = GLOBAL_GET("display/window/size/initial_screen").operator int();
      init_use_custom_screen = true;
    }
  } else if (initial_position_type == 3) {  // Center of Screen With Mouse Pointer.
    if (!init_use_custom_screen) {
      init_screen = WindowSystem::SCREEN_WITH_MOUSE_FOCUS;
      init_use_custom_screen = true;
    }
  } else if (initial_position_type == 4) {  // Center of Screen With Keyboard Focus.
    if (!init_use_custom_screen) {
      init_screen = WindowSystem::SCREEN_WITH_KEYBOARD_FOCUS;
      init_use_custom_screen = true;
    }
  }

  if (init_use_custom_pos) {
    Vector2i position = init_custom_pos;
    window_position = &position;
  }

  register_system_types();
  register_scene_types();


  Error err;
  // 注意这里的顺序（见WindowSystem里的注释）
  window_system = memnew(WindowSystem(rendering_driver, window_mode, window_vsync_mode, window_flags, window_position, window_size, init_screen, err));
  { // rendering thread mode
    int rtm = -1;
    rtm = GLOBAL_DEF("rendering/driver/render_thread_mode", OS::RenderThreadMode::RENDER_THREAD_SAFE);
    OS::GetSingleton()->set_render_thread_mode(OS::RenderThreadMode(rtm));
    render_system = memnew(RenderingSystemDefault(OS::GetSingleton()->get_render_thread_mode() == OS::RENDER_SEPARATE_THREAD));
		render_system->init();

  }
  Color clear = GLOBAL_DEF_BASIC("rendering/environment/defaults/default_clear_color", Color(0.3, 0.3, 0.3));
  RS::get_singleton()->set_default_clear_color(clear);
  // memnew(scenetree) 会 memnew一个window 会memnew一个viewport所以必须先进行RS的初始化
  MainLoop* main_loop = memnew(SceneTree);
  SceneTree *sml = static_cast<SceneTree*>(main_loop);
  if(sml){
  String main_scene = GLOBAL_GET("application/config/default_scene");
  if (main_scene.is_empty()) {
    // no default scene
    Vector<String> scene_list = da->get_files();
    for (String& scene : scene_list) {
      if (scene.ends_with(".tscn")) {
        main_scene = scene;
        break;
      }
    }
  }
  // 无scene
  // if (!main_scene.is_empty()) {
  //   // load scene
  //   Ref<PackedScene> ps = ResourceLoader::load(main_scene);
  //   if (ps.is_valid()) {
  //     GObject* newscene = ps->instantiate();
  //     SceneTree::get_singleton()->get_root()->add_child(newscene);
  //   }

  // }
  }

  OS::GetSingleton()->SetMainLoop(main_loop);

  return OK;
}
/// <summary>
/// Main iteration.
/// </summary>
/// <returns></returns>
///
bool Main::Loop() {
	iterating++;
  // get-set time and last ticks
  ui64 ticks = OS::GetSingleton()->GetTimeUsec();
  Engine::GetSingleton()->set_frame_ticks(ticks);
	const uint64_t ticks_elapsed = ticks - last_ticks;
  last_ticks = ticks;
  frame += ticks_elapsed;

  // update engine
	const double time_scale = Engine::GetSingleton()->get_time_scale();
  const int physics_ticks_per_second = Engine::GetSingleton()->get_physics_ticks_per_second();
	const double physics_step = 1.0 / physics_ticks_per_second;
  // 记录时间
  uint64_t physics_process_ticks = 0;
	uint64_t process_ticks = 0;
	uint64_t navigation_process_ticks = 0;

  MainFrameTime advance = main_timer_sync.advance(physics_step, physics_ticks_per_second);
	double process_step = advance.process_step;
	double scaled_step = process_step * time_scale;

  ui64 delta_time_usec = ticks - last_ticks;
	uint64_t process_begin = OS::GetSingleton()->GetTicksUsec();
 	// RS::get_singleton()->sync(); //sync if still drawing from previous frames.

  if (window_system->CanAnyWindowDraw() && render_system->is_render_loop_enabled()) {
				RenderingSystem::get_singleton()->draw(true, scaled_step); // flush visual commands
				Engine::GetSingleton()->increment_frames_drawn();
  }
	process_ticks = OS::GetSingleton()->GetTicksUsec() - process_begin;
  uint64_t frame_time =  OS::GetSingleton()->GetTicksUsec() - ticks;

	frames++;

  // sever's sending message
  // render server sending to windows
  window_system->SwapBuffers();
  return true;
}
}  // namespace lain
