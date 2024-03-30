#include "test_reflect.h"
#include "core/string/string_name.h"

namespace lain {
	namespace test {

		int test_StringName_reflect(){
			StringName p{ "hello" };
			String value = "hi";
			test_meta(p, "StringName", "name", &value);
			return 0; 
		}
		int test_Vector3_reflect() {
			Vector3 p{ 1,2,3 };
			float value = 10;
			test_meta(p, "Vector3", "x", &value);
			return 0;
		}
		
		int test_accessor() {
			Vector2 p(1, 2);
			Vector2 m(3, 4);
			using Reflection::TypeMeta;
			TypeMeta meta = TypeMeta::newMetaFromName("Vector2");
			if (meta.isValid()) {

				
			Reflection::FieldAccessor* fields;
			int fields_count = meta.getFieldsList(fields);
			Reflection::FieldAccessor field_accessor;

			for (int i = 0; i < fields_count; i++) {
				Reflection::FieldAccessor facc = fields[i];
				if (facc.getFieldName() == "resource_path") {
					continue;
				}
				facc.set(&m, facc.get(&p)); // 这么写，真没问题吗。。
				// 然而这里并不知道填什么
				// 其实可以序列化再反序列化
				L_PRINT("changing", facc.getFieldName());
			}
			L_JSON(p);
			L_JSON(m);
			return p == m;
			}
			else {
				L_PRINT("meta invalid");
				return -1;
			}

		}

		int test_assign_using_serializer() {
			Vector2 p(1, 2);
			Vector2 m(3, 4);
			using Reflection::TypeMeta;
			m = Serializer::read(Serializer::write(p), m);
			L_JSON(p);
			L_JSON(m);
			return p == m;

		}
	}

}
