#ifndef RES_COMPOSITOR_H
#define RES_COMPOSITOR_H
#include "function/render/scene/render_data_api.h"
#include "core/io/resource.h"
namespace lain{
class CompositorEffect : public Resource {
LCLASS(CompositorEffect, Resource);
public:
	enum EffectCallbackType {
		EFFECT_CALLBACK_TYPE_PRE_OPAQUE,
		EFFECT_CALLBACK_TYPE_POST_OPAQUE,
		EFFECT_CALLBACK_TYPE_POST_SKY,
		EFFECT_CALLBACK_TYPE_PRE_TRANSPARENT,
		EFFECT_CALLBACK_TYPE_POST_TRANSPARENT,
		EFFECT_CALLBACK_TYPE_MAX
	};
private:
	RID rid;
	bool enabled = true;
	EffectCallbackType effect_callback_type = EFFECT_CALLBACK_TYPE_POST_TRANSPARENT;

	bool access_resolved_color = false;
	bool access_resolved_depth = false;
	bool needs_motion_vectors = false;
	bool needs_normal_roughness = false;
	bool needs_separate_specular = false;
public:
	virtual RID GetRID() const override { return rid; }

	void set_enabled(bool p_enabled);
	bool get_enabled() const;

	void set_effect_callback_type(EffectCallbackType p_callback_type);
	EffectCallbackType get_effect_callback_type() const;

	void set_access_resolved_color(bool p_enabled);
	bool get_access_resolved_color() const;

	void set_access_resolved_depth(bool p_enabled);
	bool get_access_resolved_depth() const;

	void set_needs_motion_vectors(bool p_enabled);
	bool get_needs_motion_vectors() const;

	void set_needs_normal_roughness(bool p_enabled);
	bool get_needs_normal_roughness() const;

	void set_needs_separate_specular(bool p_enabled);
	bool get_needs_separate_specular() const;

	CompositorEffect();
	~CompositorEffect();

};


class Compositor : public Resource {
	LCLASS(Compositor, Resource);

private:
	RID compositor;

	// Compositor effects
	LocalVector<Ref<CompositorEffect>> effects;

protected:
	static void _bind_methods();

public:
	virtual RID GetRID() const override { return compositor; }

	Compositor();
	~Compositor();

	// Compositor effects
	void set_compositor_effects(const Vector<Ref<CompositorEffect>> &p_compositor_effects);
	Vector<Ref<CompositorEffect>> get_compositor_effects() const;
};

}
#endif