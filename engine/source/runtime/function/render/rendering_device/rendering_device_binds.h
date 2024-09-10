#ifndef RENDERING_DEVICE_BINDS_H
#define RENDERING_DEVICE_BINDS_H

#include "core/object/refcounted.h"
#include "rendering_device.h"

#define RD_SETGET(m_type, m_member)          \
  void set_##m_member(m_type p_##m_member) { \
    base.m_member = p_##m_member;            \
  }                                          \
  m_type get_##m_member() const {            \
    return base.m_member;                    \
  }

#define RD_SETGET_SUB(m_type, m_sub, m_member)         \
  void set_##m_sub##_##m_member(m_type p_##m_member) { \
    base.m_sub.m_member = p_##m_member;                \
  }                                                    \
  m_type get_##m_sub##_##m_member() const {            \
    return base.m_sub.m_member;                        \
  }
namespace lain {

class RDTextureFormat : public RefCounted {
  LCLASS(RDTextureFormat, RefCounted)

  friend class RenderingDevice;
  friend class RenderSceneBuffersRD;

  RD::TextureFormat base;

 public:
  RD_SETGET(RD::DataFormat, format)
  RD_SETGET(uint32_t, width)
  RD_SETGET(uint32_t, height)
  RD_SETGET(uint32_t, depth)
  RD_SETGET(uint32_t, array_layers)
  RD_SETGET(uint32_t, mipmaps)
  RD_SETGET(RD::TextureType, texture_type)
  RD_SETGET(RD::TextureSamples, samples)
  RD_SETGET(BitField<RenderingDevice::TextureUsageBits>, usage_bits)

  void add_shareable_format(RD::DataFormat p_format) { base.shareable_formats.push_back(p_format); }
  void remove_shareable_format(RD::DataFormat p_format) { base.shareable_formats.erase(p_format); }
};


class RDTextureView : public RefCounted {
	LCLASS(RDTextureView, RefCounted)

	friend class RenderingDevice;
	friend class RenderSceneBuffersRD;

	RD::TextureView base;

public:
	RD_SETGET(RD::DataFormat, format_override)
	RD_SETGET(RD::TextureSwizzle, swizzle_r)
	RD_SETGET(RD::TextureSwizzle, swizzle_g)
	RD_SETGET(RD::TextureSwizzle, swizzle_b)
	RD_SETGET(RD::TextureSwizzle, swizzle_a)
protected:
	
};


class RDFramebufferPass : public RefCounted {
	LCLASS(RDFramebufferPass, RefCounted)
	friend class RenderingDevice;
	friend class FramebufferCacheRD;

	RD::FramebufferPass base;

public:
	RD_SETGET(PackedInt32Array, color_attachments)
	RD_SETGET(PackedInt32Array, input_attachments)
	RD_SETGET(PackedInt32Array, resolve_attachments)
	RD_SETGET(PackedInt32Array, preserve_attachments)
	RD_SETGET(int32_t, depth_attachment)
protected:
	enum {
		ATTACHMENT_UNUSED = -1
	};

};


}  // namespace lain

#endif