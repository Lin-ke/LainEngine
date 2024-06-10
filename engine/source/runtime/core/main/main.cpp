#include "main.h"
#include "core/os/os.h"
#include "core/engine/engine.h"
#include "function/display/window_system.h"
#include "function/render/rendering_system.h"
#include "core/config/project_settings.h"
#include "core/meta/reflection/reflection_register.h"
#include "core/thread/worker_thread_pool.h"
#include "editor/editor_paths.h"
#include "editor/project_manager.h"
#include "core/register_core_types.h"
#include "core/os/main_loop.h"
#include "core/scene/scene_tree.h"

//  initialization part
namespace lain {

static Engine* engine = nullptr;
static WindowSystem* window_system = nullptr;
static RenderingSystem* render_system = nullptr;
static ProjectSettings* globals = nullptr;
static ProjectManager* pmanager = nullptr;


// Main loop vairables
 uint64_t Main::last_ticks = 0;
 uint32_t Main::frames = 0;
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

 /// <summary>
 /// Main initialization.
 /// </summary>
 Error Main::Initialize(int argc, char* argv[]) {
	 Thread::make_main_thread();

	 String project_path = "";
	 // logger
	 OS::GetSingleton()->Initialize();
	 engine = memnew(Engine); // 
	 
	 register_core_types();
	
	 Reflection::TypeMetaRegister::metaRegister();
	 Reflection::TypeMetaRegister::EnumMetaRegister();

	 globals = memnew(ProjectSettings);
	 EditorPaths::create(); // editor需要在global之后，在ProjectManager之前
	 //L_PRINT(EditorPaths::GetSingleton()->GetDataDir(), EditorPaths::GetSingleton()->GetConfigDir());
	 pmanager = memnew(ProjectManager);
	 // parse parameter
	 List<String> args;
	 for (int i = 0; i < argc; i++) {
		 args.push_back(String::utf8(argv[i]));
	 }
	 // initialize workerthreadpool
	 WorkerThreadPool::get_singleton()->init(3, 0.75);


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
	 }
	 else {
		 project_path = path;
	 }
	 globals->Initialize(project_path); 
	 // Initialize user data dir.
	 OS::GetSingleton()->EnsureUserDataDir();

	 Ref<DirAccess> da = DirAccess::create_for_path(project_path);
	 String main_scene = GLOBAL_GET("application/config/name");
	 if (main_scene=="") {
		// no default scene
		 Vector<String> scene_list = da->get_files();
		 for (String& scene : scene_list) {
			 if (scene.ends_with(".tscn")) {
				 main_scene = scene;  break;
			 }
		 }
	 }



	 Vector2i* window_position = nullptr;
	 if (init_use_custom_pos) {
		Vector2i position = init_custom_pos;
		 window_position = &position;
	 }
	 MainLoop* main_loop = memnew(SceneTree);
	 Error err;
	 window_system = memnew(WindowSystem("vulkan", window_mode, window_vsync_mode, window_flags, window_position, window_size, init_screen, err));


	 if (main_scene != "") {

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
	// gettime
	ui64 time = OS::GetSingleton()->GetTimeUsec();
	// update engine
	ui64 delta_time_usec = time - last_ticks;
	last_ticks = time;
	// start time
	Engine::GetSingleton()->set_frame_ticks(time);
	
	// do all the ticks
	if (window_system->CanAnyWindowDraw() && render_system->IsLoopEnabled()) {
		// rendering
	}

	// sever's sending message
	// render server sending to windows
	window_system->SwapBuffers();
	return  true;
}
}
