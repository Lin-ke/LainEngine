#pragma once
#ifndef VIEWPORT_TEXTURE_H
#define VIEWPORT_TEXTURE_H
#include "core/scene/object/gobject.h"
#include "core/scene/object/gobject_path.h"
#include "scene/resources/common/world.h"
#include "texture.h"
namespace lain {
	class Viewport;
	class ViewportTexture : public Texture2D {
		LCLASS(ViewportTexture, Texture2D);

		GObjectPath path;

		friend class Viewport;
		Viewport* vp = nullptr;
		bool vp_pending = false;
		bool vp_changed = false;

		void _setup_local_to_scene(const GObject* p_loc_scene);

		mutable RID proxy_ph;
		mutable RID proxy;

	protected:
		static void _bind_methods();

		virtual void reset_local_to_scene() override;

	public:
		void set_viewport_path_in_scene(const GObjectPath& p_path);
		GObjectPath get_viewport_path_in_scene() const;

		virtual int get_width() const override;
		virtual int get_height() const override;
		virtual Size2 get_size() const override;
		virtual RID GetRID() const override;

		virtual bool has_alpha() const override;

		virtual Ref<Image> get_image() const override;

		ViewportTexture();
		~ViewportTexture();
	};

	class ViewPort :public GObject{
		LCLASS(ViewPort, GObject);
	public:
		// some enums
		enum Scaling3DMode {
			SCALING_3D_MODE_BILINEAR,
			SCALING_3D_MODE_FSR,
			SCALING_3D_MODE_FSR2,
			SCALING_3D_MODE_MAX
		};

		enum PositionalShadowAtlasQuadrantSubdiv {
			SHADOW_ATLAS_QUADRANT_SUBDIV_DISABLED,
			SHADOW_ATLAS_QUADRANT_SUBDIV_1,
			SHADOW_ATLAS_QUADRANT_SUBDIV_4,
			SHADOW_ATLAS_QUADRANT_SUBDIV_16,
			SHADOW_ATLAS_QUADRANT_SUBDIV_64,
			SHADOW_ATLAS_QUADRANT_SUBDIV_256,
			SHADOW_ATLAS_QUADRANT_SUBDIV_1024,
			SHADOW_ATLAS_QUADRANT_SUBDIV_MAX,
		};

		enum MSAA {
			MSAA_DISABLED,
			MSAA_2X,
			MSAA_4X,
			MSAA_8X,
			// 16x MSAA is not supported due to its high cost and driver bugs.
			MSAA_MAX
		};

		enum ScreenSpaceAA {
			SCREEN_SPACE_AA_DISABLED,
			SCREEN_SPACE_AA_FXAA,
			SCREEN_SPACE_AA_MAX
		};

		enum RenderInfo {
			RENDER_INFO_OBJECTS_IN_FRAME,
			RENDER_INFO_PRIMITIVES_IN_FRAME,
			RENDER_INFO_DRAW_CALLS_IN_FRAME,
			RENDER_INFO_MAX
		};

		enum RenderInfoType {
			RENDER_INFO_TYPE_VISIBLE,
			RENDER_INFO_TYPE_SHADOW,
			RENDER_INFO_TYPE_CANVAS,
			RENDER_INFO_TYPE_MAX
		};

		enum DebugDraw {
			DEBUG_DRAW_DISABLED,
			DEBUG_DRAW_UNSHADED,
			DEBUG_DRAW_LIGHTING,
			DEBUG_DRAW_OVERDRAW,
			DEBUG_DRAW_WIREFRAME,
			DEBUG_DRAW_NORMAL_BUFFER,
			DEBUG_DRAW_VOXEL_GI_ALBEDO,
			DEBUG_DRAW_VOXEL_GI_LIGHTING,
			DEBUG_DRAW_VOXEL_GI_EMISSION,
			DEBUG_DRAW_SHADOW_ATLAS,
			DEBUG_DRAW_DIRECTIONAL_SHADOW_ATLAS,
			DEBUG_DRAW_SCENE_LUMINANCE,
			DEBUG_DRAW_SSAO,
			DEBUG_DRAW_SSIL,
			DEBUG_DRAW_PSSM_SPLITS,
			DEBUG_DRAW_DECAL_ATLAS,
			DEBUG_DRAW_SDFGI,
			DEBUG_DRAW_SDFGI_PROBES,
			DEBUG_DRAW_GI_BUFFER,
			DEBUG_DRAW_DISABLE_LOD,
			DEBUG_DRAW_CLUSTER_OMNI_LIGHTS,
			DEBUG_DRAW_CLUSTER_SPOT_LIGHTS,
			DEBUG_DRAW_CLUSTER_DECALS,
			DEBUG_DRAW_CLUSTER_REFLECTION_PROBES,
			DEBUG_DRAW_OCCLUDERS,
			DEBUG_DRAW_MOTION_VECTORS,
			DEBUG_DRAW_INTERNAL_BUFFER,
		};

		enum DefaultCanvasItemTextureFilter {
			DEFAULT_CANVAS_ITEM_TEXTURE_FILTER_NEAREST,
			DEFAULT_CANVAS_ITEM_TEXTURE_FILTER_LINEAR,
			DEFAULT_CANVAS_ITEM_TEXTURE_FILTER_LINEAR_WITH_MIPMAPS,
			DEFAULT_CANVAS_ITEM_TEXTURE_FILTER_NEAREST_WITH_MIPMAPS,
			DEFAULT_CANVAS_ITEM_TEXTURE_FILTER_MAX
		};

		enum DefaultCanvasItemTextureRepeat {
			DEFAULT_CANVAS_ITEM_TEXTURE_REPEAT_DISABLED,
			DEFAULT_CANVAS_ITEM_TEXTURE_REPEAT_ENABLED,
			DEFAULT_CANVAS_ITEM_TEXTURE_REPEAT_MIRROR,
			DEFAULT_CANVAS_ITEM_TEXTURE_REPEAT_MAX,
		};

		enum SDFOversize {
			SDF_OVERSIZE_100_PERCENT,
			SDF_OVERSIZE_120_PERCENT,
			SDF_OVERSIZE_150_PERCENT,
			SDF_OVERSIZE_200_PERCENT,
			SDF_OVERSIZE_MAX
		};

		enum SDFScale {
			SDF_SCALE_100_PERCENT,
			SDF_SCALE_50_PERCENT,
			SDF_SCALE_25_PERCENT,
			SDF_SCALE_MAX
		};

		enum {
			SUBWINDOW_CANVAS_LAYER = 1024
		};

		enum VRSMode {
			VRS_DISABLED,
			VRS_TEXTURE,
			VRS_XR,
			VRS_MAX
		};

		private:
			friend class ViewportTexture;
			Size2i size = Size2i(512, 512);
			Size2i size_2d_override;

			Ref<World2D> world_2d;
			RID texture_rid;
			// settings
			PositionalShadowAtlasQuadrantSubdiv positional_shadow_atlas_quadrant_subdiv[4];
			MSAA msaa_2d = MSAA_DISABLED;
			MSAA msaa_3d = MSAA_DISABLED;
			ScreenSpaceAA screen_space_aa = SCREEN_SPACE_AA_DISABLED;
			bool use_taa = false;
			Scaling3DMode scaling_3d_mode = SCALING_3D_MODE_BILINEAR;
			float scaling_3d_scale = 1.0;
			float fsr_sharpness = 0.2f;
			float texture_mipmap_bias = 0.0f;
			bool use_debanding = false;
			float mesh_lod_threshold = 1.0;
			bool use_occlusion_culling = false;

			Ref<ViewportTexture> default_texture;
			HashSet<ViewportTexture*> viewport_textures;


			SDFOversize sdf_oversize = SDF_OVERSIZE_120_PERCENT;
			SDFScale sdf_scale = SDF_SCALE_50_PERCENT;

			uint32_t canvas_cull_mask = 0xffffffff; // by default show everything
			DefaultCanvasItemTextureFilter default_canvas_item_texture_filter = DEFAULT_CANVAS_ITEM_TEXTURE_FILTER_LINEAR;
			DefaultCanvasItemTextureRepeat default_canvas_item_texture_repeat = DEFAULT_CANVAS_ITEM_TEXTURE_REPEAT_DISABLED;
	protected:
		void _set_size(const Size2i& p_size, const Size2i& p_size_2d_override, bool p_allocated);

	};
}
#endif