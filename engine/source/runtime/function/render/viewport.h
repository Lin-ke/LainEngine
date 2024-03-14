#pragma once
#ifndef __VIEW_PORT_H__
#define __VIEW_PORT_H__
#include "base.h"
#include "core/math/transform.h"
#include "core/templates/hash_map.h"
#include "resource/manager/rid.h"
#include "core/object/refcount.h"
// part of the 2D render pipeline
namespace lain{

	class RendererViewport {
	public:
		struct ViewPort {

			struct CanvasBase{};
			struct CanvasData {
				CanvasBase* canvas = nullptr;
				Transform2D transform;
				int layer;
				int sublayer;
			};
			RID self;
			RID parent;
			RID camera;
			RID senario;
			RID render_target;
			RID render_target_texture;

			// options

			//Ref<RenderSceneBuffers> render_buffers;
			// size; configs;
			int m_to_screen;

			HashMap<RID, CanvasData> m_canvas_map;
			// info


		};

	};
}

#endif