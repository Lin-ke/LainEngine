#include "resource_importer_obj.h"
#include "surface_tool.h"
using namespace lain;

static Error _parse_material_library(const String &p_path, HashMap<String, Ref<StandardMaterial3D>> &material_map, List<String> *r_missing_deps) {
	Ref<FileAccess> f = FileAccess::open(p_path, FileAccess::READ);
	ERR_FAIL_COND_V_MSG(f.is_null(), ERR_CANT_OPEN, vformat("Couldn't open MTL file '%s', it may not exist or not be readable.", p_path));

	Ref<StandardMaterial3D> current;
	String current_name;
	String base_path = p_path.get_base_dir();
	while (true) {
		String l = f->get_line().strip_edges();

		if (l.begins_with("newmtl ")) {
			//vertex

			current_name = l.replace("newmtl", "").strip_edges();
			current.instantiate();
			current->SetName(current_name);
			material_map[current_name] = current;
		} else if (l.begins_with("Ka ")) {
			//uv
			WARN_PRINT("OBJ: Ambient light for material '" + current_name + "' is ignored in PBR");

		} else if (l.begins_with("Kd ")) {
			//normal
			ERR_FAIL_COND_V(current.is_null(), ERR_FILE_CORRUPT);
			Vector<String> v = l.split(" ", false);
			ERR_FAIL_COND_V(v.size() < 4, ERR_INVALID_DATA);
			Color c = current->get_albedo();
			c.r = v[1].to_float();
			c.g = v[2].to_float();
			c.b = v[3].to_float();
			current->set_albedo(c);
		} else if (l.begins_with("Ks ")) {
			//normal
			ERR_FAIL_COND_V(current.is_null(), ERR_FILE_CORRUPT);
			Vector<String> v = l.split(" ", false);
			ERR_FAIL_COND_V(v.size() < 4, ERR_INVALID_DATA);
			float r = v[1].to_float();
			float g = v[2].to_float();
			float b = v[3].to_float();
			float metalness = MAX(r, MAX(g, b));
			current->set_metallic(metalness);
		} else if (l.begins_with("Ns ")) {
			//normal
			ERR_FAIL_COND_V(current.is_null(), ERR_FILE_CORRUPT);
			Vector<String> v = l.split(" ", false);
			ERR_FAIL_COND_V(v.size() != 2, ERR_INVALID_DATA);
			float s = v[1].to_float();
			current->set_metallic((1000.0 - s) / 1000.0);
		} else if (l.begins_with("d ")) {
			//normal
			ERR_FAIL_COND_V(current.is_null(), ERR_FILE_CORRUPT);
			Vector<String> v = l.split(" ", false);
			ERR_FAIL_COND_V(v.size() != 2, ERR_INVALID_DATA);
			float d = v[1].to_float();
			Color c = current->get_albedo();
			c.a = d;
			current->set_albedo(c);
			if (c.a < 0.99) {
				current->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA);
			}
		} else if (l.begins_with("Tr ")) {
			//normal
			ERR_FAIL_COND_V(current.is_null(), ERR_FILE_CORRUPT);
			Vector<String> v = l.split(" ", false);
			ERR_FAIL_COND_V(v.size() != 2, ERR_INVALID_DATA);
			float d = v[1].to_float();
			Color c = current->get_albedo();
			c.a = 1.0 - d;
			current->set_albedo(c);
			if (c.a < 0.99) {
				current->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA);
			}

		} else if (l.begins_with("map_Ka ")) {
			//uv
			WARN_PRINT("OBJ: Ambient light texture for material '" + current_name + "' is ignored in PBR");

		} else if (l.begins_with("map_Kd ")) {
			//normal
			ERR_FAIL_COND_V(current.is_null(), ERR_FILE_CORRUPT);

			String p = l.replace("map_Kd", "").replace("\\", "/").strip_edges();
			String path;
			if (p.is_absolute_path()) {
				path = p;
			} else {
				path = base_path.path_join(p);
			}

			Ref<Texture2D> texture = ResourceLoader::load(path);

			if (texture.is_valid()) {
				current->set_texture(StandardMaterial3D::TEXTURE_ALBEDO, texture);
			} else if (r_missing_deps) {
				r_missing_deps->push_back(path);
			}

		} else if (l.begins_with("map_Ks ")) {
			//normal
			ERR_FAIL_COND_V(current.is_null(), ERR_FILE_CORRUPT);

			String p = l.replace("map_Ks", "").replace("\\", "/").strip_edges();
			String path;
			if (p.is_absolute_path()) {
				path = p;
			} else {
				path = base_path.path_join(p);
			}

			Ref<Texture2D> texture = ResourceLoader::load(path);

			if (texture.is_valid()) {
				current->set_texture(StandardMaterial3D::TEXTURE_METALLIC, texture);
			} else if (r_missing_deps) {
				r_missing_deps->push_back(path);
			}

		} else if (l.begins_with("map_Ns ")) {
			//normal
			ERR_FAIL_COND_V(current.is_null(), ERR_FILE_CORRUPT);

			String p = l.replace("map_Ns", "").replace("\\", "/").strip_edges();
			String path;
			if (p.is_absolute_path()) {
				path = p;
			} else {
				path = base_path.path_join(p);
			}

			Ref<Texture2D> texture = ResourceLoader::load(path);

			if (texture.is_valid()) {
				current->set_texture(StandardMaterial3D::TEXTURE_ROUGHNESS, texture);
			} else if (r_missing_deps) {
				r_missing_deps->push_back(path);
			}
		} else if (l.begins_with("map_bump ")) {
			//normal
			ERR_FAIL_COND_V(current.is_null(), ERR_FILE_CORRUPT);

			String p = l.replace("map_bump", "").replace("\\", "/").strip_edges();
			String path = base_path.path_join(p);

			Ref<Texture2D> texture = ResourceLoader::load(path);

			if (texture.is_valid()) {
				current->set_feature(StandardMaterial3D::FEATURE_NORMAL_MAPPING, true);
				current->set_texture(StandardMaterial3D::TEXTURE_NORMAL, texture);
			} else if (r_missing_deps) {
				r_missing_deps->push_back(path);
			}
		} else if (f->eof_reached()) {
			break;
		}
	}

	return OK;
}


static Error _parse_obj(const String &p_path, List<Ref<ImporterMesh>> &r_meshes, bool p_single_mesh, bool p_generate_tangents, bool p_optimize, Vector3 p_scale_mesh, Vector3 p_offset_mesh, bool p_disable_compression, List<String> *r_missing_deps) {
	Ref<FileAccess> f = FileAccess::open(p_path, FileAccess::READ);
	ERR_FAIL_COND_V_MSG(f.is_null(), ERR_CANT_OPEN, vformat("Couldn't open OBJ file '%s', it may not exist or not be readable.", p_path));

	// Avoid trying to load/interpret potential build artifacts from Visual Studio (e.g. when compiling native plugins inside the project tree)
	// This should only match, if it's indeed a COFF file header
	// https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#machine-types
	const int first_bytes = f->get_16();
	static const Vector<int> coff_header_machines{
		0x0, // IMAGE_FILE_MACHINE_UNKNOWN
		0x8664, // IMAGE_FILE_MACHINE_AMD64
		0x1c0, // IMAGE_FILE_MACHINE_ARM
		0x14c, // IMAGE_FILE_MACHINE_I386
		0x200, // IMAGE_FILE_MACHINE_IA64
	};
	ERR_FAIL_COND_V_MSG(coff_header_machines.has(first_bytes), ERR_FILE_CORRUPT, vformat("Couldn't read OBJ file '%s', it seems to be binary, corrupted, or empty.", p_path));
	f->seek(0);

	Ref<ImporterMesh> mesh;
	mesh.instantiate();

	bool generate_tangents = p_generate_tangents;
	Vector3 scale_mesh = p_scale_mesh;
	Vector3 offset_mesh = p_offset_mesh;

	Vector<Vector3> vertices;
	Vector<Vector3> normals;
	Vector<Vector2> uvs;
	Vector<Color> colors;
	const String default_name = "Mesh";
	String name = default_name;

	HashMap<String, HashMap<String, Ref<StandardMaterial3D>>> material_map;

	Ref<SurfaceTool> surf_tool = memnew(SurfaceTool);
	surf_tool->begin(Mesh::PRIMITIVE_TRIANGLES);

	String current_material_library;
	String current_material;
	String current_group;
	uint32_t smooth_group = 0;
	bool smoothing = true;
	const uint32_t no_smoothing_smooth_group = (uint32_t)-1;

	while (true) {
		String l = f->get_line().strip_edges();
		while (l.length() && l[l.length() - 1] == '\\') {
			String add = f->get_line().strip_edges();
			l += add;
			if (add.is_empty()) {
				break;
			}
		}

		if (l.begins_with("v ")) {
			//vertex
			Vector<String> v = l.split(" ", false);
			ERR_FAIL_COND_V(v.size() < 4, ERR_FILE_CORRUPT);
			Vector3 vtx;
			vtx.x = v[1].to_float() * scale_mesh.x + offset_mesh.x;
			vtx.y = v[2].to_float() * scale_mesh.y + offset_mesh.y;
			vtx.z = v[3].to_float() * scale_mesh.z + offset_mesh.z;
			vertices.push_back(vtx);
			//vertex color
			if (v.size() >= 7) {
				while (colors.size() < vertices.size() - 1) {
					colors.push_back(Color(1.0, 1.0, 1.0));
				}
				Color c;
				c.r = v[4].to_float();
				c.g = v[5].to_float();
				c.b = v[6].to_float();
				colors.push_back(c);
			} else if (!colors.is_empty()) {
				colors.push_back(Color(1.0, 1.0, 1.0));
			}
		} else if (l.begins_with("vt ")) {
			//uv
			Vector<String> v = l.split(" ", false);
			ERR_FAIL_COND_V(v.size() < 3, ERR_FILE_CORRUPT);
			Vector2 uv;
			uv.x = v[1].to_float();
			uv.y = 1.0 - v[2].to_float();
			uvs.push_back(uv);
		} else if (l.begins_with("vn ")) {
			//normal
			Vector<String> v = l.split(" ", false);
			ERR_FAIL_COND_V(v.size() < 4, ERR_FILE_CORRUPT);
			Vector3 nrm;
			nrm.x = v[1].to_float();
			nrm.y = v[2].to_float();
			nrm.z = v[3].to_float();
			normals.push_back(nrm);
		} else if (l.begins_with("f ")) {
			//vertex

			Vector<String> v = l.split(" ", false);
			ERR_FAIL_COND_V(v.size() < 4, ERR_FILE_CORRUPT);

			//not very fast, could be sped up

			Vector<String> face[3];
			face[0] = v[1].split("/");
			face[1] = v[2].split("/");
			ERR_FAIL_COND_V(face[0].is_empty(), ERR_FILE_CORRUPT);

			ERR_FAIL_COND_V(face[0].size() != face[1].size(), ERR_FILE_CORRUPT);
			for (int i = 2; i < v.size() - 1; i++) {
				face[2] = v[i + 1].split("/");

				ERR_FAIL_COND_V(face[0].size() != face[2].size(), ERR_FILE_CORRUPT);
				for (int j = 0; j < 3; j++) {
					int idx = j;

					if (idx < 2) {
						idx = 1 ^ idx;
					}

					if (face[idx].size() == 3) {
						int norm = face[idx][2].to_int() - 1;
						if (norm < 0) {
							norm += normals.size() + 1;
						}
						ERR_FAIL_INDEX_V(norm, normals.size(), ERR_FILE_CORRUPT);
						surf_tool->set_normal(normals[norm]);
						if (generate_tangents && uvs.is_empty()) {
							// We can't generate tangents without UVs, so create dummy tangents.
							Vector3 tan = Vector3(normals[norm].z, -normals[norm].x, normals[norm].y).cross(normals[norm].normalized()).normalized();
							surf_tool->set_tangent(Plane(tan.x, tan.y, tan.z, 1.0));
						}
					} else {
						// No normals, use a dummy tangent since normals and tangents will be generated.
						if (generate_tangents && uvs.is_empty()) {
							// We can't generate tangents without UVs, so create dummy tangents.
							surf_tool->set_tangent(Plane(1.0, 0.0, 0.0, 1.0));
						}
					}

					if (face[idx].size() >= 2 && !face[idx][1].is_empty()) {
						int uv = face[idx][1].to_int() - 1;
						if (uv < 0) {
							uv += uvs.size() + 1;
						}
						ERR_FAIL_INDEX_V(uv, uvs.size(), ERR_FILE_CORRUPT);
						surf_tool->set_uv(uvs[uv]);
					}

					int vtx = face[idx][0].to_int() - 1;
					if (vtx < 0) {
						vtx += vertices.size() + 1;
					}
					ERR_FAIL_INDEX_V(vtx, vertices.size(), ERR_FILE_CORRUPT);

					Vector3 vertex = vertices[vtx];
					if (!colors.is_empty()) {
						surf_tool->set_color(colors[vtx]);
					}
					surf_tool->set_smooth_group(smoothing ? smooth_group : no_smoothing_smooth_group);
					surf_tool->add_vertex(vertex);
				}

				face[1] = face[2];
			}
		} else if (l.begins_with("s ")) { //smoothing
			String what = l.substr(2, l.length()).strip_edges();
			bool do_smooth;
			if (what == "off") {
				do_smooth = false;
			} else {
				do_smooth = true;
			}
			if (do_smooth != smoothing) {
				smoothing = do_smooth;
				if (smoothing) {
					smooth_group++;
				}
			}
		} else if (/*l.begins_with("g ") ||*/ l.begins_with("usemtl ") || (l.begins_with("o ") || f->eof_reached())) { //commit group to mesh
			uint64_t mesh_flags = RS::ARRAY_FLAG_COMPRESS_ATTRIBUTES;

			if (p_disable_compression) {
				mesh_flags = 0;
			} else {
				bool is_mesh_2d = true;

				// Disable compression if all z equals 0 (the mesh is 2D).
				for (int i = 0; i < vertices.size(); i++) {
					if (!Math::is_zero_approx(vertices[i].z)) {
						is_mesh_2d = false;
						break;
					}
				}

				if (is_mesh_2d) {
					mesh_flags = 0;
				}
			}

			//groups are too annoying
			if (surf_tool->get_vertex_array().size()) {
				//another group going on, commit it
				if (normals.size() == 0) {
					surf_tool->generate_normals();
				}

				if (generate_tangents && uvs.size()) {
					surf_tool->generate_tangents();
				}

				surf_tool->index();

				print_verbose("OBJ: Current material library " + current_material_library + " has " + itos(material_map.has(current_material_library)));
				print_verbose("OBJ: Current material " + current_material + " has " + itos(material_map.has(current_material_library) && material_map[current_material_library].has(current_material)));
				Ref<StandardMaterial3D> material;
				if (material_map.has(current_material_library) && material_map[current_material_library].has(current_material)) {
					material = material_map[current_material_library][current_material];
					if (!colors.is_empty()) {
						material->set_flag(StandardMaterial3D::FLAG_SRGB_VERTEX_COLOR, true);
					}
					surf_tool->set_material(material);
				}

				Array array = surf_tool->commit_to_arrays();

				if (mesh_flags & RS::ARRAY_FLAG_COMPRESS_ATTRIBUTES && generate_tangents) {
					// Compression is enabled, so let's validate that the normals and tangents are correct.
					Vector<Vector3> norms = array[Mesh::ARRAY_NORMAL];
					Vector<float> tangents = array[Mesh::ARRAY_TANGENT];
					for (int vert = 0; vert < norms.size(); vert++) {
						Vector3 tan = Vector3(tangents[vert * 4 + 0], tangents[vert * 4 + 1], tangents[vert * 4 + 2]);
						if (abs(tan.dot(norms[vert])) > 0.0001) {
							// Tangent is not perpendicular to the normal, so we can't use compression.
							mesh_flags &= ~RS::ARRAY_FLAG_COMPRESS_ATTRIBUTES;
						}
					}
				}

				mesh->add_surface(Mesh::PRIMITIVE_TRIANGLES, array, Array(), Dictionary(), material, name, mesh_flags);
				print_verbose("OBJ: Added surface :" + mesh->get_surface_name(mesh->get_surface_count() - 1));

				if (!current_material.is_empty()) {
					if (mesh->get_surface_count() >= 1) {
						mesh->set_surface_name(mesh->get_surface_count() - 1, current_material.get_basename());
					}
				} else if (!current_group.is_empty()) {
					if (mesh->get_surface_count() >= 1) {
						mesh->set_surface_name(mesh->get_surface_count() - 1, current_group);
					}
				}

				surf_tool->clear();
				surf_tool->begin(Mesh::PRIMITIVE_TRIANGLES);
			}

			if (l.begins_with("o ") || f->eof_reached()) {
				if (!p_single_mesh) {
					if (mesh->get_surface_count() > 0) {
						mesh->SetName(name);
						r_meshes.push_back(mesh);
						mesh.instantiate();
					}
					name = default_name;
					current_group = "";
					current_material = "";
				}
			}

			if (f->eof_reached()) {
				break;
			}

			if (l.begins_with("o ")) {
				name = l.substr(2, l.length()).strip_edges();
			}

			if (l.begins_with("usemtl ")) {
				current_material = l.replace("usemtl", "").strip_edges();
			}

			if (l.begins_with("g ")) {
				current_group = l.substr(2, l.length()).strip_edges();
			}

		} else if (l.begins_with("mtllib ")) { //parse material

			current_material_library = l.replace("mtllib", "").strip_edges();
			if (!material_map.has(current_material_library)) {
				HashMap<String, Ref<StandardMaterial3D>> lib;
				String lib_path = current_material_library;
				if (lib_path.is_relative_path()) {
					lib_path = p_path.get_base_dir().path_join(current_material_library);
				}
				Error err = _parse_material_library(lib_path, lib, r_missing_deps);
				if (err == OK) {
					material_map[current_material_library] = lib;
				}
			}
		}
	}

	if (p_single_mesh && mesh->get_surface_count() > 0) {
		r_meshes.push_back(mesh);
	}

	return OK;
}

void lain::ResourceImporterOBJ::get_recognized_extensions(List<String>* p_extensions) const {
  p_extensions->push_back("obj");
}

void lain::ResourceImporterOBJ::get_recognized_resources(List<String>* p_resource) const {
  p_resource->push_back("Mesh");
}
Error ResourceImporterOBJ::import(const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata) {
	List<Ref<ImporterMesh>> meshes;

	Error err = _parse_obj(p_source_file, meshes, true, p_options["generate_tangents"], p_options["optimize_mesh"], p_options["scale_mesh"], p_options["offset_mesh"], p_options["force_disable_mesh_compression"], nullptr);

	ERR_FAIL_COND_V(err != OK, err);
	ERR_FAIL_COND_V(meshes.size() != 1, ERR_BUG);

	String save_path = p_save_path + ".mesh";

	err = ResourceSaver::save(meshes.front()->get()->get_mesh(), save_path);

	ERR_FAIL_COND_V_MSG(err != OK, err, "Cannot save Mesh to file '" + save_path + "'.");

	r_gen_files->push_back(save_path);

	return OK;
}



void lain::ImporterMesh::Surface::split_normals(const LocalVector<int>& p_indices, const LocalVector<Vector3>& p_normals) {
  	_split_normals(arrays, p_indices, p_normals);

	for (BlendShape &blend_shape : blend_shape_data) {
		_split_normals(blend_shape.arrays, p_indices, p_normals);
	}
}

void lain::ImporterMesh::Surface::_split_normals(Array& r_arrays, const LocalVector<int>& p_indices, const LocalVector<Vector3>& p_normals) {
  ERR_FAIL_COND(r_arrays.size() != RS::ARRAY_MAX);

	const PackedVector3Array &vertices = r_arrays[RS::ARRAY_VERTEX];
	int current_vertex_count = vertices.size();
	int new_vertex_count = p_indices.size();
	int final_vertex_count = current_vertex_count + new_vertex_count;
	const int *indices_ptr = p_indices.ptr();

	for (int i = 0; i < r_arrays.size(); i++) {
		if (i == RS::ARRAY_INDEX) {
			continue;
		}

		if (r_arrays[i].get_type() == Variant::NIL) {
			continue;
		}

		switch (r_arrays[i].get_type()) {
			case Variant::PACKED_VECTOR3_ARRAY: {
				PackedVector3Array data = r_arrays[i];
				data.resize(final_vertex_count);
				Vector3 *data_ptr = data.ptrw();
				if (i == RS::ARRAY_NORMAL) {
					const Vector3 *normals_ptr = p_normals.ptr();
					memcpy(&data_ptr[current_vertex_count], normals_ptr, sizeof(Vector3) * new_vertex_count);
				} else {
					for (int j = 0; j < new_vertex_count; j++) {
						data_ptr[current_vertex_count + j] = data_ptr[indices_ptr[j]];
					}
				}
				r_arrays[i] = data;
			} break;
			case Variant::PACKED_VECTOR2_ARRAY: {
				PackedVector2Array data = r_arrays[i];
				data.resize(final_vertex_count);
				Vector2 *data_ptr = data.ptrw();
				for (int j = 0; j < new_vertex_count; j++) {
					data_ptr[current_vertex_count + j] = data_ptr[indices_ptr[j]];
				}
				r_arrays[i] = data;
			} break;
			case Variant::PACKED_FLOAT32_ARRAY: {
				PackedFloat32Array data = r_arrays[i];
				int elements = data.size() / current_vertex_count;
				data.resize(final_vertex_count * elements);
				float *data_ptr = data.ptrw();
				for (int j = 0; j < new_vertex_count; j++) {
					memcpy(&data_ptr[(current_vertex_count + j) * elements], &data_ptr[indices_ptr[j] * elements], sizeof(float) * elements);
				}
				r_arrays[i] = data;
			} break;
			case Variant::PACKED_INT32_ARRAY: {
				PackedInt32Array data = r_arrays[i];
				int elements = data.size() / current_vertex_count;
				data.resize(final_vertex_count * elements);
				int32_t *data_ptr = data.ptrw();
				for (int j = 0; j < new_vertex_count; j++) {
					memcpy(&data_ptr[(current_vertex_count + j) * elements], &data_ptr[indices_ptr[j] * elements], sizeof(int32_t) * elements);
				}
				r_arrays[i] = data;
			} break;
			case Variant::PACKED_BYTE_ARRAY: {
				PackedByteArray data = r_arrays[i];
				int elements = data.size() / current_vertex_count;
				data.resize(final_vertex_count * elements);
				uint8_t *data_ptr = data.ptrw();
				for (int j = 0; j < new_vertex_count; j++) {
					memcpy(&data_ptr[(current_vertex_count + j) * elements], &data_ptr[indices_ptr[j] * elements], sizeof(uint8_t) * elements);
				}
				r_arrays[i] = data;
			} break;
			case Variant::PACKED_COLOR_ARRAY: {
				PackedColorArray data = r_arrays[i];
				data.resize(final_vertex_count);
				Color *data_ptr = data.ptrw();
				for (int j = 0; j < new_vertex_count; j++) {
					data_ptr[current_vertex_count + j] = data_ptr[indices_ptr[j]];
				}
				r_arrays[i] = data;
			} break;
			default: {
				ERR_FAIL_MSG("Unhandled array type.");
			} break;
		}
	}
}

void ImporterMesh::add_blend_shape(const String &p_name) {
	ERR_FAIL_COND(surfaces.size() > 0);
	blend_shapes.push_back(p_name);
}

int ImporterMesh::get_blend_shape_count() const {
	return blend_shapes.size();
}

String ImporterMesh::get_blend_shape_name(int p_blend_shape) const {
	ERR_FAIL_INDEX_V(p_blend_shape, blend_shapes.size(), String());
	return blend_shapes[p_blend_shape];
}

void ImporterMesh::set_blend_shape_mode(Mesh::BlendShapeMode p_blend_shape_mode) {
	blend_shape_mode = p_blend_shape_mode;
}

Mesh::BlendShapeMode ImporterMesh::get_blend_shape_mode() const {
	return blend_shape_mode;
}


void ImporterMesh::add_surface(Mesh::PrimitiveType p_primitive, const Array &p_arrays, const Array &p_blend_shapes, const Dictionary &p_lods, const Ref<Material> &p_material, const String &p_name, const uint64_t p_flags) {
	ERR_FAIL_COND(p_blend_shapes.size() != blend_shapes.size());
	ERR_FAIL_COND(p_arrays.size() != Mesh::ARRAY_MAX);
	Surface s;
	s.primitive = p_primitive;
	s.arrays = p_arrays;
	s.name = p_name;
	s.flags = p_flags;

	Vector<Vector3> vertex_array = p_arrays[Mesh::ARRAY_VERTEX];
	int vertex_count = vertex_array.size();
	ERR_FAIL_COND(vertex_count == 0);

	for (int i = 0; i < blend_shapes.size(); i++) {
		Array bsdata = p_blend_shapes[i];
		ERR_FAIL_COND(bsdata.size() != Mesh::ARRAY_MAX);
		Vector<Vector3> vertex_data = bsdata[Mesh::ARRAY_VERTEX];
		ERR_FAIL_COND(vertex_data.size() != vertex_count);
		Surface::BlendShape bs;
		bs.arrays = bsdata;
		s.blend_shape_data.push_back(bs);
	}

	List<Variant> lods;
	p_lods.get_key_list(&lods);
	for (const Variant &E : lods) {
		ERR_CONTINUE(!E.is_num());
		Surface::LOD lod;
		lod.distance = E;
		lod.indices = p_lods[E];
		ERR_CONTINUE(lod.indices.is_empty());
		s.lods.push_back(lod);
	}

	s.material = p_material;

	surfaces.push_back(s);
	mesh.unref();
}

int ImporterMesh::get_surface_count() const {
	return surfaces.size();
}

Mesh::PrimitiveType ImporterMesh::get_surface_primitive_type(int p_surface) {
	ERR_FAIL_INDEX_V(p_surface, surfaces.size(), Mesh::PRIMITIVE_MAX);
	return surfaces[p_surface].primitive;
}
Array ImporterMesh::get_surface_arrays(int p_surface) const {
	ERR_FAIL_INDEX_V(p_surface, surfaces.size(), Array());
	return surfaces[p_surface].arrays;
}
String ImporterMesh::get_surface_name(int p_surface) const {
	ERR_FAIL_INDEX_V(p_surface, surfaces.size(), String());
	return surfaces[p_surface].name;
}
void ImporterMesh::set_surface_name(int p_surface, const String &p_name) {
	ERR_FAIL_INDEX(p_surface, surfaces.size());
	surfaces.write[p_surface].name = p_name;
	mesh.unref();
}

Array ImporterMesh::get_surface_blend_shape_arrays(int p_surface, int p_blend_shape) const {
	ERR_FAIL_INDEX_V(p_surface, surfaces.size(), Array());
	ERR_FAIL_INDEX_V(p_blend_shape, surfaces[p_surface].blend_shape_data.size(), Array());
	return surfaces[p_surface].blend_shape_data[p_blend_shape].arrays;
}
int ImporterMesh::get_surface_lod_count(int p_surface) const {
	ERR_FAIL_INDEX_V(p_surface, surfaces.size(), 0);
	return surfaces[p_surface].lods.size();
}
Vector<int> ImporterMesh::get_surface_lod_indices(int p_surface, int p_lod) const {
	ERR_FAIL_INDEX_V(p_surface, surfaces.size(), Vector<int>());
	ERR_FAIL_INDEX_V(p_lod, surfaces[p_surface].lods.size(), Vector<int>());

	return surfaces[p_surface].lods[p_lod].indices;
}

float ImporterMesh::get_surface_lod_size(int p_surface, int p_lod) const {
	ERR_FAIL_INDEX_V(p_surface, surfaces.size(), 0);
	ERR_FAIL_INDEX_V(p_lod, surfaces[p_surface].lods.size(), 0);
	return surfaces[p_surface].lods[p_lod].distance;
}

uint64_t ImporterMesh::get_surface_format(int p_surface) const {
	ERR_FAIL_INDEX_V(p_surface, surfaces.size(), 0);
	return surfaces[p_surface].flags;
}

Ref<Material> ImporterMesh::get_surface_material(int p_surface) const {
	ERR_FAIL_INDEX_V(p_surface, surfaces.size(), Ref<Material>());
	return surfaces[p_surface].material;
}

void ImporterMesh::set_surface_material(int p_surface, const Ref<Material> &p_material) {
	ERR_FAIL_INDEX(p_surface, surfaces.size());
	surfaces.write[p_surface].material = p_material;
	mesh.unref();
}



Ref<ArrayMesh> ImporterMesh::get_mesh(const Ref<ArrayMesh> &p_base) {
	ERR_FAIL_COND_V(surfaces.is_empty(), Ref<ArrayMesh>());

	if (mesh.is_null()) {
		if (p_base.is_valid()) {
			mesh = p_base;
		}
		if (mesh.is_null()) {
			mesh.instantiate();
		}
		// mesh->SetName(get_name());
		// if (has_meta("import_id")) {
		// 	mesh->set_meta("import_id", get_meta("import_id"));
		// }
		for (int i = 0; i < blend_shapes.size(); i++) {
			mesh->add_blend_shape(blend_shapes[i]);
		}
		mesh->set_blend_shape_mode(blend_shape_mode);
		for (int i = 0; i < surfaces.size(); i++) {
			Array bs_data;
			if (surfaces[i].blend_shape_data.size()) {
				for (int j = 0; j < surfaces[i].blend_shape_data.size(); j++) {
					bs_data.push_back(surfaces[i].blend_shape_data[j].arrays);
				}
			}
			Dictionary lods;
			if (surfaces[i].lods.size()) {
				for (int j = 0; j < surfaces[i].lods.size(); j++) {
					lods[surfaces[i].lods[j].distance] = surfaces[i].lods[j].indices;
				}
			}

			mesh->add_surface_from_arrays(surfaces[i].primitive, surfaces[i].arrays, bs_data, lods, surfaces[i].flags);
			if (surfaces[i].material.is_valid()) {
				mesh->surface_set_material(mesh->get_surface_count() - 1, surfaces[i].material);
			}
			if (!surfaces[i].name.is_empty()) {
				mesh->surface_set_name(mesh->get_surface_count() - 1, surfaces[i].name);
			}
		}

		mesh->set_lightmap_size_hint(lightmap_size_hint);

		if (shadow_mesh.is_valid()) {
			Ref<ArrayMesh> shadow = shadow_mesh->get_mesh();
			mesh->set_shadow_mesh(shadow);
		}
	}

	return mesh;
}

void ImporterMesh::clear() {
	surfaces.clear();
	blend_shapes.clear();
	mesh.unref();
}


void ImporterMesh::create_shadow_mesh() {
	if (shadow_mesh.is_valid()) {
		shadow_mesh.unref();
	}

	//no shadow mesh for blendshapes
	if (blend_shapes.size() > 0) {
		return;
	}
	//no shadow mesh for skeletons
	for (int i = 0; i < surfaces.size(); i++) {
		if (surfaces[i].arrays[RS::ARRAY_BONES].get_type() != Variant::NIL) {
			return;
		}
		if (surfaces[i].arrays[RS::ARRAY_WEIGHTS].get_type() != Variant::NIL) {
			return;
		}
	}

	shadow_mesh.instantiate();

	for (int i = 0; i < surfaces.size(); i++) {
		LocalVector<int> vertex_remap;
		Vector<Vector3> new_vertices;
		Vector<Vector3> vertices = surfaces[i].arrays[RS::ARRAY_VERTEX];
		int vertex_count = vertices.size();
		{
			HashMap<Vector3, int> unique_vertices;
			const Vector3 *vptr = vertices.ptr();
			for (int j = 0; j < vertex_count; j++) {
				const Vector3 &v = vptr[j];

				HashMap<Vector3, int>::Iterator E = unique_vertices.find(v);

				if (E) {
					vertex_remap.push_back(E->value);
				} else {
					int vcount = unique_vertices.size();
					unique_vertices[v] = vcount;
					vertex_remap.push_back(vcount);
					new_vertices.push_back(v);
				}
			}
		}

		Array new_surface;
		new_surface.resize(RS::ARRAY_MAX);
		Dictionary lods;

		//		print_line("original vertex count: " + itos(vertices.size()) + " new vertex count: " + itos(new_vertices.size()));

		new_surface[RS::ARRAY_VERTEX] = new_vertices;

		Vector<int> indices = surfaces[i].arrays[RS::ARRAY_INDEX];
		if (indices.size()) {
			int index_count = indices.size();
			const int *index_rptr = indices.ptr();
			Vector<int> new_indices;
			new_indices.resize(indices.size());
			int *index_wptr = new_indices.ptrw();

			for (int j = 0; j < index_count; j++) {
				int index = index_rptr[j];
				ERR_FAIL_INDEX(index, vertex_count);
				index_wptr[j] = vertex_remap[index];
			}

			new_surface[RS::ARRAY_INDEX] = new_indices;

			// Make sure the same LODs as the full version are used.
			// This makes it more coherent between rendered model and its shadows.
			for (int j = 0; j < surfaces[i].lods.size(); j++) {
				indices = surfaces[i].lods[j].indices;

				index_count = indices.size();
				index_rptr = indices.ptr();
				new_indices.resize(indices.size());
				index_wptr = new_indices.ptrw();

				for (int k = 0; k < index_count; k++) {
					int index = index_rptr[k];
					ERR_FAIL_INDEX(index, vertex_count);
					index_wptr[k] = vertex_remap[index];
				}

				lods[surfaces[i].lods[j].distance] = new_indices;
			}
		}

		shadow_mesh->add_surface(surfaces[i].primitive, new_surface, Array(), lods, Ref<Material>(), surfaces[i].name, surfaces[i].flags);
	}
}

Ref<Resource> lain::ResourceFormatLoaderOBJ::load(const String& p_path, const String& p_original_path, Error* r_error, bool p_use_sub_threads, float* r_progress,
                                                  CacheMode p_cache_mode) {
  String path = !p_original_path.is_empty() ? p_original_path : p_path;

  List<Ref<ImporterMesh>> meshes;

	Error err = _parse_obj(path, meshes, true, true, false, {1,1,1}, {} ,false , nullptr);

	ERR_FAIL_COND_V(err != OK, err);
	ERR_FAIL_COND_V(meshes.size() != 1, ERR_BUG);
  Ref<Resource> res  = meshes.front()->get()->get_mesh();
  return meshes.front()->get()->get_mesh();
}

void lain::ResourceFormatLoaderOBJ::get_recognized_extensions(List<String>* p_extensions) const
{
  p_extensions->push_back("obj");
  
}

void lain::ResourceFormatLoaderOBJ::get_recognized_resources(List<String>* p_resource) const {
  p_resource->push_back("ArrayMesh");
 
}
