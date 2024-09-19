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
  bool owns_texture(RID p_rid);
  /* Texture API */
  // 这里的Texture 是比较特殊的 render system 所使用的几种

  enum TextureType { TYPE_2D, TYPE_LAYERED, TYPE_3D };
  // render system 内部的texture
  class RenderTarget;
  struct Texture {
    TextureType type ;
    RS::TextureLayeredType layered_type = RS::TEXTURE_LAYERED_2D_ARRAY;
    // RD
    RenderingDevice::TextureType rd_type;
    RID rd_texture;
    RID rd_texture_srgb; // SRGB 格式与 原图共享纹理
    RenderingDevice::DataFormat rd_format;
    RenderingDevice::DataFormat rd_format_srgb; // SRGB只在有SRGB格式的情况下
    RD::TextureView rd_view;
    // Image
    Image::Format format;
    Image::Format validated_format;
    // Data
    int width;
    int height;
    int depth;
    int layers;
    int mipmaps;

    int height_2d; //used for 2D
    int width_2d;

    struct BufferSlice3D { // 记录3D纹理在一个buffer中怎么存储的（偏移s）
      Size2i size; // 该slice的大小
      uint32_t offset = 0;
      uint32_t buffer_size = 0;
    };
    Vector<BufferSlice3D> buffer_slices_3d;
    uint32_t buffer_size_3d = 0; // 总量大小

    RenderTarget* render_target = nullptr;
    bool is_render_target = false;
    Ref<Image> image_cache_2d;
    String path;

    // callbacks @todo

    void cleanup();
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

  virtual void texture_2d_initialize(RID p_texture, const Ref<Image> &p_image) override;
	virtual void texture_2d_layered_initialize(RID p_texture, const Vector<Ref<Image>> &p_layers, RS::TextureLayeredType p_layered_type) override;
	virtual void texture_3d_initialize(RID p_texture, Image::Format, int p_width, int p_height, int p_depth, bool p_mipmaps, const Vector<Ref<Image>> &p_data) override;
	virtual void texture_proxy_initialize(RID p_texture, RID p_base) override; //all slices, then all the mipmaps, must be coherent

	virtual void texture_2d_update(RID p_texture, const Ref<Image> &p_image, int p_layer = 0) override;
	virtual void texture_3d_update(RID p_texture, const Vector<Ref<Image>> &p_data) override;
	virtual void texture_proxy_update(RID p_proxy, RID p_base) override;

  private:
  // RD::texture_update
  void _texture_2d_update(RID p_texture, const Ref<Image>& p_image, int p_layer = 0, bool p_immediate = false);
  // 改格式
  void _texture_format_from_rd(RD::DataFormat p_rd_format, TextureFromRDFormat& r_format);
  // 改格式到r_format，顺便复制一个Image
  Ref<Image> _validate_texture_format(const Ref<Image>& p_image, TextureToRDFormat& r_format);
};
}  // namespace lain::RendererRD
#endif