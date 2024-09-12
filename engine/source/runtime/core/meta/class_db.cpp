#include "class_db.h"
#include "reflection/reflection.h"
namespace lain {

	MethodDefinition D_METHODP(const char* p_name, const char* const** p_args, uint32_t p_argcount) {
		MethodDefinition md;
		md.name = StaticCString::create(p_name);
		md.args.resize(p_argcount);
		for (uint32_t i = 0; i < p_argcount; i++) {
			md.args.write[i] = StaticCString::create(*p_args[i]);
		}
		return md;
	}

	HashMap<StringName, StringName> ClassDB::resource_base_extensions;
	HashMap<StringName, ClassInfo> ClassDB::classes;


	void ClassDB::add_resource_base_extension(const StringName& p_extension, const StringName& p_class) {
		if (resource_base_extensions.has(p_extension)) {
			return;
		}

		resource_base_extensions[p_extension] = p_class;
	}

	void ClassDB::get_resource_base_extensions(List<String>* p_extensions) {
		for (const KeyValue<StringName, StringName>& E : resource_base_extensions) {
			p_extensions->push_back(E.key);
		}
	}
	// class数据库
	Object* ClassDB::instantiate(const StringName& cp_class, bool p_require_real_class) {
		return static_cast<Object*>(Reflection::TypeMeta::memnewByName(SCSTR(cp_class)));
	}

	bool ClassDB::can_instantiate(const StringName& cp_class) {
		Reflection::TypeMeta meta = Reflection::TypeMeta::newMetaFromName(SCSTR(cp_class));
		return meta.isValid();
	}
	// 这个json格式不一定正确
	void* ClassDB::instantiate_with_name_json(const StringName& cp_class, const Json& p_json ){
		std::string err;
		Reflection::ReflectionInstance instance = Reflection::TypeMeta::newFromNameAndJson(SCSTR(cp_class),p_json);
		if(instance.m_instance == nullptr){
			return nullptr;
		}
		else {
			return instance.m_instance;
		}
	}
	void* ClassDB::instantiate_with_json(const Json& p_json ){
		std::string err;
		Reflection::ReflectionInstance instance = Reflection::ReflectionInstance(p_json);
		if(instance.m_instance == nullptr){
			return nullptr;
		}
		else {
			return instance.m_instance;
		}
		
	}



	void ClassDB::_add_class2(const StringName& p_class, const StringName& p_inherits) {
		//OBJTYPE_WLOCK;

		const StringName& name = p_class;

		ERR_FAIL_COND_MSG(classes.has(name), "Class '" + String(p_class) + "' already exists.");

		classes[name] = ClassInfo();
		ClassInfo& ti = classes[name];
		ti.name = name;
		ti.inherits = p_inherits;
		//ti.api = current_api;

		if (ti.inherits) {
			ERR_FAIL_COND(!classes.has(ti.inherits)); //it MUST be registered.
			ti.inherits_ptr = &classes[ti.inherits];

		}
		else {
			ti.inherits_ptr = nullptr;
		}
	}
	/*StringName ClassDB::get_parent_class(const StringName& p_class) {
		
	}*/
	



}