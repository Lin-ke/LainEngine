#include "mesh_storage.h"
using namespace lain::RendererRD;
using namespace lain;

MeshStorage* MeshStorage::p_singleton = nullptr;
RID lain::RendererRD::MeshStorage::mesh_allocate() {
    return mesh_owner.allocate_rid();
}

void MeshStorage::mesh_initialize(RID p_rid) {
	mesh_owner.initialize_rid(p_rid, Mesh());
}

void MeshStorage::mesh_free(RID p_rid) {
    mesh_owner.free(p_rid);
}