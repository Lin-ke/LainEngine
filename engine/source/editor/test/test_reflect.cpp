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
	}

}
