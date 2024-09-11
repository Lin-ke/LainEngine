#ifndef GLSLANG_REGISTER_TYPES_H
#define GLSLANG_REGISTER_TYPES_H
#include "module/register_module_types.h"
#define MODULE_GLSLANG_HAS_PREREGISTER

namespace lain {


void initialize_glslang_module(ModuleInitializationLevel p_level);
void uninitialize_glslang_module(ModuleInitializationLevel p_level);
}
#endif // GLSLANG_REGISTER_TYPES_H