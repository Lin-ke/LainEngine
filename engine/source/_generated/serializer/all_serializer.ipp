#pragma once
#include "_generated\serializer\quaternion.serializer.gen.h"
#include "_generated\serializer\transform.serializer.gen.h"
#include "_generated\serializer\vector3.serializer.gen.h"
#include "_generated\serializer\axis_aligned.serializer.gen.h"
#include "_generated\serializer\meta_example.serializer.gen.h"
#include "_generated\serializer\vector4.serializer.gen.h"
#include "_generated\serializer\vector2.serializer.gen.h"
#include "_generated\serializer\matrix4.serializer.gen.h"
namespace Lain{
    template<>
    Json Serializer::write(const Quaternion& instance){
        Json::object  ret_context;
        
        ret_context.insert_or_assign("w", Serializer::write(instance.w));
        ret_context.insert_or_assign("x", Serializer::write(instance.x));
        ret_context.insert_or_assign("y", Serializer::write(instance.y));
        ret_context.insert_or_assign("z", Serializer::write(instance.z));
        return  Json(ret_context);
    }
    template<>
    Quaternion& Serializer::read(const Json& json_context, Quaternion& instance){
        assert(json_context.is_object());
        
        if(!json_context["w"].is_null()){
            Serializer::read(json_context["w"], instance.w);
        }
        if(!json_context["x"].is_null()){
            Serializer::read(json_context["x"], instance.x);
        }
        if(!json_context["y"].is_null()){
            Serializer::read(json_context["y"], instance.y);
        }
        if(!json_context["z"].is_null()){
            Serializer::read(json_context["z"], instance.z);
        }
        return instance;
    }
    template<>
    Json Serializer::write(const Transform& instance){
        Json::object  ret_context;
        
        ret_context.insert_or_assign("position", Serializer::write(instance.m_position));
        ret_context.insert_or_assign("scale", Serializer::write(instance.m_scale));
        ret_context.insert_or_assign("rotation", Serializer::write(instance.m_rotation));
        return  Json(ret_context);
    }
    template<>
    Transform& Serializer::read(const Json& json_context, Transform& instance){
        assert(json_context.is_object());
        
        if(!json_context["position"].is_null()){
            Serializer::read(json_context["position"], instance.m_position);
        }
        if(!json_context["scale"].is_null()){
            Serializer::read(json_context["scale"], instance.m_scale);
        }
        if(!json_context["rotation"].is_null()){
            Serializer::read(json_context["rotation"], instance.m_rotation);
        }
        return instance;
    }
    template<>
    Json Serializer::write(const Vector3& instance){
        Json::object  ret_context;
        
        ret_context.insert_or_assign("x", Serializer::write(instance.x));
        ret_context.insert_or_assign("y", Serializer::write(instance.y));
        ret_context.insert_or_assign("z", Serializer::write(instance.z));
        return  Json(ret_context);
    }
    template<>
    Vector3& Serializer::read(const Json& json_context, Vector3& instance){
        assert(json_context.is_object());
        
        if(!json_context["x"].is_null()){
            Serializer::read(json_context["x"], instance.x);
        }
        if(!json_context["y"].is_null()){
            Serializer::read(json_context["y"], instance.y);
        }
        if(!json_context["z"].is_null()){
            Serializer::read(json_context["z"], instance.z);
        }
        return instance;
    }
    template<>
    Json Serializer::write(const AxisAlignedBox& instance){
        Json::object  ret_context;
        
        ret_context.insert_or_assign("center", Serializer::write(instance.m_center));
        ret_context.insert_or_assign("half_extent", Serializer::write(instance.m_half_extent));
        ret_context.insert_or_assign("min_corner", Serializer::write(instance.m_min_corner));
        ret_context.insert_or_assign("max_corner", Serializer::write(instance.m_max_corner));
        return  Json(ret_context);
    }
    template<>
    AxisAlignedBox& Serializer::read(const Json& json_context, AxisAlignedBox& instance){
        assert(json_context.is_object());
        
        if(!json_context["center"].is_null()){
            Serializer::read(json_context["center"], instance.m_center);
        }
        if(!json_context["half_extent"].is_null()){
            Serializer::read(json_context["half_extent"], instance.m_half_extent);
        }
        if(!json_context["min_corner"].is_null()){
            Serializer::read(json_context["min_corner"], instance.m_min_corner);
        }
        if(!json_context["max_corner"].is_null()){
            Serializer::read(json_context["max_corner"], instance.m_max_corner);
        }
        return instance;
    }
    template<>
    Json Serializer::write(const BaseTest& instance){
        Json::object  ret_context;
        
        ret_context.insert_or_assign("int", Serializer::write(instance.m_int));
        Json::array m_int_vector_json;
        for (auto& item : instance.m_int_vector){
            m_int_vector_json.emplace_back(Serializer::write(item));
        }
        ret_context.insert_or_assign("int_vector",m_int_vector_json);
        
        return  Json(ret_context);
    }
    template<>
    BaseTest& Serializer::read(const Json& json_context, BaseTest& instance){
        assert(json_context.is_object());
        
        if(!json_context["int"].is_null()){
            Serializer::read(json_context["int"], instance.m_int);
        }
        if(!json_context["int_vector"].is_null()){
            assert(json_context["int_vector"].is_array());
            Json::array array_m_int_vector = json_context["int_vector"].array_items();
            instance.m_int_vector.resize(array_m_int_vector.size());
            for (size_t index=0; index < array_m_int_vector.size();++index){
                Serializer::read(array_m_int_vector[index], instance.m_int_vector[index]);
            }
        }
        return instance;
    }
    template<>
    Json Serializer::write(const Test1& instance){
        Json::object  ret_context;
        auto&&  json_context_0 = Serializer::write(*(Lain::BaseTest*)&instance);
        assert(json_context_0.is_object());
        auto&& json_context_map_0 = json_context_0.object_items();
        ret_context.insert(json_context_map_0.begin() , json_context_map_0.end());
        ret_context.insert_or_assign("char", Serializer::write(instance.m_char));
        return  Json(ret_context);
    }
    template<>
    Test1& Serializer::read(const Json& json_context, Test1& instance){
        assert(json_context.is_object());
        Serializer::read(json_context,*(Lain::BaseTest*)&instance);
        if(!json_context["char"].is_null()){
            Serializer::read(json_context["char"], instance.m_char);
        }
        return instance;
    }
    template<>
    Json Serializer::write(const Test2& instance){
        Json::object  ret_context;
        auto&&  json_context_0 = Serializer::write(*(Lain::BaseTest*)&instance);
        assert(json_context_0.is_object());
        auto&& json_context_map_0 = json_context_0.object_items();
        ret_context.insert(json_context_map_0.begin() , json_context_map_0.end());
        Json::array m_test_base_array_json;
        for (auto& item : instance.m_test_base_array){
            m_test_base_array_json.emplace_back(Serializer::write(item));
        }
        ret_context.insert_or_assign("test_base_array",m_test_base_array_json);
        
        return  Json(ret_context);
    }
    template<>
    Test2& Serializer::read(const Json& json_context, Test2& instance){
        assert(json_context.is_object());
        Serializer::read(json_context,*(Lain::BaseTest*)&instance);
        if(!json_context["test_base_array"].is_null()){
            assert(json_context["test_base_array"].is_array());
            Json::array array_m_test_base_array = json_context["test_base_array"].array_items();
            instance.m_test_base_array.resize(array_m_test_base_array.size());
            for (size_t index=0; index < array_m_test_base_array.size();++index){
                Serializer::read(array_m_test_base_array[index], instance.m_test_base_array[index]);
            }
        }
        return instance;
    }
    template<>
    Json Serializer::write(const Vector4& instance){
        Json::object  ret_context;
        
        ret_context.insert_or_assign("x", Serializer::write(instance.x));
        ret_context.insert_or_assign("y", Serializer::write(instance.y));
        ret_context.insert_or_assign("z", Serializer::write(instance.z));
        ret_context.insert_or_assign("w", Serializer::write(instance.w));
        return  Json(ret_context);
    }
    template<>
    Vector4& Serializer::read(const Json& json_context, Vector4& instance){
        assert(json_context.is_object());
        
        if(!json_context["x"].is_null()){
            Serializer::read(json_context["x"], instance.x);
        }
        if(!json_context["y"].is_null()){
            Serializer::read(json_context["y"], instance.y);
        }
        if(!json_context["z"].is_null()){
            Serializer::read(json_context["z"], instance.z);
        }
        if(!json_context["w"].is_null()){
            Serializer::read(json_context["w"], instance.w);
        }
        return instance;
    }
    template<>
    Json Serializer::write(const Vector2& instance){
        Json::object  ret_context;
        
        ret_context.insert_or_assign("x", Serializer::write(instance.x));
        ret_context.insert_or_assign("y", Serializer::write(instance.y));
        return  Json(ret_context);
    }
    template<>
    Vector2& Serializer::read(const Json& json_context, Vector2& instance){
        assert(json_context.is_object());
        
        if(!json_context["x"].is_null()){
            Serializer::read(json_context["x"], instance.x);
        }
        if(!json_context["y"].is_null()){
            Serializer::read(json_context["y"], instance.y);
        }
        return instance;
    }
    template<>
    Json Serializer::write(const Matrix4x4_& instance){
        Json::object  ret_context;
        
        ret_context.insert_or_assign("v0", Serializer::write(instance.v0));
        ret_context.insert_or_assign("v1", Serializer::write(instance.v1));
        ret_context.insert_or_assign("v2", Serializer::write(instance.v2));
        ret_context.insert_or_assign("v3", Serializer::write(instance.v3));
        ret_context.insert_or_assign("v4", Serializer::write(instance.v4));
        ret_context.insert_or_assign("v5", Serializer::write(instance.v5));
        ret_context.insert_or_assign("v6", Serializer::write(instance.v6));
        ret_context.insert_or_assign("v7", Serializer::write(instance.v7));
        ret_context.insert_or_assign("v8", Serializer::write(instance.v8));
        ret_context.insert_or_assign("v9", Serializer::write(instance.v9));
        ret_context.insert_or_assign("v10", Serializer::write(instance.v10));
        ret_context.insert_or_assign("v11", Serializer::write(instance.v11));
        ret_context.insert_or_assign("v12", Serializer::write(instance.v12));
        ret_context.insert_or_assign("v13", Serializer::write(instance.v13));
        ret_context.insert_or_assign("v14", Serializer::write(instance.v14));
        ret_context.insert_or_assign("v15", Serializer::write(instance.v15));
        return  Json(ret_context);
    }
    template<>
    Matrix4x4_& Serializer::read(const Json& json_context, Matrix4x4_& instance){
        assert(json_context.is_object());
        
        if(!json_context["v0"].is_null()){
            Serializer::read(json_context["v0"], instance.v0);
        }
        if(!json_context["v1"].is_null()){
            Serializer::read(json_context["v1"], instance.v1);
        }
        if(!json_context["v2"].is_null()){
            Serializer::read(json_context["v2"], instance.v2);
        }
        if(!json_context["v3"].is_null()){
            Serializer::read(json_context["v3"], instance.v3);
        }
        if(!json_context["v4"].is_null()){
            Serializer::read(json_context["v4"], instance.v4);
        }
        if(!json_context["v5"].is_null()){
            Serializer::read(json_context["v5"], instance.v5);
        }
        if(!json_context["v6"].is_null()){
            Serializer::read(json_context["v6"], instance.v6);
        }
        if(!json_context["v7"].is_null()){
            Serializer::read(json_context["v7"], instance.v7);
        }
        if(!json_context["v8"].is_null()){
            Serializer::read(json_context["v8"], instance.v8);
        }
        if(!json_context["v9"].is_null()){
            Serializer::read(json_context["v9"], instance.v9);
        }
        if(!json_context["v10"].is_null()){
            Serializer::read(json_context["v10"], instance.v10);
        }
        if(!json_context["v11"].is_null()){
            Serializer::read(json_context["v11"], instance.v11);
        }
        if(!json_context["v12"].is_null()){
            Serializer::read(json_context["v12"], instance.v12);
        }
        if(!json_context["v13"].is_null()){
            Serializer::read(json_context["v13"], instance.v13);
        }
        if(!json_context["v14"].is_null()){
            Serializer::read(json_context["v14"], instance.v14);
        }
        if(!json_context["v15"].is_null()){
            Serializer::read(json_context["v15"], instance.v15);
        }
        return instance;
    }

}

