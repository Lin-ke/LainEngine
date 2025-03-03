/**************************************************************************/
/*  forward_id_storage.h                                                  */
/**************************************************************************/

#ifndef FORWARD_ID_STORAGE_H
#define FORWARD_ID_STORAGE_H

#include "function/render/rendering_system/utilities.h"
namespace lain{

class RendererSceneRenderRD;

namespace RendererRD {

typedef int32_t ForwardID;

enum ForwardIDType {
	FORWARD_ID_TYPE_OMNI_LIGHT,
	FORWARD_ID_TYPE_SPOT_LIGHT,
	FORWARD_ID_TYPE_REFLECTION_PROBE,
	FORWARD_ID_TYPE_DECAL,
	FORWARD_ID_MAX,
};

class ForwardIDStorage {
private:
	static ForwardIDStorage *singleton;

public:
	static ForwardIDStorage *get_singleton() { return singleton; }

	ForwardIDStorage(){singleton = this;}
	virtual ~ForwardIDStorage() { singleton = nullptr; }

	virtual RendererRD::ForwardID allocate_forward_id(RendererRD::ForwardIDType p_type) { return -1; }
	virtual void free_forward_id(RendererRD::ForwardIDType p_type, RendererRD::ForwardID p_id) {}
	virtual void map_forward_id(RendererRD::ForwardIDType p_type, RendererRD::ForwardID p_id, uint32_t p_index, uint64_t p_last_pass) {}
	virtual bool uses_forward_ids() const { return false; }
};

} // namespace RendererRD
}

#endif // FORWARD_ID_STORAGE_H
