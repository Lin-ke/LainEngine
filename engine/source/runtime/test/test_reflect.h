#ifndef EDITOR_TEST_H
#define EDITOR_TEST_H

#include "base.h"
#include "core/meta/reflection/reflection.h"
#include "_generated/serializer/all_serializer.h"
namespace lain {
	namespace test {
		int test_StringName_reflect();
		int test_Vector3_reflect();
		int test_assign_using_serializer();

		int test_accessor();

		template<typename T>
		void test_meta(T& instance, const char* tname, const char* fieldname, void* value) {
			auto meta = Reflection::TypeMeta::newMetaFromName(tname);
			if (!meta.isValid()) {
				L_ERROR("meta is not valid");
				return;
			}
			L_PRINT("before");
			L_JSON(instance);

			auto field_accessor = meta.getFieldByName(fieldname);
			if (!field_accessor.isValid()) {
				L_INFO("field_accessor is not valid");
				return;
				// set
			}
			field_accessor.set((void*)&instance, value);
			L_PRINT("after");
			L_JSON(instance);

			return;
		}
	}
}
#endif // !EDITOR_TEST_H
