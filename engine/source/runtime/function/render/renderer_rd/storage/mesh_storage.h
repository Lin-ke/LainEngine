#ifndef MESH_STORAGE_IMPL_H
#define MESH_STORAGE_IMPL_H
#include "function/render/rendering_system/mesh_storage_api.h"

namespace lain::RendererRD {
class MeshStorage : public RendererMeshStorage {
  static MeshStorage* p_singleton;
  struct Mesh {};

  mutable RID_Owner<Mesh, true> mesh_owner;

 public:
  MeshStorage() { p_singleton = this; }
  ~MeshStorage() {}
  static MeshStorage* get_singleton() { return p_singleton; };
	bool owns_mesh(RID p_rid) { return mesh_owner.owns(p_rid); };
	bool owns_multimesh(RID p_rid) { return false; };
  
  virtual RID mesh_allocate() override;
  virtual void mesh_initialize(RID p_rid) override;
  virtual void mesh_free(RID p_rid) override;
};
}  // namespace lain::RendererRD
#endif