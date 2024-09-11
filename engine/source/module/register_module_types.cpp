#include "register_module_types.h"
#include "module/glslang/glslang.h"
namespace lain{

// 这个也用gen的，但是目前比较少我就手写了
void initialize_modules(ModuleInitializationLevel p_level) {
    initialize_glslang_module(p_level);
}
void uninitialize_modules(ModuleInitializationLevel p_level) {
    uninitialize_glslang_module(p_level);
}
}
