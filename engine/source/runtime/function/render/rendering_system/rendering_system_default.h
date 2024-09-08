#include "rendering_system.h"
namespace lain{
class RenderingSystemDefult: public RenderingSystem{
	LCLASS(RenderingSystemDefult, RenderingSystem);

	#ifdef DEBUG_ENABLED
	#define MAIN_THREAD_SYNC_WARN WARN_PRINT("Call to " + String(__FUNCTION__) + " causing RenderingServer synchronizations on every frame. This significantly affects performance.");
	#endif
	#include "rendering_system_helper.h"	
	/***************
	 * SHADER API
	 ***************/
	#define ServerName RendererMaterialStorage
	#define server_name RSG::material_storage
	FUNCRIDSPLIT(shader)

};

} // namespace lain
