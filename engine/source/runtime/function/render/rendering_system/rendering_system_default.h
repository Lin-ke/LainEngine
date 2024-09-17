#include "rendering_system.h"
#include "rendering_system_globals.h"
#include "core/templates/command_queue_mt.h"
#include "core/engine/engine.h"
namespace lain{
class RenderingSystemDefault: public RenderingSystem{
	LCLASS(RenderingSystemDefault, RenderingSystem);

	Thread::ID server_thread = Thread::MAIN_ID;
	WorkerThreadPool::TaskID server_task_id = WorkerThreadPool::INVALID_TASK_ID;
	bool create_thread = false;
	mutable CommandQueueMT command_queue; // 和类状态无关的数据成员，const可用
	bool exit = false;
	#ifdef DEBUG_ENABLED
	#define MAIN_THREAD_SYNC_WARN WARN_PRINT("Call to " + String(__FUNCTION__) + " causing RenderingServer synchronizations on every frame. This significantly affects performance.");
	#endif
	#define WRITE_ACTION redraw_request();
	public:
		//if editor is redrawing when it shouldn't, enable this and put a breakpoint in _changes_changed()
	//#define DEBUG_CHANGES
	static int changes;
#ifdef DEBUG_CHANGES
	_FORCE_INLINE_ static void redraw_request() {
		changes++;
		_changes_changed();
	}

#else
	_FORCE_INLINE_ static void redraw_request() {
		changes++;
	}
#endif

#ifdef DEBUG_SYNC
#define SYNC_DEBUG print_line("sync on: " + String(__FUNCTION__));
#else
#define SYNC_DEBUG
#endif
	#include "rendering_system_helper.h"	


	/***************/
	/* CAMERA API */
	/***************/
	// Camera 的 server 是 RenderingMethod
	#define ServerName RenderingMethod
	#define server_name RSG::scene
	FUNCRIDSPLIT(camera)
	FUNC4(camera_set_perspective, RID, float, float, float)
	FUNC4(camera_set_orthogonal, RID, float, float, float)
	FUNC5(camera_set_frustum, RID, float, Vector2, float, float)
	FUNC2(camera_set_transform, RID, const Transform3D &)
	FUNC2(camera_set_cull_mask, RID, uint32_t)
	FUNC2(camera_set_environment, RID, RID)
	FUNC2(camera_set_camera_attributes, RID, RID)
	FUNC2(camera_set_compositor, RID, RID)
	FUNC2(camera_set_use_vertical_aspect, RID, bool)
	#undef ServerName
	#undef server_name
	/***************
	 * SHADER API  
	 ***************/
	// Shader 的 server 是 RendererMaterialStorage
	#define ServerName RendererMaterialStorage
	#define server_name RSG::material_storage 
	FUNCRIDSPLIT(shader)
	FUNC2(shader_set_code, RID, const String &)
	FUNC2(shader_set_path_hint, RID, const String &)
	FUNC1RC(String, shader_get_code, RID)

	FUNC2SC(get_shader_parameter_list, RID, List<PropertyInfo> *)

	FUNC4(shader_set_default_texture_parameter, RID, const StringName &, RID, int)
	FUNC3RC(RID, shader_get_default_texture_parameter, RID, const StringName &, int)
	FUNC2RC(Variant, shader_get_parameter_default, RID, const StringName &)

	FUNC1RC(ShaderNativeSourceCode, shader_get_native_source_code, RID)
	






	RenderingSystemDefault(bool p_create_thread = false);
	~RenderingSystemDefault();
	virtual void init() override;
	virtual void free(RID p_rid) override;
	virtual void finish() override;
	virtual void draw(bool p_swap_buffers, double frame_step) override;
	virtual void tick() override;
	virtual void sync() override;
	virtual void pre_draw(bool p_will_draw) override;


private:


	void _free(RID p_rid) ;
	void _finish();
	void _thread_exit();
	void _thread_loop(void*);	// 一个 thread 化的loop，用于多线程
	void _assign_mt_ids(WorkerThreadPool::TaskID p_tid);
	void _init();



};

} // namespace lain
