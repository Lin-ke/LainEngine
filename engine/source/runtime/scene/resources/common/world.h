#ifndef WORLD_2D_H
#define WORLD_2D_H

#include "core/io/resource.h"
#include "core/templates/hash_set.h"
//#include "servers/physics_server_2d.h"
#include "environment.h"
#include "compositor.h"
namespace lain {

//class VisibleOnScreenNotifier2D;
class Viewport;
//struct SpatialIndexer2D;
class CameraAttributes;
class World2D : public Resource {
	LCLASS(World2D, Resource);

	RID canvas;
	mutable RID space;
	mutable RID navigation_map;

	HashSet<Viewport*> viewports;

protected:
	friend class Viewport;

public:
	RID get_canvas() const;
	RID get_space() const;
	RID get_navigation_map() const;

	//PhysicsDirectSpaceState2D* get_direct_space_state();

	void register_viewport(Viewport* p_viewport);
	void remove_viewport(Viewport* p_viewport);

	_FORCE_INLINE_ const HashSet<Viewport*>& get_viewports() { return viewports; }

	World2D();
	~World2D();
};
class CameraAttributes;
class Camera3D;

class World3D : public Resource{
	LCLASS(World3D, Resource);
	friend class Camera3D;

private:
	RID scenario;
	mutable RID space;
	mutable RID navigation_map;

	Ref<Environment> environment;
	Ref<Environment> fallback_environment;
	Ref<CameraAttributes> camera_attributes;
	Ref<Compositor> compositor;

	HashSet<Camera3D*> cameras;
	
	void _register_camera(Camera3D *p_camera);
	void _remove_camera(Camera3D *p_camera);
	public:
	Ref<CameraAttributes> get_camera_attributes() const;
};
}

#endif // WORLD_2D_H
