#ifndef TEXTURE_STORAGE_RD_H
#define TEXTURE_STORAGE_RD_H
#include "function/render/rendering_system/texture_storage_api.h"
namespace lain::RendererRD {
class LightStorage;
class MaterialStorage;

class TextureStorage : public RendererTextureStorage {
  static TextureStorage* singleton;

 public:
  TextureStorage();
  virtual ~TextureStorage();
  static TextureStorage* get_singleton() { return singleton; }
  bool owns_decal(RID p_rid);
  bool free(RID p_rid);
bool owns_texture(RID p_rid);
  /* Texture API */
  // 这里的Texture 是比较特殊的 render system 所使用的几种

  enum TextureType { TYPE_2D, TYPE_LAYERED, TYPE_3D };
  // render system 内部的texture
  struct RenderTarget;
  struct Texture {
    TextureType type;
    RS::TextureLayeredType layered_type = RS::TEXTURE_LAYERED_2D_ARRAY;
    // RD
    RenderingDevice::TextureType rd_type;
    RID rd_texture;
    RID rd_texture_srgb;  // SRGB 格式与 原图共享纹理
    RenderingDevice::DataFormat rd_format;
    RenderingDevice::DataFormat rd_format_srgb;  // SRGB只在有SRGB格式的情况下
    RD::TextureView rd_view;
    // Image
    Image::Format format;            // 原始format
    Image::Format validated_format;  // 经过验证的format 例如，一些情况下RGB不能使用，需要convert成RGBA
    // Data
    int width;
    int height;
    int depth;
    int layers;
    int mipmaps;

    int height_2d;  //used for 2D
    int width_2d;

    struct BufferSlice3D {  // 记录3D纹理在一个buffer中怎么存储的（偏移s）
      Size2i size;          // 该slice的大小
      uint32_t offset = 0;
      uint32_t buffer_size = 0;
    };
    Vector<BufferSlice3D> buffer_slices_3d;
    uint32_t buffer_size_3d = 0;  // 总量大小

    RenderTarget* render_target = nullptr;
    bool is_render_target = false;
    Ref<Image> image_cache_2d;
    String path;

    // callbacks @todo

    void cleanup() {
      if (RD::get_singleton()->texture_is_valid(rd_texture_srgb)) {
        //erase this first, as it's a dependency of the one below
        RD::get_singleton()->free(rd_texture_srgb);
      }
      if (RD::get_singleton()->texture_is_valid(rd_texture)) {
        RD::get_singleton()->free(rd_texture);
      }
      // if (canvas_texture) {
      // 	memdelete(canvas_texture);
      // }
    }
  };
  mutable RID_Alloc<Texture, true> texture_owner;
  Texture* get_texture(RID p_rid) { return texture_owner.get_or_null(p_rid); };
  // 从Image::Format 到RDFormat
  // 有哪些rgb 分量； 是 srgb 空间还是 unorm 空间
  struct TextureToRDFormat {
    RD::DataFormat format;
    RD::DataFormat format_srgb;
    RD::TextureSwizzle swizzle_r;
    RD::TextureSwizzle swizzle_g;
    RD::TextureSwizzle swizzle_b;
    RD::TextureSwizzle swizzle_a;
    TextureToRDFormat() {
      format = RD::DATA_FORMAT_MAX;
      format_srgb = RD::DATA_FORMAT_MAX;
      swizzle_r = RD::TEXTURE_SWIZZLE_R;
      swizzle_g = RD::TEXTURE_SWIZZLE_G;
      swizzle_b = RD::TEXTURE_SWIZZLE_B;
      swizzle_a = RD::TEXTURE_SWIZZLE_A;
    }
  };
  // 从RDformat 获得 
  struct TextureFromRDFormat {
    Image::Format image_format;
    RD::DataFormat rd_format;
    RD::DataFormat rd_format_srgb;
    RD::TextureSwizzle swizzle_r;
    RD::TextureSwizzle swizzle_g;
    RD::TextureSwizzle swizzle_b;
    RD::TextureSwizzle swizzle_a;
    TextureFromRDFormat() {
      image_format = Image::FORMAT_MAX;
      rd_format = RD::DATA_FORMAT_MAX;
      rd_format_srgb = RD::DATA_FORMAT_MAX;
      swizzle_r = RD::TEXTURE_SWIZZLE_R;
      swizzle_g = RD::TEXTURE_SWIZZLE_G;
      swizzle_b = RD::TEXTURE_SWIZZLE_B;
      swizzle_a = RD::TEXTURE_SWIZZLE_A;
    }
  };
  virtual bool can_create_resources_async() const override;
  virtual RID texture_allocate() override;
  virtual void texture_free(RID p_rid) override;

  virtual void texture_2d_initialize(RID p_texture, const Ref<Image>& p_image) override;
  virtual void texture_2d_layered_initialize(RID p_texture, const Vector<Ref<Image>>& p_layers, RS::TextureLayeredType p_layered_type) override;
  virtual void texture_3d_initialize(RID p_texture, Image::Format, int p_width, int p_height, int p_depth, bool p_mipmaps, const Vector<Ref<Image>>& p_data) override;

  virtual void texture_2d_update(RID p_texture, const Ref<Image>& p_image, int p_layer = 0) override;
  virtual void texture_3d_update(RID p_texture, const Vector<Ref<Image>>& p_data) override;

  virtual Ref<Image> texture_2d_get(RID p_texture) const override;
  virtual Ref<Image> texture_2d_layer_get(RID p_texture, int p_layer) const override;
  virtual Vector<Ref<Image>> texture_3d_get(RID p_texture) const override;

  // placeholder
 private:
  RID placeholder_texture_2d = RID();
  RID placeholder_texture_2d_layered = RID();
  RID placeholder_texture_3d = RID();

 public:
  virtual void texture_2d_placeholder_initialize(RID p_texture) override;
  virtual void texture_2d_layered_placeholder_initialize(RID p_texture, RS::TextureLayeredType p_layered_type) override;
  virtual void texture_3d_placeholder_initialize(RID p_texture) override;

  virtual void texture_replace(RID p_texture, RID p_by_texture) override;
  virtual void texture_set_size_override(RID p_texture, int p_width, int p_height) override;

  virtual void texture_set_path(RID p_texture, const String& p_path) override;
  virtual String texture_get_path(RID p_texture) const override;

  virtual Image::Format texture_get_format(RID p_texture) const override;
  virtual void texture_rd_initialize(RID p_texture, const RID& p_rd_texture, const RS::TextureLayeredType p_layer_type = RS::TEXTURE_LAYERED_2D_ARRAY) override;
  virtual RID texture_get_rd_texture(RID p_texture, bool p_srgb = false) const override;
  virtual uint64_t texture_get_native_handle(RID p_texture, bool p_srgb = false) const override;

  /* RENDER TARGET */
  /* RENDER TARGET */
  /* RENDER TARGET */

  struct RenderTarget {
    Size2i size;
    uint32_t view_count;
    // RenderingDevice 的 texture
    RID color;  // 如果use hdr 是DATA_FORMAT_R16G16B16A16_SFLOAT的
    Vector<RID> color_slices;
    RID color_multisample;  // Needed when 2D MSAA is enabled.
    // (为什么还要保存两份引用)
    RS::ViewportMSAA msaa = RS::VIEWPORT_MSAA_DISABLED;  // 2D MSAA mode
    bool msaa_needs_resolve = false;                     // 2D MSAA needs resolved

    //used for retrieving from CPU
    RD::DataFormat color_format = RD::DATA_FORMAT_R4G4_UNORM_PACK8;
    RD::DataFormat color_format_srgb = RD::DATA_FORMAT_R4G4_UNORM_PACK8;
    Image::Format image_format = Image::FORMAT_L8;

    bool is_transparent = false;
    bool use_hdr = false;

    bool sdf_enabled = false;

    RID backbuffer;  //used for effects
    RID backbuffer_fb; // mipmap 0 的 形成 的 framebuffer
    RID backbuffer_mipmap0; // backbuffer的slice

    Vector<RID> backbuffer_mipmaps;

    RID framebuffer_uniform_set;
    RID backbuffer_uniform_set;

    // RID sdf_buffer_write;
    // RID sdf_buffer_write_fb;
    // RID sdf_buffer_process[2];
    // RID sdf_buffer_read;
    // RID sdf_buffer_process_uniform_sets[2];
    // RS::ViewportSDFOversize sdf_oversize = RS::VIEWPORT_SDF_OVERSIZE_120_PERCENT;
    // RS::ViewportSDFScale sdf_scale = RS::VIEWPORT_SDF_SCALE_50_PERCENT;
    // Size2i process_size;

    struct RTOverridden {
      RID color;
      RID depth;
      RID velocity;

      // In a multiview scenario, which is the most likely where we
      // override our destination textures, we need to obtain slices
      // for each layer of these textures.
      // These are likely changing every frame as we loop through
      // texture chains hence we add a cache to manage these slices.
      // For this we define a key using the RID of the texture and
      // the layer for which we create a slice.
      struct SliceKey {
        RID rid;
        uint32_t layer = 0;

        bool operator==(const SliceKey& p_val) const { return (rid == p_val.rid) && (layer == p_val.layer); }

        static uint32_t hash(const SliceKey& p_val) {
          uint32_t h = hash_one_uint64(p_val.rid.get_id());
          h = hash_murmur3_one_32(p_val.layer, h);
          return hash_fmix32(h);
        }

        SliceKey() {}
        SliceKey(RID p_rid, uint32_t p_layer) {
          rid = p_rid;
          layer = p_layer;
        }
      };

      mutable HashMap<SliceKey, RID, SliceKey> cached_slices;
    } overridden;
    //texture generated for this owner (nor RD).
    RID texture;  // 如果一个texture是render target 那么 该texture 存有对 render target的引用
    bool was_used;
    //clear request
    bool clear_requested;
    Color clear_color;

    RID get_framebuffer();
  };
  mutable RID_Owner<RenderTarget> render_target_owner;
  RenderTarget* get_render_target(RID p_rid) const { return render_target_owner.get_or_null(p_rid); };

 private:
  // RD::texture_update
  void _texture_2d_update(RID p_texture, const Ref<Image>& p_image, int p_layer = 0, bool p_immediate = false);
  // 改格式
  void _texture_format_from_rd(RD::DataFormat p_rd_format, TextureFromRDFormat& r_format);
  // 改格式到r_format，顺便复制一个Image
  Ref<Image> _validate_texture_format(const Ref<Image>& p_image, TextureToRDFormat& r_format);
  // 需要删除它管理的backbuffer， color, color_msaa
  void _clear_render_target(RenderTarget* rt);
  // 删除现在的texture，调用clear，并创建新的texture，color etc.
  void _update_render_target(RenderTarget* rt);
  // 
	void _create_render_target_backbuffer(RenderTarget *rt);


 public:
  /* RENDER TARGET API */

  bool owns_render_target(RID p_rid) const { return render_target_owner.owns(p_rid); };

  virtual RID render_target_create() override;
  virtual void render_target_free(RID p_rid) override;

  virtual void render_target_set_position(RID p_render_target, int p_x, int p_y) override;
  virtual Point2i render_target_get_position(RID p_render_target) const override;
  virtual void render_target_set_size(RID p_render_target, int p_width, int p_height, uint32_t p_view_count) override;
  virtual Size2i render_target_get_size(RID p_render_target) const override;
  RID render_target_get_texture(RID p_render_target);
  void render_target_set_override(RID p_render_target, RID p_color_texture, RID p_depth_texture, RID p_velocity_texture);
  RID render_target_get_override_color(RID p_render_target) const;
  RID render_target_get_override_depth(RID p_render_target) const;
  virtual RID render_target_get_override_velocity(RID p_render_target) const override;
  
  //  rt->overridden.depth

  virtual void render_target_set_transparent(RID p_render_target, bool p_is_transparent) override;
  virtual bool render_target_get_transparent(RID p_render_target) const override;
  virtual void render_target_set_direct_to_screen(RID p_render_target, bool p_direct_to_screen) override;
  virtual bool render_target_get_direct_to_screen(RID p_render_target) const override;
  virtual bool render_target_was_used(RID p_render_target) const override;
  virtual void render_target_set_as_unused(RID p_render_target) override;
  virtual void render_target_set_msaa(RID p_render_target, RS::ViewportMSAA p_msaa) override;
  virtual RS::ViewportMSAA render_target_get_msaa(RID p_render_target) const override;
  virtual void render_target_set_msaa_needs_resolve(RID p_render_target, bool p_needs_resolve) override;
  virtual bool render_target_get_msaa_needs_resolve(RID p_render_target) const override;
  virtual void render_target_do_msaa_resolve(RID p_render_target) override;
  virtual void render_target_set_use_hdr(RID p_render_target, bool p_use_hdr) override;
  virtual bool render_target_is_using_hdr(RID p_render_target) const override;
	RID render_target_get_override_depth_slice(RID p_render_target, const uint32_t p_layer) const;

  void render_target_copy_to_back_buffer(RID p_render_target, const Rect2i& p_region, bool p_gen_mipmaps);
  void render_target_clear_back_buffer(RID p_render_target, const Rect2i& p_region, const Color& p_color);
  void render_target_gen_back_buffer_mipmaps(RID p_render_target, const Rect2i& p_region);
  RID render_target_get_back_buffer_uniform_set(RID p_render_target, RID p_base_shader);

  virtual void render_target_request_clear(RID p_render_target, const Color& p_clear_color) override;
  virtual bool render_target_is_clear_requested(RID p_render_target) override;
  virtual Color render_target_get_clear_request_color(RID p_render_target) override;
  virtual void render_target_disable_clear_request(RID p_render_target) override;
  virtual void render_target_do_clear_request(RID p_render_target) override;

	RID render_target_get_rd_texture(RID p_render_target);

};
}  // namespace lain::RendererRD
#endif