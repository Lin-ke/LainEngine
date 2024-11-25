
#ifndef TEXTURE_H
#define TEXTURE_H

#include "core/io/file_access.h"
#include "core/io/resource.h"
#include "core/io/resource_loader.h"
#include "core/os/mutex.h"
#include "core/os/thread_safe.h"
#include "core/math/rect2.h"
#include "core/io/image.h"
namespace lain {
	class Texture : public Resource {
		Texture() {}

	};

	class Texture2D :public Texture{
		LCLASS(Texture2D, Texture);
		OBJ_SAVE_TYPE(Texture2D); // Saves derived classes with common type so they can be interchanged.

		virtual int get_width() const;
		virtual int get_height() const;
		virtual Size2 get_size() const;

		virtual bool is_pixel_opaque(int p_x, int p_y) const;

		virtual bool has_alpha() const;
		virtual void draw(RID p_canvas_item, const Point2& p_pos, const Color& p_modulate = Color(1, 1, 1), bool p_transpose = false) const;
		virtual void draw_rect(RID p_canvas_item, const Rect2& p_rect, bool p_tile = false, const Color& p_modulate = Color(1, 1, 1), bool p_transpose = false) const;
		virtual void draw_rect_region(RID p_canvas_item, const Rect2& p_rect, const Rect2& p_src_rect, const Color& p_modulate = Color(1, 1, 1), bool p_transpose = false, bool p_clip_uv = true) const;
		virtual bool get_rect_region(const Rect2& p_rect, const Rect2& p_src_rect, Rect2& r_rect, Rect2& r_src_rect) const;

		virtual Ref<Image> get_image() const { return Ref<Image>(); }

		virtual Ref<Resource> create_placeholder() const;

		Texture2D();
	};
}

#endif // TEXTURE_H
