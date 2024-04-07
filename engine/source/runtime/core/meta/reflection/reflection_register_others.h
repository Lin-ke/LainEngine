#pragma once
#ifndef REGISTER_OTHER_TYPES_H
#define REGISTER_OTHER_TYPES_H

#include "core/meta/others/stringname.reflection.h"
namespace lain {


	L_INLINE void register_other_meta() {
		Reflection::TypeWrappersRegister::StringName();
	}


}

#endif // !REGISTER_OTHER_TYPES_H
