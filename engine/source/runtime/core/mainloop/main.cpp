#include "main.h"
#include "core/os/os.h"
#include "core/engine/engine.h"
#include "function/display/window_system.h"
#include "function/render/rendering_system.h"
#include "core/config/project_settings.h"
#include "core/meta/reflection/reflection_register.h"
#include "core/register_core_types.h"
#include "core/thread/worker_thread_pool.h"
#include "editor/editor_paths.h"
#include "editor/project_manager.h"
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


 /// <summary>
 /// Main initialization.
 /// </summary>
 Error Main::Initialize(int argc, char* argv[]) {

	 String project_path = "";
	 // logger
	 OS::GetSingleton()->Initialize();
	
	 Reflection::TypeMetaRegister::metaRegister();
	 register_core_types();
	 engine = memnew(Engine); // 
	 window_system = memnew(WindowSystem);
	 render_system = memnew(RenderingSystem);
	 globals = memnew(ProjectSettings);
	 EditorPaths::create(); // editor需要在global之后，在ProjectManager之前
	 L_STRPRINT(EditorPaths::GetSingleton()->GetDataDir(), EditorPaths::GetSingleton()->GetConfigDir());
	 pmanager = memnew(ProjectManager);
	 // parse parameter
	 List<String> args;
	 for (int i = 0; i < argc; i++) {
		 args.push_back(String::utf8(argv[i]));
	 }
	 // initialize workerthreadpool
	 WorkerThreadPool::get_singleton()->init(-1, 0.75);
	 // Initialize user data dir.
	 OS::GetSingleton()->EnsureUserDataDir();


	 // change working dir
	 String path = argv[0];
	 // first time
	 if (!FileAccess::exists(path.path_join(ProjectSettings::PROJECT_FILE_NAME))) {
		 // mk
		 HashMap<String, HashMap<String, Variant>> config;
		 config["application"]["config/name"] = String("default_project");
		 config["application"]["config/description"] = String("this is a default project");
		 pmanager->CreateProject(path, config);

	 }

	 if (OS::GetSingleton()->SetCwd(path) == OK) {
		 // path already specified, don't override
	 }
	 else {
		 project_path = path;
	 }
	 globals->Initialize(project_path); 

	 // reflection
	 window_system->Initialize();
	 window_system->NewWindow(lain::WindowCreateInfo());

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
	Engine::GetSingleton()->m_frame_ticks = time;
	
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
