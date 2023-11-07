#ifdef L_PLATFORM_WINDOWS
#include <core/log/log.h>

int main(int argc, char** argv) {
	Lain::Log::Init();
	L_CORE_ERROR("Hello! Var={0}", 5);
	L_INFO("Hello! Var={0}", 5);
}
#endif
