#include "main.h"
#include "core/os/os.h"
#include "core/engine/engine.h"
#include "function/display/displayer.h"
//  initialization part
static Engine* engine = nullptr;



// Main loop vairables
 uint64_t Main::last_ticks = 0;
 uint32_t Main::frames = 0;
 int Main::iterating = 0;
 /// <summary>
 /// Main initialization.
 /// </summary>
 void Main::Init() {
	 OS::GetSingleton()->Initialize();
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
	// sever's sending message
	return  true;
}