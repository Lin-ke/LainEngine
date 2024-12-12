
#define VERSION_NAME "0"

#include "material.h"
#include "core/engine/engine.h"
using namespace lain;


void Material::set_next_pass(const Ref<Material>& p_pass) {
  for (Ref<Material> pass_child = p_pass; pass_child != nullptr; pass_child = pass_child->get_next_pass()) {
    ERR_FAIL_COND_MSG(pass_child == this, "Can't set as next_pass one of its parents to prevent crashes due to recursive loop.");
  }

  if (next_pass == p_pass) {
    return;
  }

  next_pass = p_pass;
  RID next_pass_rid;
  if (next_pass.is_valid()) {
    next_pass_rid = next_pass->GetRID();
  }
  RS::get_singleton()->material_set_next_pass(material, next_pass_rid);
}

Ref<Material> Material::get_next_pass() const {
	return next_pass;
}


void Material::set_render_priority(int p_priority) {
	ERR_FAIL_COND(p_priority < RENDER_PRIORITY_MIN);
	ERR_FAIL_COND(p_priority > RENDER_PRIORITY_MAX);
	render_priority = p_priority;
	RS::get_singleton()->material_set_render_priority(material, p_priority);
}


int Material::get_render_priority() const {
	return render_priority;
}

RID Material::GetRID() const {
	return material;
}

RID lain::Material::get_shader_rid() const {
  return RID();
}

Ref<Resource> lain::Material::create_placeholder() const {
  return Ref<Resource>();
}

Shader::Mode lain::Material::get_shader_mode() const {
  return Shader::Mode();
}

Material::Material() {
	material = RS::get_singleton()->material_create();
	render_priority = 0;
}
Material::~Material() {
	ERR_FAIL_NULL(RS::get_singleton());
	RS::get_singleton()->free(material);
}

bool lain::Material::_can_do_next_pass() const {
  return false;
}

bool lain::Material::_can_use_render_priority() const {
  return false;
}

void lain::Material::_mark_ready()
{
  init_state = INIT_STATE_READY;
}

void ShaderMaterial::set_shader_parameter(const StringName &p_param, const Variant &p_value) {
	if (p_value.get_type() == Variant::NIL) {
		param_cache.erase(p_param);
		RS::get_singleton()->material_set_param(_get_material(), p_param, Variant());
	} else {
		Variant *v = param_cache.getptr(p_param);
		if (!v) {
			// Never assigned, also update the remap cache.
			remap_cache["shader_parameter/" + p_param.operator String()] = p_param;
			param_cache.insert(p_param, p_value);
		} else {
			*v = p_value;
		}

		if (p_value.get_type() == Variant::OBJECT) {
			RID tex_rid = p_value;
			if (tex_rid == RID()) {
				param_cache.erase(p_param);
				RS::get_singleton()->material_set_param(_get_material(), p_param, Variant());
			} else {
				RS::get_singleton()->material_set_param(_get_material(), p_param, tex_rid);
			}
		} else {
			RS::get_singleton()->material_set_param(_get_material(), p_param, p_value);
		}
	}
}


Variant ShaderMaterial::get_shader_parameter(const StringName &p_param) const {
	if (param_cache.has(p_param)) {
		return param_cache[p_param];
	} else {
		return Variant();
	}
}


bool ShaderMaterial::_can_do_next_pass() const {
	return shader.is_valid() && shader->get_mode() == Shader::MODE_SPATIAL;
}

bool ShaderMaterial::_can_use_render_priority() const {
	return shader.is_valid() && shader->get_mode() == Shader::MODE_SPATIAL;
}

Shader::Mode ShaderMaterial::get_shader_mode() const {
	if (shader.is_valid()) {
		return shader->get_mode();
	} else {
		return Shader::MODE_SPATIAL;
	}
}


void ShaderMaterial::set_shader(const Ref<Shader> &p_shader) {
	// Only connect/disconnect the signal when running in the editor.
	// This can be a slow operation, and `notify_property_list_changed()` (which is called by `_shader_changed()`)
	// does nothing in non-editor builds anyway. See GH-34741 for details.
	if (shader.is_valid() && Engine::GetSingleton()->is_editor_hint()) {
		// shader->disconnect_changed(callable_mp(this, &ShaderMaterial::_shader_changed));
	}

	shader = p_shader;

	RID rid;
	if (shader.is_valid()) {
		rid = shader->GetRID();

		if (Engine::GetSingleton()->is_editor_hint()) {
			// shader->connect_changed(callable_mp(this, &ShaderMaterial::_shader_changed));
		}
	}

	RS::get_singleton()->material_set_shader(_get_material(), rid);
	notify_property_list_changed(); //properties for shader exposed
	emit_changed();
}
Ref<Shader> ShaderMaterial::get_shader() const {
	return shader;
}

RID ShaderMaterial::get_shader_rid() const {
	if (shader.is_valid()) {
		return shader->GetRID();
	} else {
		return RID();
	}
}

ShaderMaterial::ShaderMaterial() {
}

ShaderMaterial::~ShaderMaterial() {
}
/////////////////////////////////

Mutex BaseMaterial3D::material_mutex;
SelfList<BaseMaterial3D>::List BaseMaterial3D::dirty_materials;
HashMap<BaseMaterial3D::MaterialKey, BaseMaterial3D::ShaderData, BaseMaterial3D::MaterialKey> BaseMaterial3D::shader_map;
BaseMaterial3D::ShaderNames *BaseMaterial3D::shader_names = nullptr;

void BaseMaterial3D::init_shaders() {
	shader_names = memnew(ShaderNames);

	shader_names->albedo = "albedo";
	shader_names->specular = "specular";
	shader_names->roughness = "roughness";
	shader_names->metallic = "metallic";
	shader_names->emission = "emission";
	shader_names->emission_energy = "emission_energy";
	shader_names->normal_scale = "normal_scale";
	shader_names->rim = "rim";
	shader_names->rim_tint = "rim_tint";
	shader_names->clearcoat = "clearcoat";
	shader_names->clearcoat_roughness = "clearcoat_roughness";
	shader_names->anisotropy = "anisotropy_ratio";
	shader_names->heightmap_scale = "heightmap_scale";
	shader_names->subsurface_scattering_strength = "subsurface_scattering_strength";
	shader_names->backlight = "backlight";
	shader_names->refraction = "refraction";
	shader_names->point_size = "point_size";
	shader_names->uv1_scale = "uv1_scale";
	shader_names->uv1_offset = "uv1_offset";
	shader_names->uv2_scale = "uv2_scale";
	shader_names->uv2_offset = "uv2_offset";
	shader_names->uv1_blend_sharpness = "uv1_blend_sharpness";
	shader_names->uv2_blend_sharpness = "uv2_blend_sharpness";

	shader_names->particles_anim_h_frames = "particles_anim_h_frames";
	shader_names->particles_anim_v_frames = "particles_anim_v_frames";
	shader_names->particles_anim_loop = "particles_anim_loop";
	shader_names->heightmap_min_layers = "heightmap_min_layers";
	shader_names->heightmap_max_layers = "heightmap_max_layers";
	shader_names->heightmap_flip = "heightmap_flip";

	shader_names->grow = "grow";

	shader_names->ao_light_affect = "ao_light_affect";

	shader_names->proximity_fade_distance = "proximity_fade_distance";
	shader_names->distance_fade_min = "distance_fade_min";
	shader_names->distance_fade_max = "distance_fade_max";

	shader_names->msdf_pixel_range = "msdf_pixel_range";
	shader_names->msdf_outline_size = "msdf_outline_size";

	shader_names->metallic_texture_channel = "metallic_texture_channel";
	shader_names->ao_texture_channel = "ao_texture_channel";
	shader_names->clearcoat_texture_channel = "clearcoat_texture_channel";
	shader_names->rim_texture_channel = "rim_texture_channel";
	shader_names->heightmap_texture_channel = "heightmap_texture_channel";
	shader_names->refraction_texture_channel = "refraction_texture_channel";

	shader_names->transmittance_color = "transmittance_color";
	shader_names->transmittance_depth = "transmittance_depth";
	shader_names->transmittance_boost = "transmittance_boost";

	shader_names->texture_names[TEXTURE_ALBEDO] = "texture_albedo";
	shader_names->texture_names[TEXTURE_METALLIC] = "texture_metallic";
	shader_names->texture_names[TEXTURE_ROUGHNESS] = "texture_roughness";
	shader_names->texture_names[TEXTURE_EMISSION] = "texture_emission";
	shader_names->texture_names[TEXTURE_NORMAL] = "texture_normal";
	shader_names->texture_names[TEXTURE_RIM] = "texture_rim";
	shader_names->texture_names[TEXTURE_CLEARCOAT] = "texture_clearcoat";
	shader_names->texture_names[TEXTURE_FLOWMAP] = "texture_flowmap";
	shader_names->texture_names[TEXTURE_AMBIENT_OCCLUSION] = "texture_ambient_occlusion";
	shader_names->texture_names[TEXTURE_HEIGHTMAP] = "texture_heightmap";
	shader_names->texture_names[TEXTURE_SUBSURFACE_SCATTERING] = "texture_subsurface_scattering";
	shader_names->texture_names[TEXTURE_SUBSURFACE_TRANSMITTANCE] = "texture_subsurface_transmittance";
	shader_names->texture_names[TEXTURE_BACKLIGHT] = "texture_backlight";
	shader_names->texture_names[TEXTURE_REFRACTION] = "texture_refraction";
	shader_names->texture_names[TEXTURE_DETAIL_MASK] = "texture_detail_mask";
	shader_names->texture_names[TEXTURE_DETAIL_ALBEDO] = "texture_detail_albedo";
	shader_names->texture_names[TEXTURE_DETAIL_NORMAL] = "texture_detail_normal";
	shader_names->texture_names[TEXTURE_ORM] = "texture_orm";

	shader_names->alpha_scissor_threshold = "alpha_scissor_threshold";
	shader_names->alpha_hash_scale = "alpha_hash_scale";

	shader_names->alpha_antialiasing_edge = "alpha_antialiasing_edge";
	shader_names->albedo_texture_size = "albedo_texture_size";
}

HashMap<uint64_t, Ref<StandardMaterial3D>> BaseMaterial3D::materials_for_2d;

void BaseMaterial3D::finish_shaders() {
	materials_for_2d.clear();

	dirty_materials.clear();

	memdelete(shader_names);
	shader_names = nullptr;
}

void BaseMaterial3D::_update_shader() {
	MaterialKey mk = _compute_key();
	if (mk == current_key) {
		return; //no update required in the end
	}

	if (shader_map.has(current_key)) {
		shader_map[current_key].users--;
		if (shader_map[current_key].users == 0) {
			//deallocate shader, as it's no longer in use
			RS::get_singleton()->free(shader_map[current_key].shader);
			shader_map.erase(current_key);
		}
	}

	current_key = mk;

	if (shader_map.has(mk)) {
		RS::get_singleton()->material_set_shader(_get_material(), shader_map[mk].shader);
		shader_map[mk].users++;
		return;
	}

	String texfilter_str;
	// Force linear filtering for the heightmap texture, as the heightmap effect
	// looks broken with nearest-neighbor filtering (with and without Deep Parallax).
	String texfilter_height_str;
	switch (texture_filter) {
		case TEXTURE_FILTER_NEAREST:
			texfilter_str = "filter_nearest";
			texfilter_height_str = "filter_linear";
			break;
		case TEXTURE_FILTER_LINEAR:
			texfilter_str = "filter_linear";
			texfilter_height_str = "filter_linear";
			break;
		case TEXTURE_FILTER_NEAREST_WITH_MIPMAPS:
			texfilter_str = "filter_nearest_mipmap";
			texfilter_height_str = "filter_linear_mipmap";
			break;
		case TEXTURE_FILTER_LINEAR_WITH_MIPMAPS:
			texfilter_str = "filter_linear_mipmap";
			texfilter_height_str = "filter_linear_mipmap";
			break;
		case TEXTURE_FILTER_NEAREST_WITH_MIPMAPS_ANISOTROPIC:
			texfilter_str = "filter_nearest_mipmap_anisotropic";
			texfilter_height_str = "filter_linear_mipmap_anisotropic";
			break;
		case TEXTURE_FILTER_LINEAR_WITH_MIPMAPS_ANISOTROPIC:
			texfilter_str = "filter_linear_mipmap_anisotropic";
			texfilter_height_str = "filter_linear_mipmap_anisotropic";
			break;
		case TEXTURE_FILTER_MAX:
			break; // Internal value, skip.
	}

	if (flags[FLAG_USE_TEXTURE_REPEAT]) {
		texfilter_str += ", repeat_enable";
		texfilter_height_str += ", repeat_enable";
	} else {
		texfilter_str += ", repeat_disable";
		texfilter_height_str += ", repeat_disable";
	}

	// Add a comment to describe the shader origin (useful when converting to ShaderMaterial).
	String code = vformat(
			"// NOTE: Shader automatically converted from %s\n",
			orm ? "ORMMaterial3D" : "StandardMaterial3D");

	// Define shader type and render mode based on property values.
	code += "shader_type spatial;\nrender_mode ";
	switch (blend_mode) {
		case BLEND_MODE_MIX:
			code += "blend_mix";
			break;
		case BLEND_MODE_ADD:
			code += "blend_add";
			break;
		case BLEND_MODE_SUB:
			code += "blend_sub";
			break;
		case BLEND_MODE_MUL:
			code += "blend_mul";
			break;
		case BLEND_MODE_PREMULT_ALPHA:
			code += "blend_premul_alpha";
			break;
		case BLEND_MODE_MAX:
			break; // Internal value, skip.
	}

	DepthDrawMode ddm = depth_draw_mode;
	if (features[FEATURE_REFRACTION]) {
		ddm = DEPTH_DRAW_ALWAYS;
	}

	switch (ddm) {
		case DEPTH_DRAW_OPAQUE_ONLY:
			code += ", depth_draw_opaque";
			break;
		case DEPTH_DRAW_ALWAYS:
			code += ", depth_draw_always";
			break;
		case DEPTH_DRAW_DISABLED:
			code += ", depth_draw_never";
			break;
		case DEPTH_DRAW_MAX:
			break; // Internal value, skip.
	}

	switch (cull_mode) {
		case CULL_BACK:
			code += ", cull_back";
			break;
		case CULL_FRONT:
			code += ", cull_front";
			break;
		case CULL_DISABLED:
			code += ", cull_disabled";
			break;
		case CULL_MAX:
			break; // Internal value, skip.
	}
	switch (diffuse_mode) {
		case DIFFUSE_BURLEY:
			code += ", diffuse_burley";
			break;
		case DIFFUSE_LAMBERT:
			code += ", diffuse_lambert";
			break;
		case DIFFUSE_LAMBERT_WRAP:
			code += ", diffuse_lambert_wrap";
			break;
		case DIFFUSE_TOON:
			code += ", diffuse_toon";
			break;
		case DIFFUSE_MAX:
			break; // Internal value, skip.
	}
	switch (specular_mode) {
		case SPECULAR_SCHLICK_GGX:
			code += ", specular_schlick_ggx";
			break;
		case SPECULAR_TOON:
			code += ", specular_toon";
			break;
		case SPECULAR_DISABLED:
			code += ", specular_disabled";
			break;
		case SPECULAR_MAX:
			break; // Internal value, skip.
	}
	if (features[FEATURE_SUBSURFACE_SCATTERING] && flags[FLAG_SUBSURFACE_MODE_SKIN]) {
		code += ", sss_mode_skin";
	}

	if (shading_mode == SHADING_MODE_UNSHADED) {
		code += ", unshaded";
	}
	if (flags[FLAG_DISABLE_DEPTH_TEST]) {
		code += ", depth_test_disabled";
	}
	if (flags[FLAG_PARTICLE_TRAILS_MODE]) {
		code += ", particle_trails";
	}
	if (shading_mode == SHADING_MODE_PER_VERTEX) {
		code += ", vertex_lighting";
	}
	if (flags[FLAG_DONT_RECEIVE_SHADOWS]) {
		code += ", shadows_disabled";
	}
	if (flags[FLAG_DISABLE_AMBIENT_LIGHT]) {
		code += ", ambient_light_disabled";
	}
	if (flags[FLAG_USE_SHADOW_TO_OPACITY]) {
		code += ", shadow_to_opacity";
	}
	if (flags[FLAG_DISABLE_FOG]) {
		code += ", fog_disabled";
	}

	if (transparency == TRANSPARENCY_ALPHA_DEPTH_PRE_PASS) {
		code += ", depth_prepass_alpha";
	}

	// Although it's technically possible to do alpha antialiasing without using alpha hash or alpha scissor,
	// it is restricted in the base material because it has no use, and abusing it with regular Alpha blending can
	// saturate the MSAA mask.
	if (transparency == TRANSPARENCY_ALPHA_HASH || transparency == TRANSPARENCY_ALPHA_SCISSOR) {
		// Alpha antialiasing is only useful with ALPHA_HASH or ALPHA_SCISSOR.
		if (alpha_antialiasing_mode == ALPHA_ANTIALIASING_ALPHA_TO_COVERAGE) {
			code += ", alpha_to_coverage";
		} else if (alpha_antialiasing_mode == ALPHA_ANTIALIASING_ALPHA_TO_COVERAGE_AND_TO_ONE) {
			code += ", alpha_to_coverage_and_one";
		}
	}

	code += ";\n";

	// Generate list of uniforms.
	code += vformat(R"(
uniform vec4 albedo : source_color;
uniform sampler2D texture_albedo : source_color, %s;
)",
			texfilter_str);

	if (grow_enabled) {
		code += "uniform float grow : hint_range(-16.0, 16.0, 0.001);\n";
	}

	if (proximity_fade_enabled) {
		code += "uniform float proximity_fade_distance : hint_range(0.0, 4096.0, 0.01);\n";
	}
	if (distance_fade != DISTANCE_FADE_DISABLED) {
		code += R"(
uniform float distance_fade_min : hint_range(0.0, 4096.0, 0.01);
uniform float distance_fade_max : hint_range(0.0, 4096.0, 0.01);
)";
	}

	if (flags[FLAG_ALBEDO_TEXTURE_MSDF] && flags[FLAG_UV1_USE_TRIPLANAR]) {
		String msg = "MSDF is not supported on triplanar materials. Ignoring MSDF in favor of triplanar mapping.";
		if (textures[TEXTURE_ALBEDO].is_valid()) {
			WARN_PRINT(vformat("%s (albedo %s): " + msg, GetPath(), textures[TEXTURE_ALBEDO]->GetPath()));
		} else if (!GetPath().is_empty()) {
			WARN_PRINT(vformat("%s: " + msg, GetPath()));
		} else {
			WARN_PRINT(msg);
		}
	}

	if (flags[FLAG_ALBEDO_TEXTURE_MSDF] && !flags[FLAG_UV1_USE_TRIPLANAR]) {
		code += R"(
uniform float msdf_pixel_range : hint_range(1.0, 100.0, 1.0);
uniform float msdf_outline_size : hint_range(0.0, 250.0, 1.0);
)";
	}

	// Alpha scissor is only valid if there is no antialiasing edge.
	// Alpha hash is valid whenever, but not with alpha scissor.
	if (transparency == TRANSPARENCY_ALPHA_SCISSOR) {
		code += "uniform float alpha_scissor_threshold : hint_range(0.0, 1.0, 0.001);\n";
	} else if (transparency == TRANSPARENCY_ALPHA_HASH) {
		code += "uniform float alpha_hash_scale : hint_range(0.0, 2.0, 0.01);\n";
	}
	// If alpha antialiasing isn't off, add in the edge variable.
	if (alpha_antialiasing_mode != ALPHA_ANTIALIASING_OFF &&
			(transparency == TRANSPARENCY_ALPHA_SCISSOR || transparency == TRANSPARENCY_ALPHA_HASH)) {
		code += R"(
uniform float alpha_antialiasing_edge : hint_range(0.0, 1.0, 0.01);
uniform ivec2 albedo_texture_size;
)";
	}

	code += "uniform float point_size : hint_range(0.1, 128.0, 0.1);\n";

	if (!orm) {
		code += vformat(R"(
uniform float roughness : hint_range(0.0, 1.0);
uniform sampler2D texture_metallic : hint_default_white, %s;
uniform vec4 metallic_texture_channel;
)",
				texfilter_str);
		switch (roughness_texture_channel) {
			case TEXTURE_CHANNEL_RED: {
				code += vformat("uniform sampler2D texture_roughness : hint_roughness_r, %s;\n", texfilter_str);
			} break;
			case TEXTURE_CHANNEL_GREEN: {
				code += vformat("uniform sampler2D texture_roughness : hint_roughness_g, %s;\n", texfilter_str);
			} break;
			case TEXTURE_CHANNEL_BLUE: {
				code += vformat("uniform sampler2D texture_roughness : hint_roughness_b, %s;\n", texfilter_str);
			} break;
			case TEXTURE_CHANNEL_ALPHA: {
				code += vformat("uniform sampler2D texture_roughness : hint_roughness_a, %s;\n", texfilter_str);
			} break;
			case TEXTURE_CHANNEL_GRAYSCALE: {
				code += vformat("uniform sampler2D texture_roughness : hint_roughness_gray, %s;\n", texfilter_str);
			} break;
			case TEXTURE_CHANNEL_MAX:
				break; // Internal value, skip.
		}

		code += R"(
uniform float specular : hint_range(0.0, 1.0, 0.01);
uniform float metallic : hint_range(0.0, 1.0, 0.01);
)";
	} else {
		code += "uniform sampler2D texture_orm : hint_roughness_g, " + texfilter_str + ";\n";
	}

	if (billboard_mode == BILLBOARD_PARTICLES) {
		code += R"(
uniform int particles_anim_h_frames : hint_range(1, 128);
uniform int particles_anim_v_frames : hint_range(1, 128);
uniform bool particles_anim_loop;
)";
	}

	if (features[FEATURE_EMISSION]) {
		code += vformat(R"(
uniform sampler2D texture_emission : source_color, hint_default_black, %s;
uniform vec4 emission : source_color;
uniform float emission_energy : hint_range(0.0, 100.0, 0.01);
)",
				texfilter_str);
	}

	if (features[FEATURE_REFRACTION]) {
		code += vformat(R"(
uniform sampler2D texture_refraction : %s;
uniform float refraction : hint_range(-1.0, 1.0, 0.001);
uniform vec4 refraction_texture_channel;
)",
				texfilter_str);
	}

	if (features[FEATURE_REFRACTION]) {
		code += "uniform sampler2D screen_texture : hint_screen_texture, repeat_disable, filter_linear_mipmap;\n";
	}

	if (proximity_fade_enabled) {
		code += "uniform sampler2D depth_texture : hint_depth_texture, repeat_disable, filter_nearest;\n";
	}

	if (features[FEATURE_NORMAL_MAPPING]) {
		code += vformat(R"(
uniform sampler2D texture_normal : hint_roughness_normal, %s;
uniform float normal_scale : hint_range(-16.0, 16.0);
)",
				texfilter_str);
	}
	if (features[FEATURE_RIM]) {
		code += vformat(R"(
uniform float rim : hint_range(0.0, 1.0, 0.01);
uniform float rim_tint : hint_range(0.0, 1.0, 0.01);
uniform sampler2D texture_rim : hint_default_white, %s;
)",
				texfilter_str);
	}
	if (features[FEATURE_CLEARCOAT]) {
		code += vformat(R"(
uniform float clearcoat : hint_range(0.0, 1.0, 0.01);
uniform float clearcoat_roughness : hint_range(0.0, 1.0, 0.01);
uniform sampler2D texture_clearcoat : hint_default_white, %s;
)",
				texfilter_str);
	}
	if (features[FEATURE_ANISOTROPY]) {
		code += vformat(R"(
uniform float anisotropy_ratio : hint_range(0.0, 1.0, 0.01);
uniform sampler2D texture_flowmap : hint_anisotropy, %s;
)",
				texfilter_str);
	}
	if (features[FEATURE_AMBIENT_OCCLUSION]) {
		code += vformat(R"(
uniform sampler2D texture_ambient_occlusion : hint_default_white, %s;
uniform vec4 ao_texture_channel;
uniform float ao_light_affect : hint_range(0.0, 1.0, 0.01);
)",
				texfilter_str);
	}

	if (features[FEATURE_DETAIL]) {
		code += vformat(R"(
uniform sampler2D texture_detail_albedo : source_color, %s;
uniform sampler2D texture_detail_normal : hint_normal, %s;
uniform sampler2D texture_detail_mask : hint_default_white, %s;
)",
				texfilter_str, texfilter_str, texfilter_str);
	}

	if (features[FEATURE_SUBSURFACE_SCATTERING]) {
		code += vformat(R"(
uniform float subsurface_scattering_strength : hint_range(0.0, 1.0, 0.01);
uniform sampler2D texture_subsurface_scattering : hint_default_white, %s;
)",
				texfilter_str);
	}

	if (features[FEATURE_SUBSURFACE_TRANSMITTANCE]) {
		code += vformat(R"(
uniform vec4 transmittance_color : source_color;
uniform float transmittance_depth : hint_range(0.001, 8.0, 0.001);
uniform sampler2D texture_subsurface_transmittance : hint_default_white, %s;
uniform float transmittance_boost : hint_range(0.0, 1.0, 0.01);
)",
				texfilter_str);
	}

	if (features[FEATURE_BACKLIGHT]) {
		code += vformat(R"(
uniform vec4 backlight : source_color;
uniform sampler2D texture_backlight : hint_default_black, %s;
)",
				texfilter_str);
	}

	if (features[FEATURE_HEIGHT_MAPPING]) {
		code += vformat(R"(
uniform sampler2D texture_heightmap : hint_default_black, %s;
uniform float heightmap_scale : hint_range(-16.0, 16.0, 0.001);
uniform int heightmap_min_layers : hint_range(1, 64);
uniform int heightmap_max_layers : hint_range(1, 64);
uniform vec2 heightmap_flip;
)",
				texfilter_height_str);
	}
	if (flags[FLAG_UV1_USE_TRIPLANAR]) {
		code += "varying vec3 uv1_triplanar_pos;\n";
	}
	if (flags[FLAG_UV2_USE_TRIPLANAR]) {
		code += "varying vec3 uv2_triplanar_pos;\n";
	}
	if (flags[FLAG_UV1_USE_TRIPLANAR]) {
		code += R"(
uniform float uv1_blend_sharpness : hint_range(0.0, 150.0, 0.001);
varying vec3 uv1_power_normal;
)";
	}

	if (flags[FLAG_UV2_USE_TRIPLANAR]) {
		code += R"(uniform float uv2_blend_sharpness : hint_range(0.0, 150.0, 0.001);
varying vec3 uv2_power_normal;
)";
	}

	code += R"(
uniform vec3 uv1_scale;
uniform vec3 uv1_offset;
uniform vec3 uv2_scale;
uniform vec3 uv2_offset;
)";

	// Generate vertex shader.
	code += R"(
void vertex() {)";

	if (flags[FLAG_SRGB_VERTEX_COLOR]) {
		code += R"(
	// Vertex Color is sRGB: Enabled
	if (!OUTPUT_IS_SRGB) {
		COLOR.rgb = mix(
				pow((COLOR.rgb + vec3(0.055)) * (1.0 / (1.0 + 0.055)), vec3(2.4)),
				COLOR.rgb * (1.0 / 12.92),
				lessThan(COLOR.rgb, vec3(0.04045)));
	}
)";
	}
	if (flags[FLAG_USE_POINT_SIZE]) {
		code += R"(
	// Use Point Size: Enabled
	POINT_SIZE = point_size;
)";
	}

	if (shading_mode == SHADING_MODE_PER_VERTEX) {
		code += R"(
	// Shading Mode: Per Vertex
	ROUGHNESS = roughness;
)";
	}

	if (!flags[FLAG_UV1_USE_TRIPLANAR]) {
		code += R"(
	UV = UV * uv1_scale.xy + uv1_offset.xy;
)";
	}

	if (detail_uv == DETAIL_UV_2 && !flags[FLAG_UV2_USE_TRIPLANAR]) {
		// Don't add a newline if the UV assignment above is already performed,
		// so that UV1 and UV2 are closer to each other.
		if (flags[FLAG_UV1_USE_TRIPLANAR]) {
			code += "\n";
		}
		code += R"(	// Detail UV Layer: UV2
	UV2 = UV2 * uv2_scale.xy + uv2_offset.xy;
)";
	}

	switch (billboard_mode) {
		case BILLBOARD_DISABLED: {
		} break;
		case BILLBOARD_ENABLED: {
			// `MAIN_CAM_INV_VIEW_MATRIX` is inverse of the camera, even on shadow passes.
			// This ensures the billboard faces the camera when casting shadows.
			code += R"(
	// Billboard Mode: Enabled
	MODELVIEW_MATRIX = VIEW_MATRIX * mat4(
			MAIN_CAM_INV_VIEW_MATRIX[0],
			MAIN_CAM_INV_VIEW_MATRIX[1],
			MAIN_CAM_INV_VIEW_MATRIX[2],
			MODEL_MATRIX[3]);
)";
			if (flags[FLAG_BILLBOARD_KEEP_SCALE]) {
				code += R"(
	// Billboard Keep Scale: Enabled
	MODELVIEW_MATRIX = MODELVIEW_MATRIX * mat4(
			vec4(length(MODEL_MATRIX[0].xyz), 0.0, 0.0, 0.0),
			vec4(0.0, length(MODEL_MATRIX[1].xyz), 0.0, 0.0),
			vec4(0.0, 0.0, length(MODEL_MATRIX[2].xyz), 0.0),
			vec4(0.0, 0.0, 0.0, 1.0));
)";
			}
			code += "	MODELVIEW_NORMAL_MATRIX = mat3(MODELVIEW_MATRIX);\n";
		} break;
		case BILLBOARD_FIXED_Y: {
			// `MAIN_CAM_INV_VIEW_MATRIX` is inverse of the camera, even on shadow passes.
			// This ensures the billboard faces the camera when casting shadows.
			code += R"(
	// Billboard Mode: Y-Billboard
	MODELVIEW_MATRIX = VIEW_MATRIX * mat4(
			vec4(normalize(cross(vec3(0.0, 1.0, 0.0), MAIN_CAM_INV_VIEW_MATRIX[2].xyz)), 0.0),
			vec4(0.0, 1.0, 0.0, 0.0),
			vec4(normalize(cross(MAIN_CAM_INV_VIEW_MATRIX[0].xyz, vec3(0.0, 1.0, 0.0))), 0.0),
			MODEL_MATRIX[3]);
)";
			if (flags[FLAG_BILLBOARD_KEEP_SCALE]) {
				code += R"(
	// Billboard Keep Scale: Enabled
	MODELVIEW_MATRIX = MODELVIEW_MATRIX * mat4(
			vec4(length(MODEL_MATRIX[0].xyz), 0.0, 0.0, 0.0),
			vec4(0.0, length(MODEL_MATRIX[1].xyz), 0.0, 0.0),
			vec4(0.0, 0.0, length(MODEL_MATRIX[2].xyz), 0.0),
			vec4(0.0, 0.0, 0.0, 1.0));
)";
			}
			code += "	MODELVIEW_NORMAL_MATRIX = mat3(MODELVIEW_MATRIX);\n";
		} break;
		case BILLBOARD_PARTICLES: {
			// Make billboard and rotated by rotation.
			code += R"(
	// Billboard Mode: Particles
	mat4 mat_world = mat4(
			normalize(INV_VIEW_MATRIX[0]),
			normalize(INV_VIEW_MATRIX[1]),
			normalize(INV_VIEW_MATRIX[2]),
			MODEL_MATRIX[3]);
	mat_world = mat_world * mat4(
			vec4(cos(INSTANCE_CUSTOM.x), -sin(INSTANCE_CUSTOM.x), 0.0, 0.0),
			vec4(sin(INSTANCE_CUSTOM.x), cos(INSTANCE_CUSTOM.x), 0.0, 0.0),
			vec4(0.0, 0.0, 1.0, 0.0),
			vec4(0.0, 0.0, 0.0, 1.0));
)";
			// Set modelview.
			code += "	MODELVIEW_MATRIX = VIEW_MATRIX * mat_world;\n";
			if (flags[FLAG_BILLBOARD_KEEP_SCALE]) {
				code += R"(
	// Billboard Keep Scale: Enabled
	MODELVIEW_MATRIX = MODELVIEW_MATRIX * mat4(
			vec4(length(MODEL_MATRIX[0].xyz), 0.0, 0.0, 0.0),
			vec4(0.0, length(MODEL_MATRIX[1].xyz), 0.0, 0.0),
			vec4(0.0, 0.0, length(MODEL_MATRIX[2].xyz), 0.0),
			vec4(0.0, 0.0, 0.0, 1.0));
)";
			}
			// Set modelview normal and handle animation.
			code += R"(
	MODELVIEW_NORMAL_MATRIX = mat3(MODELVIEW_MATRIX);

	float h_frames = float(particles_anim_h_frames);
	float v_frames = float(particles_anim_v_frames);
	float particle_total_frames = float(particles_anim_h_frames * particles_anim_v_frames);
	float particle_frame = floor(INSTANCE_CUSTOM.z * float(particle_total_frames));
	if (!particles_anim_loop) {
		particle_frame = clamp(particle_frame, 0.0, particle_total_frames - 1.0);
	} else {
		particle_frame = mod(particle_frame, particle_total_frames);
	}
	UV /= vec2(h_frames, v_frames);
	UV += vec2(mod(particle_frame, h_frames) / h_frames, floor((particle_frame + 0.5) / h_frames) / v_frames);
)";
		} break;
		case BILLBOARD_MAX:
			break; // Internal value, skip.
	}

	if (flags[FLAG_FIXED_SIZE]) {
		code += R"(
	// Fixed Size: Enabled
	if (PROJECTION_MATRIX[3][3] != 0.0) {
		// Orthogonal matrix; try to do about the same with viewport size.
		float h = abs(1.0 / (2.0 * PROJECTION_MATRIX[1][1]));
		// Consistent with vertical FOV (Keep Height).
		float sc = (h * 2.0);
		MODELVIEW_MATRIX[0] *= sc;
		MODELVIEW_MATRIX[1] *= sc;
		MODELVIEW_MATRIX[2] *= sc;
	} else {
		// Scale by depth.
		float sc = -(MODELVIEW_MATRIX)[3].z;
		MODELVIEW_MATRIX[0] *= sc;
		MODELVIEW_MATRIX[1] *= sc;
		MODELVIEW_MATRIX[2] *= sc;
	}
)";
	}

	if (flags[FLAG_UV1_USE_TRIPLANAR] || flags[FLAG_UV2_USE_TRIPLANAR]) {
		// Generate tangent and binormal in world space.
		if (flags[FLAG_UV1_USE_WORLD_TRIPLANAR]) {
			code += R"(
	vec3 normal = MODEL_NORMAL_MATRIX * NORMAL;
)";
		} else {
			code += R"(
	vec3 normal = NORMAL;
)";
		}
		code += R"(
	TANGENT = vec3(0.0, 0.0, -1.0) * abs(normal.x);
	TANGENT += vec3(1.0, 0.0, 0.0) * abs(normal.y);
	TANGENT += vec3(1.0, 0.0, 0.0) * abs(normal.z);
)";
		if (flags[FLAG_UV1_USE_WORLD_TRIPLANAR]) {
			code += "	TANGENT = inverse(MODEL_NORMAL_MATRIX) * normalize(TANGENT);\n";
		} else {
			code += "	TANGENT = normalize(TANGENT);\n";
		}

		code += R"(
	BINORMAL = vec3(0.0, 1.0, 0.0) * abs(normal.x);
	BINORMAL += vec3(0.0, 0.0, -1.0) * abs(normal.y);
	BINORMAL += vec3(0.0, 1.0, 0.0) * abs(normal.z);
)";
		if (flags[FLAG_UV1_USE_WORLD_TRIPLANAR]) {
			code += "	BINORMAL = inverse(MODEL_NORMAL_MATRIX) * normalize(BINORMAL);\n";
		} else {
			code += "	BINORMAL = normalize(BINORMAL);\n";
		}
	}

	if (flags[FLAG_UV1_USE_TRIPLANAR]) {
		if (flags[FLAG_UV1_USE_WORLD_TRIPLANAR]) {
			code += R"(
	// UV1 Triplanar: Enabled (with World Triplanar)
	uv1_power_normal = pow(abs(normal), vec3(uv1_blend_sharpness));
	uv1_triplanar_pos = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz * uv1_scale + uv1_offset;
)";
		} else {
			code += R"(
	// UV1 Triplanar: Enabled
	uv1_power_normal = pow(abs(NORMAL), vec3(uv1_blend_sharpness));
	uv1_triplanar_pos = VERTEX * uv1_scale + uv1_offset;
)";
		}
		code += R"(	uv1_power_normal /= dot(uv1_power_normal, vec3(1.0));
	uv1_triplanar_pos *= vec3(1.0, -1.0, 1.0);
)";
	}

	if (flags[FLAG_UV2_USE_TRIPLANAR]) {
		if (flags[FLAG_UV2_USE_WORLD_TRIPLANAR]) {
			code += R"(
	// UV2 Triplanar: Enabled (with World Triplanar)
	uv2_power_normal = pow(abs(mat3(MODEL_MATRIX) * NORMAL), vec3(uv2_blend_sharpness));
	uv2_triplanar_pos = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz * uv2_scale + uv2_offset;
)";
		} else {
			code += R"(
	// UV2 Triplanar: Enabled
	uv2_power_normal = pow(abs(NORMAL), vec3(uv2_blend_sharpness));
	uv2_triplanar_pos = VERTEX * uv2_scale + uv2_offset;
)";
		}
		code += R"(	uv2_power_normal /= dot(uv2_power_normal, vec3(1.0));
	uv2_triplanar_pos *= vec3(1.0, -1.0, 1.0);
)";
	}

	if (grow_enabled) {
		code += R"(
	// Grow: Enabled
	VERTEX += NORMAL * grow;
)";
	}

	code += "}\n";

	if (flags[FLAG_ALBEDO_TEXTURE_MSDF] && !flags[FLAG_UV1_USE_TRIPLANAR]) {
		code += R"(
float msdf_median(float r, float g, float b, float a) {
	return min(max(min(r, g), min(max(r, g), b)), a);
}
)";
	}

	if (flags[FLAG_UV1_USE_TRIPLANAR] || flags[FLAG_UV2_USE_TRIPLANAR]) {
		code += R"(
vec4 triplanar_texture(sampler2D p_sampler, vec3 p_weights, vec3 p_triplanar_pos) {
	vec4 samp = vec4(0.0);
	samp += texture(p_sampler, p_triplanar_pos.xy) * p_weights.z;
	samp += texture(p_sampler, p_triplanar_pos.xz) * p_weights.y;
	samp += texture(p_sampler, p_triplanar_pos.zy * vec2(-1.0, 1.0)) * p_weights.x;
	return samp;
}
)";
	}

	// Generate fragment shader.
	code += R"(
void fragment() {)";

	if (!flags[FLAG_UV1_USE_TRIPLANAR]) {
		code += R"(
	vec2 base_uv = UV;
)";
	}

	if ((features[FEATURE_DETAIL] && detail_uv == DETAIL_UV_2) || (features[FEATURE_AMBIENT_OCCLUSION] && flags[FLAG_AO_ON_UV2]) || (features[FEATURE_EMISSION] && flags[FLAG_EMISSION_ON_UV2])) {
		// Don't add a newline if the UV assignment above is already performed,
		// so that UV1 and UV2 are closer to each other.
		if (flags[FLAG_UV1_USE_TRIPLANAR]) {
			code += "\n";
		}
		code += R"(	vec2 base_uv2 = UV2;
)";
	}

	if (features[FEATURE_HEIGHT_MAPPING] && flags[FLAG_UV1_USE_TRIPLANAR]) {
		// Display both resource name and albedo texture name.
		// Materials are often built-in to scenes, so displaying the resource name alone may not be meaningful.
		// On the other hand, albedo textures are almost always external to the scene.
		if (textures[TEXTURE_ALBEDO].is_valid()) {
			WARN_PRINT(vformat("%s (albedo %s): Height mapping is not supported on triplanar materials. Ignoring height mapping in favor of triplanar mapping.", GetPath(), textures[TEXTURE_ALBEDO]->GetPath()));
		} else if (!GetPath().is_empty()) {
			WARN_PRINT(vformat("%s: Height mapping is not supported on triplanar materials. Ignoring height mapping in favor of triplanar mapping.", GetPath()));
		} else {
			// Resource wasn't saved yet.
			WARN_PRINT("Height mapping is not supported on triplanar materials. Ignoring height mapping in favor of triplanar mapping.");
		}
	}

	// Heightmapping isn't supported at the same time as triplanar mapping.
	if (features[FEATURE_HEIGHT_MAPPING] && !flags[FLAG_UV1_USE_TRIPLANAR]) {
		// Binormal is negative due to mikktspace. Flipping it "unflips" it.
		code += R"(
	{
		// Height: Enabled
		vec3 view_dir = normalize(normalize(-VERTEX + EYE_OFFSET) * mat3(TANGENT * heightmap_flip.x, -BINORMAL * heightmap_flip.y, NORMAL));
)";

		if (deep_parallax) {
			// Multiply the heightmap scale by 0.01 to improve heightmap scale usability.
			code += R"(
		// Height Deep Parallax: Enabled
		float num_layers = mix(float(heightmap_max_layers), float(heightmap_min_layers), abs(dot(vec3(0.0, 0.0, 1.0), view_dir)));
		float layer_depth = 1.0 / num_layers;
		float current_layer_depth = 0.0;
		vec2 p = view_dir.xy * heightmap_scale * 0.01;
		vec2 delta = p / num_layers;
		vec2 ofs = base_uv;
)";
			if (flags[FLAG_INVERT_HEIGHTMAP]) {
				code += "		float depth = texture(texture_heightmap, ofs).r;\n";
			} else {
				code += "		float depth = 1.0 - texture(texture_heightmap, ofs).r;\n";
			}
			code += R"(
		float current_depth = 0.0;
		while (current_depth < depth) {
			ofs -= delta;
)";
			if (flags[FLAG_INVERT_HEIGHTMAP]) {
				code += "			depth = texture(texture_heightmap, ofs).r;\n";
			} else {
				code += "			depth = 1.0 - texture(texture_heightmap, ofs).r;\n";
			}
			code += R"(
			current_depth += layer_depth;
		}

		vec2 prev_ofs = ofs + delta;
		float after_depth = depth - current_depth;
)";
			if (flags[FLAG_INVERT_HEIGHTMAP]) {
				code += "		float before_depth = texture(texture_heightmap, prev_ofs).r - current_depth + layer_depth;\n";
			} else {
				code += "		float before_depth = (1.0 - texture(texture_heightmap, prev_ofs).r) - current_depth + layer_depth;\n";
			}
			code += R"(
		float weight = after_depth / (after_depth - before_depth);
		ofs = mix(ofs, prev_ofs, weight);
)";

		} else {
			if (flags[FLAG_INVERT_HEIGHTMAP]) {
				code += "		float depth = texture(texture_heightmap, base_uv).r;\n";
			} else {
				code += "		float depth = 1.0 - texture(texture_heightmap, base_uv).r;\n";
			}
			// Use offset limiting to improve the appearance of non-deep parallax.
			// This reduces the impression of depth, but avoids visible warping in the distance.
			// Multiply the heightmap scale by 0.01 to improve heightmap scale usability.
			code += "		vec2 ofs = base_uv - view_dir.xy * depth * heightmap_scale * 0.01;\n";
		}

		code += "		base_uv = ofs;\n";
		if (features[FEATURE_DETAIL] && detail_uv == DETAIL_UV_2) {
			code += "		base_uv2 -= ofs;\n";
		}

		code += "	}\n";
	}

	if (flags[FLAG_USE_POINT_SIZE]) {
		code += R"(
	// Use Point Size: Enabled
	vec4 albedo_tex = texture(texture_albedo, POINT_COORD);
)";
	} else {
		if (flags[FLAG_UV1_USE_TRIPLANAR]) {
			code += R"(
	vec4 albedo_tex = triplanar_texture(texture_albedo, uv1_power_normal, uv1_triplanar_pos);
)";
		} else {
			code += R"(
	vec4 albedo_tex = texture(texture_albedo, base_uv);
)";
		}
	}

	if (flags[FLAG_ALBEDO_TEXTURE_MSDF] && !flags[FLAG_UV1_USE_TRIPLANAR]) {
		code += R"(
	{
		// Albedo Texture MSDF: Enabled
		albedo_tex.rgb = mix(
				vec3(1.0 + 0.055) * pow(albedo_tex.rgb, vec3(1.0 / 2.4)) - vec3(0.055),
				vec3(12.92) * albedo_tex.rgb,
				lessThan(albedo_tex.rgb, vec3(0.0031308)));
		vec2 msdf_size = vec2(msdf_pixel_range) / vec2(textureSize(texture_albedo, 0));
)";
		if (flags[FLAG_USE_POINT_SIZE]) {
			code += "		vec2 dest_size = vec2(1.0) / fwidth(POINT_COORD);\n";
		} else {
			code += "		vec2 dest_size = vec2(1.0) / fwidth(base_uv);\n";
		}
		code += R"(
		float px_size = max(0.5 * dot(msdf_size, dest_size), 1.0);
		float d = msdf_median(albedo_tex.r, albedo_tex.g, albedo_tex.b, albedo_tex.a) - 0.5;
		if (msdf_outline_size > 0.0) {
			float cr = clamp(msdf_outline_size, 0.0, msdf_pixel_range / 2.0) / msdf_pixel_range;
			albedo_tex.a = clamp((d + cr) * px_size, 0.0, 1.0);
		} else {
			albedo_tex.a = clamp(d * px_size + 0.5, 0.0, 1.0);
		}
		albedo_tex.rgb = vec3(1.0);
	}
)";
	} else if (flags[FLAG_ALBEDO_TEXTURE_FORCE_SRGB]) {
		code += R"(
	// Albedo Texture Force sRGB: Enabled
	albedo_tex.rgb = mix(
			pow((albedo_tex.rgb + vec3(0.055)) * (1.0 / (1.0 + 0.055)), vec3(2.4)),
			albedo_tex.rgb.rgb * (1.0 / 12.92),
			lessThan(albedo_tex.rgb, vec3(0.04045)));
)";
	}

	if (flags[FLAG_ALBEDO_FROM_VERTEX_COLOR]) {
		code += R"(
	// Vertex Color Use as Albedo: Enabled
	albedo_tex *= COLOR;

)";
	}
	code += "	ALBEDO = albedo.rgb * albedo_tex.rgb;\n";

	if (!orm) {
		if (flags[FLAG_UV1_USE_TRIPLANAR]) {
			code += R"(
	float metallic_tex = dot(triplanar_texture(texture_metallic, uv1_power_normal, uv1_triplanar_pos), metallic_texture_channel);
)";
		} else {
			code += R"(
	float metallic_tex = dot(texture(texture_metallic, base_uv), metallic_texture_channel);
)";
		}

		code += R"(	METALLIC = metallic_tex * metallic;
	SPECULAR = specular;
)";
		switch (roughness_texture_channel) {
			case TEXTURE_CHANNEL_RED: {
				code += R"(
	vec4 roughness_texture_channel = vec4(1.0, 0.0, 0.0, 0.0);
)";
			} break;
			case TEXTURE_CHANNEL_GREEN: {
				code += R"(
	vec4 roughness_texture_channel = vec4(0.0, 1.0, 0.0, 0.0);
)";
			} break;
			case TEXTURE_CHANNEL_BLUE: {
				code += R"(
	vec4 roughness_texture_channel = vec4(0.0, 0.0, 1.0, 0.0);
)";
			} break;
			case TEXTURE_CHANNEL_ALPHA: {
				code += R"(
	vec4 roughness_texture_channel = vec4(0.0, 0.0, 0.0, 1.0);
)";
			} break;
			case TEXTURE_CHANNEL_GRAYSCALE: {
				code += R"(
	vec4 roughness_texture_channel = vec4(0.333333, 0.333333, 0.333333, 0.0);
)";
			} break;
			case TEXTURE_CHANNEL_MAX:
				break; // Internal value, skip.
		}

		if (flags[FLAG_UV1_USE_TRIPLANAR]) {
			code += "	float roughness_tex = dot(triplanar_texture(texture_roughness, uv1_power_normal, uv1_triplanar_pos), roughness_texture_channel);\n";
		} else {
			code += "	float roughness_tex = dot(texture(texture_roughness, base_uv), roughness_texture_channel);\n";
		}
		code += R"(	ROUGHNESS = roughness_tex * roughness;
)";
	} else {
		if (flags[FLAG_UV1_USE_TRIPLANAR]) {
			code += R"(
	vec4 orm_tex = triplanar_texture(texture_orm, uv1_power_normal, uv1_triplanar_pos);
)";
		} else {
			code += R"(
	vec4 orm_tex = texture(texture_orm, base_uv);
)";
		}

		code += R"(	ROUGHNESS = orm_tex.g;
 	METALLIC = orm_tex.b;
)";
	}

	if (features[FEATURE_NORMAL_MAPPING]) {
		code += R"(
	// Normal Map: Enabled
)";
		if (flags[FLAG_UV1_USE_TRIPLANAR]) {
			code += "	NORMAL_MAP = triplanar_texture(texture_normal, uv1_power_normal, uv1_triplanar_pos).rgb;\n";
		} else {
			code += "	NORMAL_MAP = texture(texture_normal, base_uv).rgb;\n";
		}
		code += "	NORMAL_MAP_DEPTH = normal_scale;\n";
	}

	if (features[FEATURE_EMISSION]) {
		code += R"(
	// Emission: Enabled
)";
		if (flags[FLAG_EMISSION_ON_UV2]) {
			if (flags[FLAG_UV2_USE_TRIPLANAR]) {
				code += "	vec3 emission_tex = triplanar_texture(texture_emission, uv2_power_normal, uv2_triplanar_pos).rgb;\n";
			} else {
				code += "	vec3 emission_tex = texture(texture_emission, base_uv2).rgb;\n";
			}
		} else {
			if (flags[FLAG_UV1_USE_TRIPLANAR]) {
				code += "	vec3 emission_tex = triplanar_texture(texture_emission, uv1_power_normal, uv1_triplanar_pos).rgb;\n";
			} else {
				code += "	vec3 emission_tex = texture(texture_emission, base_uv).rgb;\n";
			}
		}

		if (emission_op == EMISSION_OP_ADD) {
			code += R"(	// Emission Operator: Add
	EMISSION = (emission.rgb + emission_tex) * emission_energy;
)";
		} else {
			code += R"(	// Emission Operator: Multiply
	EMISSION = (emission.rgb * emission_tex) * emission_energy;
)";
		}
	}

	if (features[FEATURE_REFRACTION]) {
		if (features[FEATURE_NORMAL_MAPPING]) {
			code += R"(
	// Refraction: Enabled (with normal map texture)
	vec3 unpacked_normal = NORMAL_MAP;
	unpacked_normal.xy = unpacked_normal.xy * 2.0 - 1.0;
	unpacked_normal.z = sqrt(max(0.0, 1.0 - dot(unpacked_normal.xy, unpacked_normal.xy)));
	vec3 ref_normal = normalize(mix(
			NORMAL,
			TANGENT * unpacked_normal.x + BINORMAL * unpacked_normal.y + NORMAL * unpacked_normal.z,
			NORMAL_MAP_DEPTH));
)";
		} else {
			code += R"(
	// Refraction: Enabled
	vec3 ref_normal = NORMAL;
)";
		}
		if (flags[FLAG_UV1_USE_TRIPLANAR]) {
			code += "	vec2 ref_ofs = SCREEN_UV - ref_normal.xy * dot(triplanar_texture(texture_refraction, uv1_power_normal, uv1_triplanar_pos), refraction_texture_channel) * refraction;\n";
		} else {
			code += "	vec2 ref_ofs = SCREEN_UV - ref_normal.xy * dot(texture(texture_refraction, base_uv), refraction_texture_channel) * refraction;\n";
		}
		code += R"(
	float ref_amount = 1.0 - albedo.a * albedo_tex.a;
	EMISSION += textureLod(screen_texture, ref_ofs, ROUGHNESS * 8.0).rgb * ref_amount * EXPOSURE;
	ALBEDO *= 1.0 - ref_amount;
	// Force transparency on the material (required for refraction).
	ALPHA = 1.0;
)";

	} else if (transparency != TRANSPARENCY_DISABLED || flags[FLAG_USE_SHADOW_TO_OPACITY] || (distance_fade == DISTANCE_FADE_PIXEL_ALPHA) || proximity_fade_enabled) {
		code += "	ALPHA *= albedo.a * albedo_tex.a;\n";
	}
	if (transparency == TRANSPARENCY_ALPHA_HASH) {
		code += "	ALPHA_HASH_SCALE = alpha_hash_scale;\n";
	} else if (transparency == TRANSPARENCY_ALPHA_SCISSOR) {
		code += "	ALPHA_SCISSOR_THRESHOLD = alpha_scissor_threshold;\n";
	}
	if (alpha_antialiasing_mode != ALPHA_ANTIALIASING_OFF && (transparency == TRANSPARENCY_ALPHA_HASH || transparency == TRANSPARENCY_ALPHA_SCISSOR)) {
		code += "	ALPHA_ANTIALIASING_EDGE = alpha_antialiasing_edge;\n";
		code += "	ALPHA_TEXTURE_COORDINATE = UV * vec2(albedo_texture_size);\n";
	}

	if (proximity_fade_enabled) {
		code += R"(
	// Proximity Fade: Enabled
	float depth_tex = textureLod(depth_texture, SCREEN_UV, 0.0).r;
	vec4 world_pos = INV_PROJECTION_MATRIX * vec4(SCREEN_UV * 2.0 - 1.0, depth_tex, 1.0);
	world_pos.xyz /= world_pos.w;
	ALPHA *= clamp(1.0 - smoothstep(world_pos.z + proximity_fade_distance, world_pos.z, VERTEX.z), 0.0, 1.0);
)";
	}

	if (distance_fade != DISTANCE_FADE_DISABLED) {
		// Use the slightly more expensive circular fade (distance to the object) instead of linear
		// (Z distance), so that the fade is always the same regardless of the camera angle.
		if ((distance_fade == DISTANCE_FADE_OBJECT_DITHER || distance_fade == DISTANCE_FADE_PIXEL_DITHER)) {
			code += "\n	{";

			if (distance_fade == DISTANCE_FADE_OBJECT_DITHER) {
				code += R"(
		// Distance Fade: Object Dither
		float fade_distance = length((VIEW_MATRIX * MODEL_MATRIX[3]));
)";
			} else {
				code += R"(
		// Distance Fade: Pixel Dither
		float fade_distance = length(VERTEX);
)";
			}
			code += R"(
		// Use interleaved gradient noise, which is fast but still looks good.
		const vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189);
		float fade = clamp(smoothstep(distance_fade_min, distance_fade_max, fade_distance), 0.0, 1.0);
		// Use a hard cap to prevent a few stray pixels from remaining when past the fade-out distance.
		if (fade < 0.001 || fade < fract(magic.z * fract(dot(FRAGCOORD.xy, magic.xy)))) {
			discard;
		}
	}
)";
		} else {
			code += R"(
	// Distance Fade: Pixel Alpha
	ALPHA *= clamp(smoothstep(distance_fade_min, distance_fade_max, length(VERTEX)), 0.0, 1.0);
)";
		}
	}

	if (features[FEATURE_RIM]) {
		code += R"(
	// Rim: Enabled
)";
		if (flags[FLAG_UV1_USE_TRIPLANAR]) {
			code += "	vec2 rim_tex = triplanar_texture(texture_rim, uv1_power_normal, uv1_triplanar_pos).xy;\n";
		} else {
			code += "	vec2 rim_tex = texture(texture_rim, base_uv).xy;\n";
		}
		code += R"(	RIM = rim * rim_tex.x;
	RIM_TINT = rim_tint * rim_tex.y;
)";
	}

	if (features[FEATURE_CLEARCOAT]) {
		code += R"(
	// Clearcoat: Enabled
)";
		if (flags[FLAG_UV1_USE_TRIPLANAR]) {
			code += "	vec2 clearcoat_tex = triplanar_texture(texture_clearcoat, uv1_power_normal, uv1_triplanar_pos).xy;\n";
		} else {
			code += "	vec2 clearcoat_tex = texture(texture_clearcoat, base_uv).xy;\n";
		}
		code += R"(	CLEARCOAT = clearcoat * clearcoat_tex.x;
	CLEARCOAT_ROUGHNESS = clearcoat_roughness * clearcoat_tex.y;
)";
	}

	if (features[FEATURE_ANISOTROPY]) {
		code += R"(
	// Anisotropy: Enabled
)";
		if (flags[FLAG_UV1_USE_TRIPLANAR]) {
			code += "	vec3 anisotropy_tex = triplanar_texture(texture_flowmap, uv1_power_normal, uv1_triplanar_pos).rga;\n";
		} else {
			code += "	vec3 anisotropy_tex = texture(texture_flowmap, base_uv).rga;\n";
		}
		code += R"(	ANISOTROPY = anisotropy_ratio * anisotropy_tex.b;
	ANISOTROPY_FLOW = anisotropy_tex.rg * 2.0 - 1.0;
)";
	}

	if (features[FEATURE_AMBIENT_OCCLUSION]) {
		code += R"(
	// Ambient Occlusion: Enabled
)";
		if (!orm) {
			if (flags[FLAG_AO_ON_UV2]) {
				if (flags[FLAG_UV2_USE_TRIPLANAR]) {
					code += "	AO = dot(triplanar_texture(texture_ambient_occlusion, uv2_power_normal, uv2_triplanar_pos), ao_texture_channel);\n";
				} else {
					code += "	AO = dot(texture(texture_ambient_occlusion, base_uv2), ao_texture_channel);\n";
				}
			} else {
				if (flags[FLAG_UV1_USE_TRIPLANAR]) {
					code += "	AO = dot(triplanar_texture(texture_ambient_occlusion, uv1_power_normal, uv1_triplanar_pos), ao_texture_channel);\n";
				} else {
					code += "	AO = dot(texture(texture_ambient_occlusion, base_uv), ao_texture_channel);\n";
				}
			}
		} else {
			code += "	AO = orm_tex.r;\n";
		}

		code += "	AO_LIGHT_AFFECT = ao_light_affect;\n";
	}

	if (features[FEATURE_SUBSURFACE_SCATTERING]) {
		code += R"(
	// Subsurface Scattering: Enabled
)";
		if (flags[FLAG_UV1_USE_TRIPLANAR]) {
			code += "	float sss_tex = triplanar_texture(texture_subsurface_scattering, uv1_power_normal, uv1_triplanar_pos).r;\n";
		} else {
			code += "	float sss_tex = texture(texture_subsurface_scattering, base_uv).r;\n";
		}
		code += "	SSS_STRENGTH = subsurface_scattering_strength * sss_tex;\n";
	}

	if (features[FEATURE_SUBSURFACE_TRANSMITTANCE]) {
		code += R"(
	// Subsurface Scattering Transmittance: Enabled
)";
		if (flags[FLAG_UV1_USE_TRIPLANAR]) {
			code += "	vec4 trans_color_tex = triplanar_texture(texture_subsurface_transmittance, uv1_power_normal, uv1_triplanar_pos);\n";
		} else {
			code += "	vec4 trans_color_tex = texture(texture_subsurface_transmittance, base_uv);\n";
		}
		code += "	SSS_TRANSMITTANCE_COLOR = transmittance_color * trans_color_tex;\n";

		code += R"(	SSS_TRANSMITTANCE_DEPTH = transmittance_depth;
	SSS_TRANSMITTANCE_BOOST = transmittance_boost;
)";
	}

	if (features[FEATURE_BACKLIGHT]) {
		code += R"(
	// Backlight: Enabled
)";
		if (flags[FLAG_UV1_USE_TRIPLANAR]) {
			code += "	vec3 backlight_tex = triplanar_texture(texture_backlight, uv1_power_normal, uv1_triplanar_pos).rgb;\n";
		} else {
			code += "	vec3 backlight_tex = texture(texture_backlight, base_uv).rgb;\n";
		}
		code += "	BACKLIGHT = (backlight.rgb + backlight_tex);\n";
	}

	if (features[FEATURE_DETAIL]) {
		code += R"(
	// Detail: Enabled
)";
		const bool triplanar = (flags[FLAG_UV1_USE_TRIPLANAR] && detail_uv == DETAIL_UV_1) || (flags[FLAG_UV2_USE_TRIPLANAR] && detail_uv == DETAIL_UV_2);
		if (triplanar) {
			const String tp_uv = detail_uv == DETAIL_UV_1 ? "uv1" : "uv2";
			code += vformat(R"(	vec4 detail_tex = triplanar_texture(texture_detail_albedo, %s_power_normal, %s_triplanar_pos);
	vec4 detail_norm_tex = triplanar_texture(texture_detail_normal, %s_power_normal, %s_triplanar_pos);
)",
					tp_uv, tp_uv, tp_uv, tp_uv);
		} else {
			const String det_uv = detail_uv == DETAIL_UV_1 ? "base_uv" : "base_uv2";
			code += vformat(R"(	vec4 detail_tex = texture(texture_detail_albedo, %s);
	vec4 detail_norm_tex = texture(texture_detail_normal, %s);
)",
					det_uv, det_uv);
		}

		if (flags[FLAG_UV1_USE_TRIPLANAR]) {
			code += "	vec4 detail_mask_tex = triplanar_texture(texture_detail_mask, uv1_power_normal, uv1_triplanar_pos);\n";
		} else {
			code += "	vec4 detail_mask_tex = texture(texture_detail_mask, base_uv);\n";
		}

		switch (detail_blend_mode) {
			case BLEND_MODE_MIX: {
				code += R"(
	// Detail Blend Mode: Mix
	vec3 detail = mix(ALBEDO.rgb, detail_tex.rgb, detail_tex.a);
)";
			} break;
			case BLEND_MODE_ADD: {
				code += R"(
	// Detail Blend Mode: Add
	vec3 detail = mix(ALBEDO.rgb, ALBEDO.rgb + detail_tex.rgb, detail_tex.a);
)";
			} break;
			case BLEND_MODE_SUB: {
				code += R"(
	// Detail Blend Mode: Subtract
	vec3 detail = mix(ALBEDO.rgb, ALBEDO.rgb - detail_tex.rgb, detail_tex.a);
)";
			} break;
			case BLEND_MODE_MUL: {
				code += R"(
	// Detail Blend Mode: Multiply
	vec3 detail = mix(ALBEDO.rgb, ALBEDO.rgb * detail_tex.rgb, detail_tex.a);
)";
			} break;
			case BLEND_MODE_PREMULT_ALPHA: {
				// This is unlikely to ever be used for detail textures, and in order for it to function in the editor, another bit must be used in MaterialKey,
				// but there are only 5 bits left, so I'm going to leave this disabled unless it's actually requested.
				//code += "\tvec3 detail = (1.0-detail_tex.a)*ALBEDO.rgb+detail_tex.rgb;\n";
			} break;
			case BLEND_MODE_MAX:
				break; // Internal value, skip.
		}

		code += R"(	vec3 detail_norm = mix(NORMAL_MAP, detail_norm_tex.rgb, detail_tex.a);
	NORMAL_MAP = mix(NORMAL_MAP, detail_norm, detail_mask_tex.r);
	ALBEDO.rgb = mix(ALBEDO.rgb, detail, detail_mask_tex.r);
)";
	}

	code += "}\n";

	ShaderData shader_data;
	shader_data.shader = RS::get_singleton()->shader_create();
	shader_data.users = 1;

	RS::get_singleton()->shader_set_code(shader_data.shader, code);

	shader_map[mk] = shader_data;

	RS::get_singleton()->material_set_shader(_get_material(), shader_data.shader);
}


void BaseMaterial3D::flush_changes() {
	MutexLock lock(material_mutex);

	while (dirty_materials.first()) {
		dirty_materials.first()->self()->_update_shader();
		dirty_materials.first()->remove_from_list();
	}
}

void BaseMaterial3D::_queue_shader_change() {
	MutexLock lock(material_mutex);

	if (_is_initialized() && !element.in_list()) {
		dirty_materials.add(&element);
	}
}

void lain::BaseMaterial3D::_bind_methods() {}

void BaseMaterial3D::set_albedo(const Color& p_albedo) {
  albedo = p_albedo;

  RS::get_singleton()->material_set_param(_get_material(), shader_names->albedo, p_albedo);
}

Color BaseMaterial3D::get_albedo() const {
	return albedo;
}

void BaseMaterial3D::set_specular(float p_specular) {
	specular = p_specular;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->specular, p_specular);
}

float BaseMaterial3D::get_specular() const {
	return specular;
}

void BaseMaterial3D::set_roughness(float p_roughness) {
	roughness = p_roughness;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->roughness, p_roughness);
}

float BaseMaterial3D::get_roughness() const {
	return roughness;
}

void BaseMaterial3D::set_metallic(float p_metallic) {
	metallic = p_metallic;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->metallic, p_metallic);
}

float BaseMaterial3D::get_metallic() const {
	return metallic;
}

void BaseMaterial3D::set_emission(const Color &p_emission) {
	emission = p_emission;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->emission, p_emission);
}

Color BaseMaterial3D::get_emission() const {
	return emission;
}


void BaseMaterial3D::set_emission_energy_multiplier(float p_emission_energy_multiplier) {
	emission_energy_multiplier = p_emission_energy_multiplier;
	if (GLOBAL_GET("rendering/lights_and_shadows/use_physical_light_units")) {
		RS::get_singleton()->material_set_param(_get_material(), shader_names->emission_energy, p_emission_energy_multiplier * emission_intensity);
	} else {
		RS::get_singleton()->material_set_param(_get_material(), shader_names->emission_energy, p_emission_energy_multiplier);
	}
}

float BaseMaterial3D::get_emission_energy_multiplier() const {
	return emission_energy_multiplier;
}

void BaseMaterial3D::set_emission_intensity(float p_emission_intensity) {
	ERR_FAIL_COND_EDMSG(!GLOBAL_GET("rendering/lights_and_shadows/use_physical_light_units"), "Cannot set material emission intensity when Physical Light Units disabled.");
	emission_intensity = p_emission_intensity;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->emission_energy, emission_energy_multiplier * emission_intensity);
}


float BaseMaterial3D::get_emission_intensity() const {
	return emission_intensity;
}

void BaseMaterial3D::set_normal_scale(float p_normal_scale) {
	normal_scale = p_normal_scale;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->normal_scale, p_normal_scale);
}

float BaseMaterial3D::get_normal_scale() const {
	return normal_scale;
}

void BaseMaterial3D::set_rim(float p_rim) {
	rim = p_rim;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->rim, p_rim);
}

float BaseMaterial3D::get_rim() const {
	return rim;
}

void BaseMaterial3D::set_rim_tint(float p_rim_tint) {
	rim_tint = p_rim_tint;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->rim_tint, p_rim_tint);
}

float BaseMaterial3D::get_rim_tint() const {
	return rim_tint;
}

void BaseMaterial3D::set_ao_light_affect(float p_ao_light_affect) {
	ao_light_affect = p_ao_light_affect;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->ao_light_affect, p_ao_light_affect);
}

float BaseMaterial3D::get_ao_light_affect() const {
	return ao_light_affect;
}

void BaseMaterial3D::set_clearcoat(float p_clearcoat) {
	clearcoat = p_clearcoat;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->clearcoat, p_clearcoat);
}

float BaseMaterial3D::get_clearcoat() const {
	return clearcoat;
}

void BaseMaterial3D::set_clearcoat_roughness(float p_clearcoat_roughness) {
	clearcoat_roughness = p_clearcoat_roughness;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->clearcoat_roughness, p_clearcoat_roughness);
}

float BaseMaterial3D::get_clearcoat_roughness() const {
	return clearcoat_roughness;
}

void BaseMaterial3D::set_anisotropy(float p_anisotropy) {
	anisotropy = p_anisotropy;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->anisotropy, p_anisotropy);
}

float BaseMaterial3D::get_anisotropy() const {
	return anisotropy;
}

void BaseMaterial3D::set_heightmap_scale(float p_heightmap_scale) {
	heightmap_scale = p_heightmap_scale;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->heightmap_scale, p_heightmap_scale);
}

float BaseMaterial3D::get_heightmap_scale() const {
	return heightmap_scale;
}

void BaseMaterial3D::set_subsurface_scattering_strength(float p_subsurface_scattering_strength) {
	subsurface_scattering_strength = p_subsurface_scattering_strength;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->subsurface_scattering_strength, subsurface_scattering_strength);
}

float BaseMaterial3D::get_subsurface_scattering_strength() const {
	return subsurface_scattering_strength;
}

void BaseMaterial3D::set_transmittance_color(const Color &p_color) {
	transmittance_color = p_color;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->transmittance_color, p_color);
}

Color BaseMaterial3D::get_transmittance_color() const {
	return transmittance_color;
}

void BaseMaterial3D::set_transmittance_depth(float p_depth) {
	transmittance_depth = p_depth;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->transmittance_depth, p_depth);
}

float BaseMaterial3D::get_transmittance_depth() const {
	return transmittance_depth;
}

void BaseMaterial3D::set_transmittance_boost(float p_boost) {
	transmittance_boost = p_boost;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->transmittance_boost, p_boost);
}

float BaseMaterial3D::get_transmittance_boost() const {
	return transmittance_boost;
}

void BaseMaterial3D::set_backlight(const Color &p_backlight) {
	backlight = p_backlight;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->backlight, backlight);
}

Color BaseMaterial3D::get_backlight() const {
	return backlight;
}

void BaseMaterial3D::set_refraction(float p_refraction) {
	refraction = p_refraction;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->refraction, refraction);
}

float BaseMaterial3D::get_refraction() const {
	return refraction;
}

void BaseMaterial3D::set_detail_uv(DetailUV p_detail_uv) {
	if (detail_uv == p_detail_uv) {
		return;
	}

	detail_uv = p_detail_uv;
	_queue_shader_change();
}

BaseMaterial3D::DetailUV BaseMaterial3D::get_detail_uv() const {
	return detail_uv;
}

void BaseMaterial3D::set_blend_mode(BlendMode p_mode) {
	if (blend_mode == p_mode) {
		return;
	}

	blend_mode = p_mode;
	_queue_shader_change();
}

BaseMaterial3D::BlendMode BaseMaterial3D::get_blend_mode() const {
	return blend_mode;
}

void BaseMaterial3D::set_detail_blend_mode(BlendMode p_mode) {
	detail_blend_mode = p_mode;
	_queue_shader_change();
}

BaseMaterial3D::BlendMode BaseMaterial3D::get_detail_blend_mode() const {
	return detail_blend_mode;
}

void BaseMaterial3D::set_transparency(Transparency p_transparency) {
	if (transparency == p_transparency) {
		return;
	}

	transparency = p_transparency;
	_queue_shader_change();
	notify_property_list_changed();
}

BaseMaterial3D::Transparency BaseMaterial3D::get_transparency() const {
	return transparency;
}

void BaseMaterial3D::set_alpha_antialiasing(AlphaAntiAliasing p_alpha_aa) {
	if (alpha_antialiasing_mode == p_alpha_aa) {
		return;
	}

	alpha_antialiasing_mode = p_alpha_aa;
	_queue_shader_change();
	notify_property_list_changed();
}

BaseMaterial3D::AlphaAntiAliasing BaseMaterial3D::get_alpha_antialiasing() const {
	return alpha_antialiasing_mode;
}

void BaseMaterial3D::set_shading_mode(ShadingMode p_shading_mode) {
	if (shading_mode == p_shading_mode) {
		return;
	}

	shading_mode = p_shading_mode;
	_queue_shader_change();
	notify_property_list_changed();
}

BaseMaterial3D::ShadingMode BaseMaterial3D::get_shading_mode() const {
	return shading_mode;
}

void BaseMaterial3D::set_depth_draw_mode(DepthDrawMode p_mode) {
	if (depth_draw_mode == p_mode) {
		return;
	}

	depth_draw_mode = p_mode;
	_queue_shader_change();
}

BaseMaterial3D::DepthDrawMode BaseMaterial3D::get_depth_draw_mode() const {
	return depth_draw_mode;
}

void BaseMaterial3D::set_cull_mode(CullMode p_mode) {
	if (cull_mode == p_mode) {
		return;
	}

	cull_mode = p_mode;
	_queue_shader_change();
}

BaseMaterial3D::CullMode BaseMaterial3D::get_cull_mode() const {
	return cull_mode;
}

void BaseMaterial3D::set_diffuse_mode(DiffuseMode p_mode) {
	if (diffuse_mode == p_mode) {
		return;
	}

	diffuse_mode = p_mode;
	_queue_shader_change();
}

BaseMaterial3D::DiffuseMode BaseMaterial3D::get_diffuse_mode() const {
	return diffuse_mode;
}

void BaseMaterial3D::set_specular_mode(SpecularMode p_mode) {
	if (specular_mode == p_mode) {
		return;
	}

	specular_mode = p_mode;
	_queue_shader_change();
}

BaseMaterial3D::SpecularMode BaseMaterial3D::get_specular_mode() const {
	return specular_mode;
}

void BaseMaterial3D::set_flag(Flags p_flag, bool p_enabled) {
	ERR_FAIL_INDEX(p_flag, FLAG_MAX);

	if (flags[p_flag] == p_enabled) {
		return;
	}

	flags[p_flag] = p_enabled;

	if (
			p_flag == FLAG_USE_SHADOW_TO_OPACITY ||
			p_flag == FLAG_USE_TEXTURE_REPEAT ||
			p_flag == FLAG_SUBSURFACE_MODE_SKIN ||
			p_flag == FLAG_USE_POINT_SIZE ||
			p_flag == FLAG_UV1_USE_TRIPLANAR ||
			p_flag == FLAG_UV2_USE_TRIPLANAR) {
		notify_property_list_changed();
	}

	if (p_flag == FLAG_PARTICLE_TRAILS_MODE) {
		// update_configuration_warning();
	}

	_queue_shader_change();
}

bool BaseMaterial3D::get_flag(Flags p_flag) const {
	ERR_FAIL_INDEX_V(p_flag, FLAG_MAX, false);
	return flags[p_flag];
}

void BaseMaterial3D::set_feature(Feature p_feature, bool p_enabled) {
	ERR_FAIL_INDEX(p_feature, FEATURE_MAX);
	if (features[p_feature] == p_enabled) {
		return;
	}

	features[p_feature] = p_enabled;
	notify_property_list_changed();
	_queue_shader_change();
}


Ref<Texture2D> BaseMaterial3D::get_texture(TextureParam p_param) const {
	ERR_FAIL_INDEX_V(p_param, TEXTURE_MAX, Ref<Texture2D>());
	return textures[p_param];
}

Ref<Texture2D> BaseMaterial3D::get_texture_by_name(const StringName &p_name) const {
	for (int i = 0; i < (int)BaseMaterial3D::TEXTURE_MAX; i++) {
		TextureParam param = TextureParam(i);
		if (p_name == shader_names->texture_names[param]) {
			return textures[param];
		}
	}
	return Ref<Texture2D>();
}

void BaseMaterial3D::set_texture_filter(TextureFilter p_filter) {
	texture_filter = p_filter;
	_queue_shader_change();
}


void BaseMaterial3D::set_point_size(float p_point_size) {
	point_size = p_point_size;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->point_size, p_point_size);
}

float BaseMaterial3D::get_point_size() const {
	return point_size;
}

void BaseMaterial3D::set_uv1_scale(const Vector3 &p_scale) {
	uv1_scale = p_scale;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->uv1_scale, p_scale);
}

Vector3 BaseMaterial3D::get_uv1_scale() const {
	return uv1_scale;
}

void BaseMaterial3D::set_uv1_offset(const Vector3 &p_offset) {
	uv1_offset = p_offset;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->uv1_offset, p_offset);
}

Vector3 BaseMaterial3D::get_uv1_offset() const {
	return uv1_offset;
}

void BaseMaterial3D::set_uv1_triplanar_blend_sharpness(float p_sharpness) {
	// Negative values or values higher than 150 can result in NaNs, leading to broken rendering.
	uv1_triplanar_sharpness = CLAMP(p_sharpness, 0.0, 150.0);
	RS::get_singleton()->material_set_param(_get_material(), shader_names->uv1_blend_sharpness, uv1_triplanar_sharpness);
}

float BaseMaterial3D::get_uv1_triplanar_blend_sharpness() const {
	return uv1_triplanar_sharpness;
}

void BaseMaterial3D::set_uv2_scale(const Vector3 &p_scale) {
	uv2_scale = p_scale;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->uv2_scale, p_scale);
}

Vector3 BaseMaterial3D::get_uv2_scale() const {
	return uv2_scale;
}

void BaseMaterial3D::set_uv2_offset(const Vector3 &p_offset) {
	uv2_offset = p_offset;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->uv2_offset, p_offset);
}

Vector3 BaseMaterial3D::get_uv2_offset() const {
	return uv2_offset;
}

void BaseMaterial3D::set_uv2_triplanar_blend_sharpness(float p_sharpness) {
	// Negative values or values higher than 150 can result in NaNs, leading to broken rendering.
	uv2_triplanar_sharpness = CLAMP(p_sharpness, 0.0, 150.0);
	RS::get_singleton()->material_set_param(_get_material(), shader_names->uv2_blend_sharpness, uv2_triplanar_sharpness);
}

float BaseMaterial3D::get_uv2_triplanar_blend_sharpness() const {
	return uv2_triplanar_sharpness;
}

void BaseMaterial3D::set_billboard_mode(BillboardMode p_mode) {
	billboard_mode = p_mode;
	_queue_shader_change();
	notify_property_list_changed();
}

BaseMaterial3D::BillboardMode BaseMaterial3D::get_billboard_mode() const {
	return billboard_mode;
}

void BaseMaterial3D::set_particles_anim_h_frames(int p_frames) {
	particles_anim_h_frames = p_frames;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->particles_anim_h_frames, p_frames);
}

int BaseMaterial3D::get_particles_anim_h_frames() const {
	return particles_anim_h_frames;
}

void BaseMaterial3D::set_particles_anim_v_frames(int p_frames) {
	particles_anim_v_frames = p_frames;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->particles_anim_v_frames, p_frames);
}

int BaseMaterial3D::get_particles_anim_v_frames() const {
	return particles_anim_v_frames;
}

void BaseMaterial3D::set_particles_anim_loop(bool p_loop) {
	particles_anim_loop = p_loop;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->particles_anim_loop, particles_anim_loop);
}

bool BaseMaterial3D::get_particles_anim_loop() const {
	return particles_anim_loop;
}

void BaseMaterial3D::set_heightmap_deep_parallax(bool p_enable) {
	deep_parallax = p_enable;
	_queue_shader_change();
	notify_property_list_changed();
}

bool BaseMaterial3D::is_heightmap_deep_parallax_enabled() const {
	return deep_parallax;
}

void BaseMaterial3D::set_heightmap_deep_parallax_min_layers(int p_layer) {
	deep_parallax_min_layers = p_layer;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->heightmap_min_layers, p_layer);
}

int BaseMaterial3D::get_heightmap_deep_parallax_min_layers() const {
	return deep_parallax_min_layers;
}

void BaseMaterial3D::set_heightmap_deep_parallax_max_layers(int p_layer) {
	deep_parallax_max_layers = p_layer;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->heightmap_max_layers, p_layer);
}

int BaseMaterial3D::get_heightmap_deep_parallax_max_layers() const {
	return deep_parallax_max_layers;
}

void BaseMaterial3D::set_heightmap_deep_parallax_flip_tangent(bool p_flip) {
	heightmap_parallax_flip_tangent = p_flip;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->heightmap_flip, Vector2(heightmap_parallax_flip_tangent ? -1 : 1, heightmap_parallax_flip_binormal ? -1 : 1));
}

bool BaseMaterial3D::get_heightmap_deep_parallax_flip_tangent() const {
	return heightmap_parallax_flip_tangent;
}

void BaseMaterial3D::set_heightmap_deep_parallax_flip_binormal(bool p_flip) {
	heightmap_parallax_flip_binormal = p_flip;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->heightmap_flip, Vector2(heightmap_parallax_flip_tangent ? -1 : 1, heightmap_parallax_flip_binormal ? -1 : 1));
}

bool BaseMaterial3D::get_heightmap_deep_parallax_flip_binormal() const {
	return heightmap_parallax_flip_binormal;
}

void BaseMaterial3D::set_grow_enabled(bool p_enable) {
	grow_enabled = p_enable;
	_queue_shader_change();
	notify_property_list_changed();
}

bool BaseMaterial3D::is_grow_enabled() const {
	return grow_enabled;
}

void BaseMaterial3D::set_alpha_scissor_threshold(float p_threshold) {
	alpha_scissor_threshold = p_threshold;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->alpha_scissor_threshold, p_threshold);
}

float BaseMaterial3D::get_alpha_scissor_threshold() const {
	return alpha_scissor_threshold;
}

void BaseMaterial3D::set_alpha_hash_scale(float p_scale) {
	alpha_hash_scale = p_scale;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->alpha_hash_scale, p_scale);
}

float BaseMaterial3D::get_alpha_hash_scale() const {
	return alpha_hash_scale;
}

void BaseMaterial3D::set_alpha_antialiasing_edge(float p_edge) {
	alpha_antialiasing_edge = p_edge;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->alpha_antialiasing_edge, p_edge);
}

float BaseMaterial3D::get_alpha_antialiasing_edge() const {
	return alpha_antialiasing_edge;
}

void BaseMaterial3D::set_grow(float p_grow) {
	grow = p_grow;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->grow, p_grow);
}

float BaseMaterial3D::get_grow() const {
	return grow;
}

static Plane _get_texture_mask(BaseMaterial3D::TextureChannel p_channel) {
	static const Plane masks[5] = {
		Plane(1, 0, 0, 0),
		Plane(0, 1, 0, 0),
		Plane(0, 0, 1, 0),
		Plane(0, 0, 0, 1),
		Plane(0.3333333, 0.3333333, 0.3333333, 0),
	};

	return masks[p_channel];
}

void BaseMaterial3D::set_metallic_texture_channel(TextureChannel p_channel) {
	ERR_FAIL_INDEX(p_channel, 5);
	metallic_texture_channel = p_channel;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->metallic_texture_channel, _get_texture_mask(p_channel));
}

BaseMaterial3D::TextureChannel BaseMaterial3D::get_metallic_texture_channel() const {
	return metallic_texture_channel;
}

void BaseMaterial3D::set_roughness_texture_channel(TextureChannel p_channel) {
	ERR_FAIL_INDEX(p_channel, 5);
	roughness_texture_channel = p_channel;
	_queue_shader_change();
}

BaseMaterial3D::TextureChannel BaseMaterial3D::get_roughness_texture_channel() const {
	return roughness_texture_channel;
}

void BaseMaterial3D::set_ao_texture_channel(TextureChannel p_channel) {
	ERR_FAIL_INDEX(p_channel, 5);
	ao_texture_channel = p_channel;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->ao_texture_channel, _get_texture_mask(p_channel));
}

BaseMaterial3D::TextureChannel BaseMaterial3D::get_ao_texture_channel() const {
	return ao_texture_channel;
}

void BaseMaterial3D::set_refraction_texture_channel(TextureChannel p_channel) {
	ERR_FAIL_INDEX(p_channel, 5);
	refraction_texture_channel = p_channel;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->refraction_texture_channel, _get_texture_mask(p_channel));
}

BaseMaterial3D::TextureChannel BaseMaterial3D::get_refraction_texture_channel() const {
	return refraction_texture_channel;
}


void BaseMaterial3D::set_on_top_of_alpha() {
	set_transparency(TRANSPARENCY_DISABLED);
	set_render_priority(RENDER_PRIORITY_MAX);
	set_flag(FLAG_DISABLE_DEPTH_TEST, true);
}

void BaseMaterial3D::set_proximity_fade_enabled(bool p_enable) {
	proximity_fade_enabled = p_enable;
	_queue_shader_change();
	notify_property_list_changed();
}

bool BaseMaterial3D::is_proximity_fade_enabled() const {
	return proximity_fade_enabled;
}

void BaseMaterial3D::set_proximity_fade_distance(float p_distance) {
	proximity_fade_distance = MAX(p_distance, 0.01);
	RS::get_singleton()->material_set_param(_get_material(), shader_names->proximity_fade_distance, proximity_fade_distance);
}

float BaseMaterial3D::get_proximity_fade_distance() const {
	return proximity_fade_distance;
}

void BaseMaterial3D::set_msdf_pixel_range(float p_range) {
	msdf_pixel_range = p_range;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->msdf_pixel_range, p_range);
}

float BaseMaterial3D::get_msdf_pixel_range() const {
	return msdf_pixel_range;
}

void BaseMaterial3D::set_msdf_outline_size(float p_size) {
	msdf_outline_size = p_size;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->msdf_outline_size, p_size);
}

float BaseMaterial3D::get_msdf_outline_size() const {
	return msdf_outline_size;
}

void BaseMaterial3D::set_distance_fade(DistanceFadeMode p_mode) {
	distance_fade = p_mode;
	_queue_shader_change();
	notify_property_list_changed();
}

BaseMaterial3D::DistanceFadeMode BaseMaterial3D::get_distance_fade() const {
	return distance_fade;
}

void BaseMaterial3D::set_distance_fade_max_distance(float p_distance) {
	distance_fade_max_distance = p_distance;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->distance_fade_max, distance_fade_max_distance);
}

float BaseMaterial3D::get_distance_fade_max_distance() const {
	return distance_fade_max_distance;
}

void BaseMaterial3D::set_distance_fade_min_distance(float p_distance) {
	distance_fade_min_distance = p_distance;
	RS::get_singleton()->material_set_param(_get_material(), shader_names->distance_fade_min, distance_fade_min_distance);
}

float BaseMaterial3D::get_distance_fade_min_distance() const {
	return distance_fade_min_distance;
}

void BaseMaterial3D::set_emission_operator(EmissionOperator p_op) {
	if (emission_op == p_op) {
		return;
	}
	emission_op = p_op;
	_queue_shader_change();
}

BaseMaterial3D::EmissionOperator BaseMaterial3D::get_emission_operator() const {
	return emission_op;
}

RID BaseMaterial3D::get_shader_rid() const {
	MutexLock lock(material_mutex);
	if (element.in_list()) {
		((BaseMaterial3D *)this)->_update_shader();
	}
	ERR_FAIL_COND_V(!shader_map.has(current_key), RID());
	return shader_map[current_key].shader;
}

Shader::Mode BaseMaterial3D::get_shader_mode() const {
	return Shader::MODE_SPATIAL;
}

BaseMaterial3D::BaseMaterial3D(bool p_orm) :
		element(this) {
	orm = p_orm;
	// Initialize to the same values as the shader
	set_albedo(Color(1.0, 1.0, 1.0, 1.0));
	set_specular(0.5);
	set_roughness(1.0);
	set_metallic(0.0);
	set_emission(Color(0, 0, 0));
	set_emission_energy_multiplier(1.0);
	set_normal_scale(1);
	set_rim(1.0);
	set_rim_tint(0.5);
	set_clearcoat(1);
	set_clearcoat_roughness(0.5);
	set_anisotropy(0);
	set_heightmap_scale(5.0);
	set_subsurface_scattering_strength(0);
	set_backlight(Color(0, 0, 0));
	set_transmittance_color(Color(1, 1, 1, 1));
	set_transmittance_depth(0.1);
	set_transmittance_boost(0.0);
	set_refraction(0.05);
	set_point_size(1);
	set_uv1_offset(Vector3(0, 0, 0));
	set_uv1_scale(Vector3(1, 1, 1));
	set_uv1_triplanar_blend_sharpness(1);
	set_uv2_offset(Vector3(0, 0, 0));
	set_uv2_scale(Vector3(1, 1, 1));
	set_uv2_triplanar_blend_sharpness(1);
	set_billboard_mode(BILLBOARD_DISABLED);
	set_particles_anim_h_frames(1);
	set_particles_anim_v_frames(1);
	set_particles_anim_loop(false);

	set_transparency(TRANSPARENCY_DISABLED);
	set_alpha_antialiasing(ALPHA_ANTIALIASING_OFF);
	// Alpha scissor threshold of 0.5 matches the glTF specification and Label3D default.
	// <https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html#_material_alphacutoff>
	set_alpha_scissor_threshold(0.5);
	set_alpha_hash_scale(1.0);
	set_alpha_antialiasing_edge(0.3);

	set_proximity_fade_distance(1);
	set_distance_fade_min_distance(0);
	set_distance_fade_max_distance(10);

	set_ao_light_affect(0.0);

	set_metallic_texture_channel(TEXTURE_CHANNEL_RED);
	set_roughness_texture_channel(TEXTURE_CHANNEL_RED);
	set_ao_texture_channel(TEXTURE_CHANNEL_RED);
	set_refraction_texture_channel(TEXTURE_CHANNEL_RED);

	set_grow(0.0);

	set_msdf_pixel_range(4.0);
	set_msdf_outline_size(0.0);

	set_heightmap_deep_parallax_min_layers(8);
	set_heightmap_deep_parallax_max_layers(32);
	set_heightmap_deep_parallax_flip_tangent(false); //also sets binormal

	flags[FLAG_ALBEDO_TEXTURE_MSDF] = false;
	flags[FLAG_USE_TEXTURE_REPEAT] = true;

	current_key.invalid_key = 1;

	// _mark_initialized(callable_mp(this, &BaseMaterial3D::_queue_shader_change), callable_mp(this, &BaseMaterial3D::_update_shader));
  init_state = INIT_STATE_READY;
  _queue_shader_change();
}


BaseMaterial3D::~BaseMaterial3D() {
	ERR_FAIL_NULL(RS::get_singleton());
	MutexLock lock(material_mutex);

	if (shader_map.has(current_key)) {
		shader_map[current_key].users--;
		if (shader_map[current_key].users == 0) {
			//deallocate shader, as it's no longer in use
			RS::get_singleton()->free(shader_map[current_key].shader);
			shader_map.erase(current_key);
		}

		RS::get_singleton()->material_set_shader(_get_material(), RID());
	}
}

void lain::ShaderMaterial::_get_property_list(List<PropertyInfo>* p_list) const {}
