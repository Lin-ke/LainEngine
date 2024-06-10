#pragma once
#ifndef __MESH_H__
#define __MESH_H__
#include "core/io/resource.h"
#include "core/math/triangle_mesh.h"
namespace lain {

	// RS::get_singleton()->material_set_shader(_get_material(), rid);

class Mesh : public Resource {
	LCLASS(Mesh, Resource);
	RES_BASE_EXTENSION("material")

	mutable Ref<TriangleMesh> triangle_mesh; //cached
	mutable Vector<Ref<TriangleMesh>> surface_triangle_meshes; //cached 

};
}

#endif