#include "main.h"
#include "core/os/os.h"
#include "core/engine/engine.h"
#include "function/display/window_system.h"
#include "function/render/rendering_system.h"
#include "core/config/project_settings.h"
#include "core/meta/reflection/reflection_register.h"
#include "core/register_core_types.h"
//  initialization part
namespace lain {

static lain::Engine* engine = nullptr;
static lain::WindowSystem* window_system = nullptr;
static lain::RenderingSystem* render_system = nullptr;
static lain::ProjectSettings* globals = nullptr;

// Main loop vairables
 uint64_t Main::last_ticks = 0;
 uint32_t Main::frames = 0;
 int Main::iterating = 0;


 /// <summary>
 /// Main initialization.
 /// </summary>
 void Main::Init() {
	 // logger
	 OS::GetSingleton()->Initialize();
	
	 Reflection::TypeMetaRegister::metaRegister();
	 register_core_types();

	 
	 engine = memnew(Engine); // 
	 window_system = memnew(lain::WindowSystem);
	 render_system = memnew(lain::RenderingSystem);
	 globals = memnew(lain::ProjectSettings);
	 window_system->Initialize();
	 window_system->NewWindow(lain::WindowCreateInfo());
	 globals->Initialize("D:\\LainEngine\\proj");
	 // reflection

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
