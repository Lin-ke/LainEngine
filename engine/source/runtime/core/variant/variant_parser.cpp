//#include "variant_parser.h"
//namespace lain {
//	static String rtos_fix(double p_value) {
//		if (p_value == 0.0) {
//			return "0"; //avoid negative zero (-0) being written, which may annoy git, svn, etc. for changes when they don't exist.
//		}
//		else if (isnan(p_value)) {
//			return "nan";
//		}
//		else if (isinf(p_value)) {
//			if (p_value > 0) {
//				return "inf";
//			}
//			else {
//				return "inf_neg";
//			}
//		}
//		else {
//			return rtoss(p_value);
//		}
//	}
//	Error VariantWriter::write(const Variant& p_variant, StoreStringFunc p_store_string_func, void* p_store_string_ud, EncodeResourceFunc p_encode_res_func, void* p_encode_res_ud, int p_recursion_count) {
//		switch (p_variant.get_type()) {
//		case Variant::NIL: {
//			p_store_string_func(p_store_string_ud, "null");
//		} break;
//		case Variant::BOOL: {
//			p_store_string_func(p_store_string_ud, p_variant.operator bool() ? "true" : "false");
//		} break;
//		case Variant::INT: {
//			p_store_string_func(p_store_string_ud, itos(p_variant.operator int64_t()));
//		} break;
//		case Variant::FLOAT: {
//			String s = rtos_fix(p_variant.operator double());
//			if (s != "inf" && s != "inf_neg" && s != "nan") {
//				if (!s.contains(".") && !s.contains("e")) {
//					s += ".0";
//				}
//			}
//			p_store_string_func(p_store_string_ud, s);
//		} break;
//		case Variant::STRING: {
//			String str = p_variant;
//			str = "\"" + str.c_escape_multiline() + "\"";
//			p_store_string_func(p_store_string_ud, str);
//		} break;
//
//			// Math types.
//		case Variant::VECTOR2: {
//			Vector2 v = p_variant;
//			p_store_string_func(p_store_string_ud, "Vector2(" + rtos_fix(v.x) + ", " + rtos_fix(v.y) + ")");
//		} break;
//		/*case Variant::VECTOR2I: {
//			Vector2i v = p_variant;
//			p_store_string_func(p_store_string_ud, "Vector2i(" + itos(v.x) + ", " + itos(v.y) + ")");
//		} break;
//		case Variant::RECT2: {
//			Rect2 aabb = p_variant;
//			p_store_string_func(p_store_string_ud, "Rect2(" + rtos_fix(aabb.position.x) + ", " + rtos_fix(aabb.position.y) + ", " + rtos_fix(aabb.size.x) + ", " + rtos_fix(aabb.size.y) + ")");
//		} break;
//		case Variant::RECT2I: {
//			Rect2i aabb = p_variant;
//			p_store_string_func(p_store_string_ud, "Rect2i(" + itos(aabb.position.x) + ", " + itos(aabb.position.y) + ", " + itos(aabb.size.x) + ", " + itos(aabb.size.y) + ")");
//		} break;*/
//		case Variant::VECTOR3: {
//			Vector3 v = p_variant;
//			p_store_string_func(p_store_string_ud, "Vector3(" + rtos_fix(v.x) + ", " + rtos_fix(v.y) + ", " + rtos_fix(v.z) + ")");
//		} break;
//		/*case Variant::VECTOR3I: {
//			Vector3i v = p_variant;
//			p_store_string_func(p_store_string_ud, "Vector3i(" + itos(v.x) + ", " + itos(v.y) + ", " + itos(v.z) + ")");
//		} break;*/
//		case Variant::VECTOR4: {
//			Vector4 v = p_variant;
//			p_store_string_func(p_store_string_ud, "Vector4(" + rtos_fix(v.x) + ", " + rtos_fix(v.y) + ", " + rtos_fix(v.z) + ", " + rtos_fix(v.w) + ")");
//		} break;
//		/*case Variant::VECTOR4I: {
//			Vector4i v = p_variant;
//			p_store_string_func(p_store_string_ud, "Vector4i(" + itos(v.x) + ", " + itos(v.y) + ", " + itos(v.z) + ", " + itos(v.w) + ")");
//		} break;
//		case Variant::PLANE: {
//			Plane p = p_variant;
//			p_store_string_func(p_store_string_ud, "Plane(" + rtos_fix(p.normal.x) + ", " + rtos_fix(p.normal.y) + ", " + rtos_fix(p.normal.z) + ", " + rtos_fix(p.d) + ")");
//		} break;
//		case Variant::AABB: {
//			AABB aabb = p_variant;
//			p_store_string_func(p_store_string_ud, "AABB(" + rtos_fix(aabb.position.x) + ", " + rtos_fix(aabb.position.y) + ", " + rtos_fix(aabb.position.z) + ", " + rtos_fix(aabb.size.x) + ", " + rtos_fix(aabb.size.y) + ", " + rtos_fix(aabb.size.z) + ")");
//		} break;*/
//		case Variant::QUATERNION: {
//			Quaternion quaternion = p_variant;
//			p_store_string_func(p_store_string_ud, "Quaternion(" + rtos_fix(quaternion.x) + ", " + rtos_fix(quaternion.y) + ", " + rtos_fix(quaternion.z) + ", " + rtos_fix(quaternion.w) + ")");
//		} break;
//		/*case Variant::TRANSFORM2D: {
//			String s = "Transform2D(";
//			Transform2D m3 = p_variant;
//			for (int i = 0; i < 3; i++) {
//				for (int j = 0; j < 2; j++) {
//					if (i != 0 || j != 0) {
//						s += ", ";
//					}
//					s += rtos_fix(m3.columns[i][j]);
//				}
//			}
//
//			p_store_string_func(p_store_string_ud, s + ")");
//		} break;*/
//		/*case Variant::BASIS: {
//			String s = "Basis(";
//			Basis m3 = p_variant;
//			for (int i = 0; i < 3; i++) {
//				for (int j = 0; j < 3; j++) {
//					if (i != 0 || j != 0) {
//						s += ", ";
//					}
//					s += rtos_fix(m3.rows[i][j]);
//				}
//			}
//
//			p_store_string_func(p_store_string_ud, s + ")");
//		} break;
//		case Variant::TRANSFORM3D: {
//			String s = "Transform3D(";
//			Transform3D t = p_variant;
//			Basis& m3 = t.basis;
//			for (int i = 0; i < 3; i++) {
//				for (int j = 0; j < 3; j++) {
//					if (i != 0 || j != 0) {
//						s += ", ";
//					}
//					s += rtos_fix(m3.rows[i][j]);
//				}
//			}
//
//			s = s + ", " + rtos_fix(t.origin.x) + ", " + rtos_fix(t.origin.y) + ", " + rtos_fix(t.origin.z);
//
//			p_store_string_func(p_store_string_ud, s + ")");
//		} break;
//		case Variant::PROJECTION: {
//			String s = "Projection(";
//			Projection t = p_variant;
//			for (int i = 0; i < 4; i++) {
//				for (int j = 0; j < 4; j++) {
//					if (i != 0 || j != 0) {
//						s += ", ";
//					}
//					s += rtos_fix(t.columns[i][j]);
//				}
//			}
//
//			p_store_string_func(p_store_string_ud, s + ")");
//		} break;*/
//
//			// Misc types.
//		case Variant::COLOR: {
//			Color c = p_variant;
//			p_store_string_func(p_store_string_ud, "Color(" + rtos_fix(c.r) + ", " + rtos_fix(c.g) + ", " + rtos_fix(c.b) + ", " + rtos_fix(c.a) + ")");
//		} break;
//		case Variant::STRING_NAME: {
//			String str = p_variant;
//			str = "&\"" + str.c_escape() + "\"";
//			p_store_string_func(p_store_string_ud, str);
//		} break;
//		case Variant::NODE_PATH: {
//			String str = p_variant;
//			str = "GObjectPath(\"" + str.c_escape() + "\")";
//			p_store_string_func(p_store_string_ud, str);
//		} break;
//		case Variant::RID: {
//			RID rid = p_variant;
//			if (rid == RID()) {
//				p_store_string_func(p_store_string_ud, "RID()");
//			}
//			else {
//				p_store_string_func(p_store_string_ud, "RID(" + itos(rid.get_id()) + ")");
//			}
//		} break;
//
//			// Do not really store these, but ensure that assignments are not empty.
//		case Variant::SIGNAL: {
//			p_store_string_func(p_store_string_ud, "Signal()");
//		} break;
//		case Variant::CALLABLE: {
//			p_store_string_func(p_store_string_ud, "Callable()");
//		} break;
//
//		//case Variant::OBJECT: { // resource/generic object
//		//	if (unlikely(p_recursion_count > MAX_RECURSION)) {
//		//		ERR_PRINT("Max recursion reached");
//		//		p_store_string_func(p_store_string_ud, "null");
//		//		return OK;
//		//	}
//		//	p_recursion_count++;
//
//		//	Object* obj = p_variant.get_validated_object();
//
//		//	if (!obj) {
//		//		p_store_string_func(p_store_string_ud, "null");
//		//		break; // don't save it
//		//	}
//
//		//	Ref<Resource> res = p_variant;
//		//	if (res.is_valid()) {
//		//		//is resource
//		//		String res_text;
//
//		//		//try external function
//		//		if (p_encode_res_func) {
//		//			res_text = p_encode_res_func(p_encode_res_ud, res);
//		//		}
//
//		//		//try path because it's a file
//		//		if (res_text.is_empty() && res->get_path().is_resource_file()) {
//		//			//external resource
//		//			String path = res->get_path();
//		//			res_text = "Resource(\"" + path + "\")";
//		//		}
//
//		//		//could come up with some sort of text
//		//		if (!res_text.is_empty()) {
//		//			p_store_string_func(p_store_string_ud, res_text);
//		//			break;
//		//		}
//		//	}
//
//		//	//store as generic object
//
//		//	p_store_string_func(p_store_string_ud, "Object(" + obj->get_class() + ",");
//
//		//	List<PropertyInfo> props;
//		//	obj->get_property_list(&props);
//		//	bool first = true;
//		//	for (const PropertyInfo& E : props) {
//		//		if (E.usage & PROPERTY_USAGE_STORAGE || E.usage & PROPERTY_USAGE_SCRIPT_VARIABLE) {
//		//			//must be serialized
//
//		//			if (first) {
//		//				first = false;
//		//			}
//		//			else {
//		//				p_store_string_func(p_store_string_ud, ",");
//		//			}
//
//		//			p_store_string_func(p_store_string_ud, "\"" + E.name + "\":");
//		//			write(obj->get(E.name), p_store_string_func, p_store_string_ud, p_encode_res_func, p_encode_res_ud, p_recursion_count);
//		//		}
//		//	}
//
//		//	p_store_string_func(p_store_string_ud, ")\n");
//		//} break;
//
//		//case Variant::DICTIONARY: {
//		//	Dictionary dict = p_variant;
//		//	if (unlikely(p_recursion_count > MAX_RECURSION)) {
//		//		ERR_PRINT("Max recursion reached");
//		//		p_store_string_func(p_store_string_ud, "{}");
//		//	}
//		//	else {
//		//		p_recursion_count++;
//
//		//		List<Variant> keys;
//		//		dict.get_key_list(&keys);
//		//		keys.sort();
//
//		//		if (keys.is_empty()) { // Avoid unnecessary line break.
//		//			p_store_string_func(p_store_string_ud, "{}");
//		//			break;
//		//		}
//
//		//		p_store_string_func(p_store_string_ud, "{\n");
//		//		for (List<Variant>::Element* E = keys.front(); E; E = E->next()) {
//		//			write(E->get(), p_store_string_func, p_store_string_ud, p_encode_res_func, p_encode_res_ud, p_recursion_count);
//		//			p_store_string_func(p_store_string_ud, ": ");
//		//			write(dict[E->get()], p_store_string_func, p_store_string_ud, p_encode_res_func, p_encode_res_ud, p_recursion_count);
//		//			if (E->next()) {
//		//				p_store_string_func(p_store_string_ud, ",\n");
//		//			}
//		//			else {
//		//				p_store_string_func(p_store_string_ud, "\n");
//		//			}
//		//		}
//
//		//		p_store_string_func(p_store_string_ud, "}");
//		//	}
//		//} break;
//
//		//case Variant::ARRAY: {
//		//	Array array = p_variant;
//		//	if (array.get_typed_builtin() != Variant::NIL) {
//		//		p_store_string_func(p_store_string_ud, "Array[");
//
//		//		Variant::Type builtin_type = (Variant::Type)array.get_typed_builtin();
//		//		StringName class_name = array.get_typed_class_name();
//		//		Ref<Script> script = array.get_typed_script();
//
//		//		if (script.is_valid()) {
//		//			String resource_text = String();
//		//			if (p_encode_res_func) {
//		//				resource_text = p_encode_res_func(p_encode_res_ud, script);
//		//			}
//		//			if (resource_text.is_empty() && script->get_path().is_resource_file()) {
//		//				resource_text = "Resource(\"" + script->get_path() + "\")";
//		//			}
//
//		//			if (!resource_text.is_empty()) {
//		//				p_store_string_func(p_store_string_ud, resource_text);
//		//			}
//		//			else {
//		//				ERR_PRINT("Failed to encode a path to a custom script for an array type.");
//		//				p_store_string_func(p_store_string_ud, class_name);
//		//			}
//		//		}
//		//		else if (class_name != StringName()) {
//		//			p_store_string_func(p_store_string_ud, class_name);
//		//		}
//		//		else {
//		//			p_store_string_func(p_store_string_ud, Variant::get_type_name(builtin_type));
//		//		}
//
//		//		p_store_string_func(p_store_string_ud, "](");
//		//	}
//
//		//	if (unlikely(p_recursion_count > MAX_RECURSION)) {
//		//		ERR_PRINT("Max recursion reached");
//		//		p_store_string_func(p_store_string_ud, "[]");
//		//	}
//		//	else {
//		//		p_recursion_count++;
//
//		//		p_store_string_func(p_store_string_ud, "[");
//		//		int len = array.size();
//		//		for (int i = 0; i < len; i++) {
//		//			if (i > 0) {
//		//				p_store_string_func(p_store_string_ud, ", ");
//		//			}
//		//			write(array[i], p_store_string_func, p_store_string_ud, p_encode_res_func, p_encode_res_ud, p_recursion_count);
//		//		}
//
//		//		p_store_string_func(p_store_string_ud, "]");
//		//	}
//
//		//	if (array.get_typed_builtin() != Variant::NIL) {
//		//		p_store_string_func(p_store_string_ud, ")");
//		//	}
//		//} break;
//
//		case Variant::PACKED_BYTE_ARRAY: {
//			p_store_string_func(p_store_string_ud, "PackedByteArray(");
//			Vector<uint8_t> data = p_variant;
//			int len = data.size();
//			const uint8_t* ptr = data.ptr();
//
//			for (int i = 0; i < len; i++) {
//				if (i > 0) {
//					p_store_string_func(p_store_string_ud, ", ");
//				}
//
//				p_store_string_func(p_store_string_ud, itos(ptr[i]));
//			}
//
//			p_store_string_func(p_store_string_ud, ")");
//		} break;
//		case Variant::PACKED_INT32_ARRAY: {
//			p_store_string_func(p_store_string_ud, "PackedInt32Array(");
//			Vector<int32_t> data = p_variant;
//			int32_t len = data.size();
//			const int32_t* ptr = data.ptr();
//
//			for (int32_t i = 0; i < len; i++) {
//				if (i > 0) {
//					p_store_string_func(p_store_string_ud, ", ");
//				}
//
//				p_store_string_func(p_store_string_ud, itos(ptr[i]));
//			}
//
//			p_store_string_func(p_store_string_ud, ")");
//		} break;
//		case Variant::PACKED_INT64_ARRAY: {
//			p_store_string_func(p_store_string_ud, "PackedInt64Array(");
//			Vector<int64_t> data = p_variant;
//			int64_t len = data.size();
//			const int64_t* ptr = data.ptr();
//
//			for (int64_t i = 0; i < len; i++) {
//				if (i > 0) {
//					p_store_string_func(p_store_string_ud, ", ");
//				}
//
//				p_store_string_func(p_store_string_ud, itos(ptr[i]));
//			}
//
//			p_store_string_func(p_store_string_ud, ")");
//		} break;
//		case Variant::PACKED_FLOAT32_ARRAY: {
//			p_store_string_func(p_store_string_ud, "PackedFloat32Array(");
//			Vector<float> data = p_variant;
//			int len = data.size();
//			const float* ptr = data.ptr();
//
//			for (int i = 0; i < len; i++) {
//				if (i > 0) {
//					p_store_string_func(p_store_string_ud, ", ");
//				}
//				p_store_string_func(p_store_string_ud, rtos_fix(ptr[i]));
//			}
//
//			p_store_string_func(p_store_string_ud, ")");
//		} break;
//		case Variant::PACKED_FLOAT64_ARRAY: {
//			p_store_string_func(p_store_string_ud, "PackedFloat64Array(");
//			Vector<double> data = p_variant;
//			int len = data.size();
//			const double* ptr = data.ptr();
//
//			for (int i = 0; i < len; i++) {
//				if (i > 0) {
//					p_store_string_func(p_store_string_ud, ", ");
//				}
//				p_store_string_func(p_store_string_ud, rtos_fix(ptr[i]));
//			}
//
//			p_store_string_func(p_store_string_ud, ")");
//		} break;
//		case Variant::PACKED_STRING_ARRAY: {
//			p_store_string_func(p_store_string_ud, "PackedStringArray(");
//			Vector<String> data = p_variant;
//			int len = data.size();
//			const String* ptr = data.ptr();
//
//			for (int i = 0; i < len; i++) {
//				if (i > 0) {
//					p_store_string_func(p_store_string_ud, ", ");
//				}
//				p_store_string_func(p_store_string_ud, "\"" + ptr[i].c_escape() + "\"");
//			}
//
//			p_store_string_func(p_store_string_ud, ")");
//		} break;
//		case Variant::PACKED_VECTOR2_ARRAY: {
//			p_store_string_func(p_store_string_ud, "PackedVector2Array(");
//			Vector<Vector2> data = p_variant;
//			int len = data.size();
//			const Vector2* ptr = data.ptr();
//
//			for (int i = 0; i < len; i++) {
//				if (i > 0) {
//					p_store_string_func(p_store_string_ud, ", ");
//				}
//				p_store_string_func(p_store_string_ud, rtos_fix(ptr[i].x) + ", " + rtos_fix(ptr[i].y));
//			}
//
//			p_store_string_func(p_store_string_ud, ")");
//		} break;
//		case Variant::PACKED_VECTOR3_ARRAY: {
//			p_store_string_func(p_store_string_ud, "PackedVector3Array(");
//			Vector<Vector3> data = p_variant;
//			int len = data.size();
//			const Vector3* ptr = data.ptr();
//
//			for (int i = 0; i < len; i++) {
//				if (i > 0) {
//					p_store_string_func(p_store_string_ud, ", ");
//				}
//				p_store_string_func(p_store_string_ud, rtos_fix(ptr[i].x) + ", " + rtos_fix(ptr[i].y) + ", " + rtos_fix(ptr[i].z));
//			}
//
//			p_store_string_func(p_store_string_ud, ")");
//		} break;
//		case Variant::PACKED_COLOR_ARRAY: {
//			p_store_string_func(p_store_string_ud, "PackedColorArray(");
//			Vector<Color> data = p_variant;
//			int len = data.size();
//			const Color* ptr = data.ptr();
//
//			for (int i = 0; i < len; i++) {
//				if (i > 0) {
//					p_store_string_func(p_store_string_ud, ", ");
//				}
//				p_store_string_func(p_store_string_ud, rtos_fix(ptr[i].r) + ", " + rtos_fix(ptr[i].g) + ", " + rtos_fix(ptr[i].b) + ", " + rtos_fix(ptr[i].a));
//			}
//
//			p_store_string_func(p_store_string_ud, ")");
//		} break;
//
//		default: {
//			ERR_PRINT("Unknown variant type");
//			return ERR_BUG;
//		}
//		}
//
//		return OK;
//	}
//	static Error _write_to_str(void* ud, const String& p_string) {
//		String* str = (String*)ud;
//		(*str) += p_string;
//		return OK;
//	}
//	Error VariantWriter::write_to_string(const Variant& p_variant, String& r_string, EncodeResourceFunc p_encode_res_func, void* p_encode_res_ud) {
//		r_string = String();
//
//		return write(p_variant, _write_to_str, &r_string, p_encode_res_func, p_encode_res_ud);
//	}
//
//}