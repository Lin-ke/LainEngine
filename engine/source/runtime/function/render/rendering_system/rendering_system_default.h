#include "core/engine/engine.h"
#include "core/templates/command_queue_mt.h"
#include "rendering_system.h"
#include "rendering_system_globals.h"
namespace lain {
class RenderingSystemDefault : public RenderingSystem {
  LCLASS(RenderingSystemDefault, RenderingSystem);

  Thread::ID server_thread = Thread::MAIN_ID;
  WorkerThreadPool::TaskID server_task_id = WorkerThreadPool::INVALID_TASK_ID;
  bool create_thread = false;
  mutable CommandQueueMT command_queue;  // 和类状态无关的数据成员，const可用
  bool exit = false;                     // 是否已经退出

  List<Callable> frame_drawn_callbacks;

  double frame_setup_time = 0;  // 记录
  Vector<FrameProfileArea> frame_profile;
  uint64_t frame_profile_frame = 0;

  //for printing
  bool print_gpu_profile = false;
  HashMap<String, float> print_gpu_profile_task_time;
  uint64_t print_frame_profile_ticks_from = 0;
  uint32_t print_frame_profile_frame_count = 0;

#ifdef DEBUG_ENABLED
#define MAIN_THREAD_SYNC_WARN \
  WARN_PRINT("Call to " + String(__FUNCTION__) + " causing RenderingServer synchronizations on every frame. This significantly affects performance.");
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
  _FORCE_INLINE_ static void redraw_request() { changes++; }
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
  FUNC2(camera_set_transform, RID, const Transform3D&)
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

  FUNC2(shader_set_code, RID, const String&)
  FUNC2(shader_set_path_hint, RID, const String&)
  FUNC1RC(String, shader_get_code, RID)

  FUNC2SC(get_shader_parameter_list, RID, List<PropertyInfo>*)

  FUNC4(shader_set_default_texture_parameter, RID, const StringName&, RID, int)
  FUNC3RC(RID, shader_get_default_texture_parameter, RID, const StringName&, int)
  FUNC2RC(Variant, shader_get_parameter_default, RID, const StringName&)

  FUNC1RC(ShaderNativeSourceCode, shader_get_native_source_code, RID)

  /* COMMON MATERIAL API */

  FUNCRIDSPLIT(material)

  FUNC2(material_set_shader, RID, RID)

  FUNC3(material_set_param, RID, const StringName&, const Variant&)
  FUNC2RC(Variant, material_get_param, RID, const StringName&)

  FUNC2(material_set_render_priority, RID, int)
  FUNC2(material_set_next_pass, RID, RID)

  /* MESH API */

//from now on, calls forwarded to this singleton
#undef ServerName
#undef server_name
  /************* */
  /* TEXTURE API */
  /************ */

#define ServerName RendererTextureStorage
#define server_name RSG::texture_storage

#define FUNCRIDTEX0(m_type)                                                                               \
  virtual RID m_type##_create() override {                                                                \
    RID ret = RSG::texture_storage->texture_allocate();                                                   \
    if (Thread::get_caller_id() == server_thread || RSG::texture_storage->can_create_resources_async()) { \
      RSG::texture_storage->m_type##_initialize(ret);                                                     \
    } else {                                                                                              \
      command_queue.push(RSG::texture_storage, &RendererTextureStorage::m_type##_initialize, ret);        \
    }                                                                                                     \
    return ret;                                                                                           \
  }

#define FUNCRIDTEX1(m_type, m_type1)                                                                      \
  virtual RID m_type##_create(m_type1 p1) override {                                                      \
    RID ret = RSG::texture_storage->texture_allocate();                                                   \
    if (Thread::get_caller_id() == server_thread || RSG::texture_storage->can_create_resources_async()) { \
      RSG::texture_storage->m_type##_initialize(ret, p1);                                                 \
    } else {                                                                                              \
      command_queue.push(RSG::texture_storage, &RendererTextureStorage::m_type##_initialize, ret, p1);    \
    }                                                                                                     \
    return ret;                                                                                           \
  }

#define FUNCRIDTEX2(m_type, m_type1, m_type2)                                                              \
  virtual RID m_type##_create(m_type1 p1, m_type2 p2) override {                                           \
    RID ret = RSG::texture_storage->texture_allocate();                                                    \
    if (Thread::get_caller_id() == server_thread || RSG::texture_storage->can_create_resources_async()) {  \
      RSG::texture_storage->m_type##_initialize(ret, p1, p2);                                              \
    } else {                                                                                               \
      command_queue.push(RSG::texture_storage, &RendererTextureStorage::m_type##_initialize, ret, p1, p2); \
    }                                                                                                      \
    return ret;                                                                                            \
  }

#define FUNCRIDTEX6(m_type, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6)                                          \
  virtual RID m_type##_create(m_type1 p1, m_type2 p2, m_type3 p3, m_type4 p4, m_type5 p5, m_type6 p6) override {           \
    RID ret = RSG::texture_storage->texture_allocate();                                                                    \
    if (Thread::get_caller_id() == server_thread || RSG::texture_storage->can_create_resources_async()) {                  \
      RSG::texture_storage->m_type##_initialize(ret, p1, p2, p3, p4, p5, p6);                                              \
    } else {                                                                                                               \
      command_queue.push(RSG::texture_storage, &RendererTextureStorage::m_type##_initialize, ret, p1, p2, p3, p4, p5, p6); \
    }                                                                                                                      \
    return ret;                                                                                                            \
  }

  //these go pass-through, as they can be called from any thread
  FUNCRIDTEX1(texture_2d, const Ref<Image>&)
  FUNCRIDTEX2(texture_2d_layered, const Vector<Ref<Image>>&, TextureLayeredType)
  FUNCRIDTEX6(texture_3d, Image::Format, int, int, int, bool, const Vector<Ref<Image>>&)
  FUNCRIDTEX2(texture_rd, const RID&, const RS::TextureLayeredType)

  //these go through command queue if they are in another thread
  FUNC3(texture_2d_update, RID, const Ref<Image>&, int)
  FUNC2(texture_3d_update, RID, const Vector<Ref<Image>>&)

  FUNC1RC(Ref<Image>, texture_2d_get, RID)
  FUNC2RC(Ref<Image>, texture_2d_layer_get, RID, int)
  FUNC1RC(Vector<Ref<Image>>, texture_3d_get, RID)

  FUNC2RC(RID, texture_get_rd_texture, RID, bool)
  FUNC2RC(uint64_t, texture_get_native_handle, RID, bool)
  // 这边的函数写法都可以codegen

#undef server_name
#undef ServerName
//from now on, calls forwarded to this singleton
#define ServerName RendererViewport
#define server_name RSG::viewport

  /* VIEWPORT TARGET API */

  FUNCRIDSPLIT(viewport)
  FUNC3(viewport_set_size, RID, int, int)
  FUNC2(viewport_set_parent_viewport, RID, RID)
  FUNC2(viewport_set_active, RID, bool)

  FUNC3(viewport_attach_to_screen, RID, const Rect2&, int)
  FUNC2(viewport_set_render_direct_to_screen, RID, bool)

  FUNC2(viewport_set_update_mode, RID, ViewportUpdateMode)
  FUNC1RC(ViewportUpdateMode, viewport_get_update_mode, RID)

  FUNC2(viewport_set_clear_mode, RID, ViewportClearMode)
  FUNC1RC(RID, viewport_get_render_target, RID)
  FUNC1RC(RID, viewport_get_texture, RID)
  FUNC2(viewport_set_msaa_2d, RID, ViewportMSAA)
  FUNC2(viewport_set_msaa_3d, RID, ViewportMSAA)

  FUNC2(viewport_attach_camera, RID, RID)
  FUNC2(viewport_set_scenario, RID, RID)
  FUNC2(viewport_attach_canvas, RID, RID)

  /* Light API */
#undef ServerName
#undef server_name

#define ServerName RendererLightStorage
#define server_name RSG::light_storage

  FUNCRIDSPLIT(directional_light)
  FUNCRIDSPLIT(omni_light)
  FUNCRIDSPLIT(spot_light)
  FUNC2(light_set_color, RID, const Color&)
  FUNC3(light_set_param, RID, LightParam, float)
  FUNC2(light_set_shadow, RID, bool)
  FUNC2(light_set_projector, RID, RID)
  FUNC2(light_set_negative, RID, bool)
  FUNC2(light_set_cull_mask, RID, uint32_t)
  FUNC5(light_set_distance_fade, RID, bool, float, float, float)
  FUNC2(light_set_reverse_cull_face_mode, RID, bool)

  FUNC2(light_omni_set_shadow_mode, RID, LightOmniShadowMode)

  FUNC2(light_directional_set_shadow_mode, RID, LightDirectionalShadowMode)
  FUNC2(light_directional_set_blend_splits, RID, bool)
  FUNC2(light_directional_set_sky_mode, RID, LightDirectionalSkyMode)

  /*Shadow Atlas*/
  FUNC0R(RID, shadow_atlas_create)
  FUNC3(shadow_atlas_set_size, RID, int, bool)
  FUNC3(shadow_atlas_set_quadrant_subdivision, RID, int, int)

  FUNC2(directional_shadow_atlas_set_size, int, bool)
#undef server_name
#undef ServerName

  /* CAMERA ATTRIBUTES */
//from now on, calls forwarded to this singleton
#define ServerName RendererCameraAttributes
#define server_name RSG::camera_attributes

  FUNCRIDSPLIT(camera_attributes)

  FUNC2(camera_attributes_set_dof_blur_quality, DOFBlurQuality, bool)
  FUNC1(camera_attributes_set_dof_blur_bokeh_shape, DOFBokehShape)

  FUNC8(camera_attributes_set_dof_blur, RID, bool, float, float, bool, float, float, float)
  FUNC3(camera_attributes_set_exposure, RID, float, float)
  FUNC6(camera_attributes_set_auto_exposure, RID, bool, float, float, float, float)

#undef server_name
#undef ServerName

#define ServerName RendererMeshStorage
#define server_name RSG::mesh_storage
	FUNCRIDSPLIT(mesh)
	virtual RID mesh_create_from_surfaces(const Vector<RS::SurfaceData> &p_surfaces, int p_blend_shape_count) override {
		RID mesh = RSG::mesh_storage->mesh_allocate();

		// TODO once we have RSG::mesh_storage, add can_create_resources_async and call here instead of texture_storage!!

		if (Thread::get_caller_id() == server_thread || RSG::texture_storage->can_create_resources_async()) {
			if (Thread::get_caller_id() == server_thread) {
				command_queue.flush_if_pending();
			}
			RSG::mesh_storage->mesh_initialize(mesh);
			RSG::mesh_storage->mesh_set_blend_shape_count(mesh, p_blend_shape_count);
			for (int i = 0; i < p_surfaces.size(); i++) {
				RSG::mesh_storage->mesh_add_surface(mesh, p_surfaces[i]);
			}
		} else {
			command_queue.push(RSG::mesh_storage, &RendererMeshStorage::mesh_initialize, mesh);
			command_queue.push(RSG::mesh_storage, &RendererMeshStorage::mesh_set_blend_shape_count, mesh, p_blend_shape_count);
			for (int i = 0; i < p_surfaces.size(); i++) {
				command_queue.push(RSG::mesh_storage, &RendererMeshStorage::mesh_add_surface, mesh, p_surfaces[i]);
			}
		}

		return mesh;
	}

	FUNC3(mesh_surface_set_material, RID, int, RID)
	FUNC1(mesh_clear, RID)
	FUNC2(mesh_add_surface, RID, const SurfaceData &)
	FUNC2(mesh_set_blend_shape_count, RID, int)
	FUNC2RC(SurfaceData, mesh_get_surface, RID, int)
	FUNC2(mesh_set_path, RID, const String &)
	FUNC1RC(String, mesh_get_path, RID)
  
	FUNC2(mesh_set_blend_shape_mode, RID, BlendShapeMode)

#undef server_name
#undef ServerName

#define ServerName RenderingMethod
#define server_name RSG::scene
  FUNCRIDSPLIT(scenario)

  FUNC2(scenario_set_environment, RID, RID)
  FUNC2(scenario_set_camera_attributes, RID, RID)
  FUNC2(scenario_set_fallback_environment, RID, RID)
  FUNC2(scenario_set_compositor, RID, RID)

  FUNCRIDSPLIT(compositor)
  FUNCRIDSPLIT(compositor_effect)
  /* ENVIRONMENT */

  FUNCRIDSPLIT(environment)

  FUNC2(environment_set_background, RID, EnvironmentBG)
  FUNC2(environment_set_sky, RID, RID)
  FUNC2(environment_set_sky_custom_fov, RID, float)
  FUNC2(environment_set_sky_orientation, RID, const Basis&)
  FUNC2(environment_set_bg_color, RID, const Color&)
  FUNC3(environment_set_bg_energy, RID, float, float)
  FUNC2(environment_set_canvas_max_layer, RID, int)
  FUNC6(environment_set_ambient_light, RID, const Color&, EnvironmentAmbientSource, float, float, EnvironmentReflectionSource)

  /* Instance API*/
  FUNCRIDSPLIT(instance)

  FUNC2(instance_set_base, RID, RID)
  FUNC2(instance_set_scenario, RID, RID)
  FUNC2(instance_set_layer_mask, RID, uint32_t)
  FUNC3(instance_set_pivot_data, RID, float, bool)
  FUNC2(instance_set_transform, RID, const Transform3D&)
  FUNC2(instance_attach_object_instance_id, RID, ObjectID)
  FUNC3(instance_set_blend_shape_weight, RID, int, float)
  FUNC3(instance_set_surface_override_material, RID, int, RID)
  FUNC2(instance_set_visible, RID, bool)

  FUNC2(instance_set_custom_aabb, RID, AABB)

  FUNC2(instance_attach_skeleton, RID, RID)

  FUNC2(instance_set_extra_visibility_margin, RID, real_t)
  FUNC2(instance_set_visibility_parent, RID, RID)

  FUNC2(instance_set_ignore_culling, RID, bool)
  FUNC2(instance_geometry_set_material_override, RID, RID)
  FUNC2(instance_geometry_set_material_overlay, RID, RID)
  FUNC4(instance_geometry_set_lightmap, RID, RID, const Rect2&, int)
  // shader instance
  FUNC3(instance_geometry_set_shader_parameter, RID, const StringName &, const Variant &)
	FUNC2RC(Variant, instance_geometry_get_shader_parameter, RID, const StringName &)
	FUNC2RC(Variant, instance_geometry_get_shader_parameter_default_value, RID, const StringName &)
	FUNC2C(instance_geometry_get_shader_parameter_list, RID, List<PropertyInfo> *)


  RenderingSystemDefault(bool p_create_thread = false);
  ~RenderingSystemDefault();
  virtual void init() override;
  virtual void free(RID p_rid) override;
  virtual void finish() override;
  virtual void draw(bool p_swap_buffers, double frame_step) override;
  virtual void tick() override;
  virtual bool has_changed() const override;
  virtual void sync() override;
  virtual uint64_t get_rendering_info(RenderingInfo p_info) override;

  virtual void set_default_clear_color(const Color& p_color) override;
  virtual Color get_default_clear_color() const override;

 private:
  void _free(RID p_rid);
  void _finish();
  void _thread_exit() { exit = true; }
  void _thread_loop(void*);  // 一个 thread 化的loop，用于多线程
  void _assign_mt_ids(WorkerThreadPool::TaskID p_tid);
  void _init();
  void _draw(bool p_swap_buffers, double frame_step);
  void _run_post_draw_steps();
};

}  // namespace lain
