#include "test_reflect.h"
#include "core/string/string_name.h"
#include "core/math/basis.h"
#include "core/math/matrix3.h"
#include "_generated/typename/all_typename.h"
namespace lain {
	namespace test {

		int TestMeta::test_StringName_reflect(){
			StringName p{ "hello" };
			String value = "hi";
			test_meta(p, "StringName", "name", &value);
			return 0; 
		}
		int TestMeta::test_Vector3_reflect() {
			Vector3 p{ 1,2,3 };
			float value = 10;
			auto _ = test_meta(p, "Vector3", "x", &value);
			return 0;
		}

		int TestMeta::test_Basis() {
			Vector3 p1{ 1,2,3 };
			Vector3 p2{ 1,2,3 };
			Vector3 p3{ 1,2,3 };
			Vector3 p4{ 1,2,3 };
			Vector3 p5{ 4,5,6 };
			Vector3 p6{ 7,8,9 };
			Vector3 p[3]{ p4,p5,p6 };
			auto bss = Basis(p1,p2,p3);
			auto _ = test_meta(bss, "Basis", "rows", &p);
			
			return 0;
		}
		
		int TestMeta::test_accessor() {
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
		int TestMeta::test_Matrix3x3() {
			Matrix3x3 p = { 1,2,3,4,5,6,7,8,9 };
			Matrix3x3 p1 = { 9,8,7,6,5,4,3,2,1 };
			auto _ = test_meta(p, "Matrix3x3", "mat", &p1);
			return 0;
		}
		/*TEST_CASE("testing Matrix3x3") {
			Matrix3x3 p = { 1,2,3,4,5,6,7,8,9 };
			Matrix3x3 p1 = { 9,8,7,6,5,4,3,2,1 };
			p = test_meta(p, "Matrix3x3", "mat", &p1);
			CHECK(p == p1);
		}*/
		

		int TestMeta::test_assign_using_serializer() {
			Vector2 p(1, 2);
			Vector2 m(3, 4);
			using Reflection::TypeMeta;
			m = Serializer::read(Serializer::write(p), m);
			L_JSON(p);
			L_JSON(m);
			L_PRINT(Reflection::to_string<Vector2>());
			L_PRINT(Reflection::get_class(p));

			return p == m;
		}

		
	}

}
