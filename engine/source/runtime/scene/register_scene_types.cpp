#include "core/meta/serializer/serializer.h"
// scene
#include "scene/resources/io/resource_format_text.h"
#include "scene/resources/io/resource_format_shader.h"
#include "scene/resources/io/resource_importer_obj.h"
#include "scene/resources/common/shader_include.h"
#include "register_scene_types.h"
#include "core/meta/class_db.h"
#include "scene/3d/camera_3d.h"
#include "scene/3d/visual_instance_3d.h"
#include "scene/3d/light_3d.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/world_environment.h"

// function types
#include "function/shader/shader_types.h"

// resource types
#include "scene/resources/common/mesh.h"
#include "scene/resources/common/material.h"
#include "scene/resources/common/primitive_meshes.h"
#include "scene/resources/common/image_texture.h"
#include "scene/resources/common/sky_material.h"

namespace lain{
	static Ref<ResourceFormatLoaderText> resource_format_loader_text;
	static Ref<ResourceFormatSaverText> resource_format_saver_text;
	static Ref<ResourceFormatLoaderShader> resource_format_loader_shader;
	static Ref<ResourceFormatSaverShader> resource_format_saver_shader;
	static Ref<ResourceFormatLoaderOBJ> resource_format_loader_obj;

	static Ref<ResourceFormatLoaderShaderInclude> resource_format_loader_shader_include;
	static Ref<ResourceFormatSaverShaderInclude> resource_format_saver_shader_include;
	

  void register_scene_types(){
    resource_format_loader_text.instantiate();
		ResourceLoader::add_resource_format_loader(resource_format_loader_text);


		resource_format_saver_text.instantiate();
		ResourceSaver::add_resource_format_saver(resource_format_saver_text);

		resource_format_loader_shader.instantiate();
		ResourceLoader::add_resource_format_loader(resource_format_loader_shader);

		resource_format_saver_shader.instantiate();
		ResourceSaver::add_resource_format_saver(resource_format_saver_shader);

		resource_format_loader_shader_include.instantiate();
		ResourceLoader::add_resource_format_loader(resource_format_loader_shader_include);

		resource_format_saver_shader_include.instantiate();
		ResourceSaver::add_resource_format_saver(resource_format_saver_shader_include);

		auto& importers = ResourceLoader::ext_to_loader_idx;
		// L_JSON(importers);
		resource_format_loader_obj.instantiate();
		ResourceLoader::add_resource_format_loader(resource_format_loader_obj);
		
  }
	shader::ShaderTypes *shader_types = nullptr;

  void register_system_types() {
		shader_types = memnew(shader::ShaderTypes);
	
	GDREGISTER_CLASS(GObject);

	GDREGISTER_CLASS(GObject3D);
	GDREGISTER_VIRTUAL_CLASS(VisualInstance3D);
	GDREGISTER_CLASS(DirectionalLight3D);
	GDREGISTER_CLASS(Camera3D);
	GDREGISTER_CLASS(Camera3DMove);

	GDREGISTER_CLASS(OmniLight3D);
	GDREGISTER_CLASS(MeshInstance3D);

	GDREGISTER_VIRTUAL_CLASS(Mesh);
	GDREGISTER_VIRTUAL_CLASS(PrimitiveMesh);
	GDREGISTER_CLASS(CapsuleMesh);
	GDREGISTER_CLASS(ArrayMesh);
	GDREGISTER_CLASS(SphereMesh);

	
	GDREGISTER_VIRTUAL_CLASS(Material);
	GDREGISTER_ABSTRACT_CLASS(BaseMaterial3D);
	GDREGISTER_CLASS(StandardMaterial3D);
	GDREGISTER_CLASS(ORMMaterial3D);
	GDREGISTER_CLASS(PanoramaSkyMaterial);
	GDREGISTER_CLASS(PhysicalSkyMaterial);

	SceneTree::add_idle_callback(BaseMaterial3D::flush_changes);

	GDREGISTER_CLASS(WorldEnvironment);
	
	GDREGISTER_VIRTUAL_CLASS(Texture);
	GDREGISTER_VIRTUAL_CLASS(Texture2D);
	GDREGISTER_CLASS(Sky);
	GDREGISTER_CLASS(ImageTexture);

	GDREGISTER_CLASS(Environment);

	BaseMaterial3D::init_shaders();

	
	}

  }  // namespace lain