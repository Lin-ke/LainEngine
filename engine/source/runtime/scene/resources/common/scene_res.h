#pragma once
#ifndef SCENE_RES_H
#define SCENE_RES_H
#include "core/meta/reflection/reflection.h"
#include "gobject_res.h"
#include "core/string/string_name.h"
// ���飺reflection typeֻ�������л�/�����л�������������
namespace lain {
    REFLECTION_TYPE(PackedSceneRes)
        CLASS(PackedSceneRes, Fields) {
        REFLECTION_BODY(PackedSceneRes);
public:
        Dictionary head;
        Vector<ExtRes> ext_res;
        Vector<GObjectInstanceRes> gobjects;

    };

}
#endif // !SCENE_RES_H
