#ifndef SHADER_TYPES_H
#define SHADER_TYPES_H
#include "core/templates/rb_map.h"
#include "function/render/rendering_system/rendering_system.h"
#include "shader_language.h"
namespace lain{

class ShaderTypes {
	struct Type {
		HashMap<StringName, ShaderLanguage::FunctionInfo> functions;
		Vector<ShaderLanguage::ModeInfo> modes;
	};

	HashMap<RS::ShaderMode, Type> shader_modes;

	static ShaderTypes *singleton;

	HashSet<String> shader_types;
	List<String> shader_types_list;

public:
	static ShaderTypes *get_singleton() { return singleton; }

	const HashMap<StringName, ShaderLanguage::FunctionInfo> &get_functions(RS::ShaderMode p_mode) const;
	const Vector<ShaderLanguage::ModeInfo> &get_modes(RS::ShaderMode p_mode) const;
	const HashSet<String> &get_types() const;
	const List<String> &get_types_list() const;

	ShaderTypes();
};
}
#endif // SHADER_TYPES_H
