#ifdef L_PLATFORM_WINDOWS
#ifdef L_DEBUG
#define refcount(x) (*(reinterpret_cast<s_u32*> (const_cast<int*>(x.ptr())) - 2)).get()
#endif
#include <base.h>
#include <core/meta/meta_example.cpp>
#include <core/engine/engine.h>
#include <core/math/vector2.h>
#include <core/error/error_macros.h>
#include <core/os/memory.h>
#include <core/templates/cowdata.h>
#include <core/templates/vector.h>
#include <core/templates/safe_refcount.h>
int YminusA(int a, lain::Vector2& obj);
void TryERR_FAIL_INDEX();
int main(int argc, char** argv) {
	lain::Log::Init();
	L_CORE_ERROR("Hello! Var={0}", 5);
	L_INFO("Hello! Var={0}", 5);
	lain::Engine* engine = new lain::Engine();
	engine->startEngine("");
	engine->initialize();
	//lain::CowData<std::string>();
	/*std::vector<int> a;
	a.push_back(123);*/
	Vector<int> v({ 1,2,3 });
	v.push_back(123);
	v.push_back(5);

	Vector<int> v2(v);
	auto v3 = v;

	s_u32* p = reinterpret_cast<s_u32*> (const_cast<int*>(v.ptr()))-2;
	
	/*v.push_back(123);
	for (int i = 0; i < v.size(); i++) {
		std::cout << v[i] << std::endl;
	}*/

	L_PRINT(refcount(v), refcount(v2), refcount(v3), Memory::get_mem_max_usage(), "used", Memory::get_mem_usage());
	v3.insert(2, 12);
	s_u32* p1 = reinterpret_cast<s_u32*> (const_cast<int*>(v3.ptr())) - 2;
	for (int i = 0; i < v3.size(); i++) {
		L_INFO(v3[i]);
	}
}
void TryERR_FAIL_INDEX() {
	ERR_FAIL_INDEX(-1, 3);
}
int YminusA(int a, lain::Vector2& obj) {
	auto meta = lain::Reflection::TypeMeta::newMetaFromName("Vector2");
	if (!meta.isValid()) {
		L_ERROR("meta is not valid");
	}
	auto field_accessor = meta.getFieldByName("x");
	float b = 16;
	if (field_accessor.isValid()) {
		L_INFO("valid");
		// set
		field_accessor.set((void*)&obj, (float *)&b);
		L_JSON(obj);
	}
	else {
		L_ERROR("Field is not valid");
	}
	return 0;


}

#endif
// in debug mode, each allocation is padded with 16 bytes