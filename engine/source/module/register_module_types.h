
#ifndef REGISTER_MODULE_TYPES_H
#define REGISTER_MODULE_TYPES_H

// #include "core/extension/gdextension_interface.h"
namespace lain{

enum ModuleInitializationLevel {
	MODULE_INITIALIZATION_LEVEL_CORE,
	MODULE_INITIALIZATION_LEVEL_SERVERS,
	MODULE_INITIALIZATION_LEVEL_SCENE,
	MODULE_INITIALIZATION_LEVEL_EDITOR 
};

void initialize_modules(ModuleInitializationLevel p_level);
void uninitialize_modules(ModuleInitializationLevel p_level);

}
#endif // REGISTER_MODULE_TYPES_H
