#pragma once
#ifndef __MESH_H__
#define __MESH_H__

#include "runtime/core/math/transform.h"
#include "runtime/core/meta/reflection/reflection.h"
#include "core/templates/vector.h"
#include "core/string/ustring.h"
namespace lain
{
    REFLECTION_TYPE(SubMeshRes)
        CLASS(SubMeshRes, Fields)
    {
        REFLECTION_BODY(SubMeshRes);

    public:
        String m_obj_file_ref;
        Transform   m_transform;
        String m_material;
    };

    REFLECTION_TYPE(MeshComponentRes)
        CLASS(MeshComponentRes, Fields)
    {
        REFLECTION_BODY(MeshComponentRes);

    public:
        Vector<SubMeshRes> m_sub_meshes;
    };
} // namespace Piccolo
#endif // !__MESH_H__
