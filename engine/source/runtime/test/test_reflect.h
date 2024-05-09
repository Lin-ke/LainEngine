#ifndef EDITOR_TEST_H
#define EDITOR_TEST_H

#include "base.h"
#include "core/meta/reflection/reflection.h"
#include "_generated/serializer/all_serializer.h"
#include "test/test_marcos.h"
namespace lain {
	namespace test {
		class TestMeta {

		static int test_StringName_reflect();
		static int test_accessor();
		static int test_Vector3_reflect();
		static int test_assign_using_serializer();
		static int test_Basis();
		static int test_Matrix3x3();
		public:
			static void test_reflect() {
				test_accessor();
				test_Vector3_reflect();
				test_assign_using_serializer();
				test_StringName_reflect();
				test_Basis();
				test_Matrix3x3();
			}
		};

		template<typename T>
		T test_meta(T& instance, const char* tname, const char* fieldname, void* value) {
			auto meta = Reflection::TypeMeta::newMetaFromName(tname);
			if (!meta.isValid()) {
				L_PERROR("meta is not valid");
				return instance;
			}
			L_PRINT("before");
			L_JSON(instance);

			auto field_accessor = meta.getFieldByName(fieldname);
			if (!field_accessor.isValid()) {
				L_PINFO("field_accessor is not valid");
				return instance;
				// set
			}
			field_accessor.set((void*)&instance, value);
			L_PRINT("after");
			L_JSON(instance);

			return instance;
		}
	}
}
#endif // !EDITOR_TEST_H
