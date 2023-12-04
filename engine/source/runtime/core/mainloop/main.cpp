#include "main.h"
#include "core/os/os.h"
#include "core/engine/engine.h"
#include "function/display/window_system.h"
#include "render/render_system.h"

//  initialization part
namespace lain {

static lain::Engine* engine = nullptr;
static lain::WindowSystem* window_system = nullptr;
static lain::RenderSystem* render_system = nullptr;


// Main loop vairables
 uint64_t Main::last_ticks = 0;
 uint32_t Main::frames = 0;
 int Main::iterating = 0;
 /// <summary>
 /// Main initialization.
 /// </summary>
 void Main::Init() {
	 OS::GetSingleton()->Initialize();
	 engine = memnew(Engine); // 
	 window_system = memnew(lain::WindowSystem);
	 render_system = memnew(lain::RenderSystem);
	 window_system->Initialize();
	 window_system->NewWindow(lain::WindowCreateInfo());

 }
/// <summary>
/// Main iteration.
/// </summary>
/// <returns></returns>
/// 
bool Main::Loop() {
	// gettime
	u64 time = OS::GetSingleton()->GetTimeUsec();
	// update engine
	u64 delta_time_usec = time - last_ticks;
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
