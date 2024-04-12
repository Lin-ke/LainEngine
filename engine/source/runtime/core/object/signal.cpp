#include "signal.h"
#include "core/string/string_name.h"
#include "core/object/objectdb.h"

namespace lain {
	_FORCE_INLINE_ bool Signal::is_null() const {
		return object.is_null() && name == StringName();
	}
	Object* Signal::get_object() const {
		return ObjectDB::get_instance(object);
	}

	bool Signal::operator==(const Signal& p_signal) const {
		return object == p_signal.object && name == p_signal.name;
	}

	bool Signal::operator!=(const Signal& p_signal) const {
		return object != p_signal.object || name != p_signal.name;
	}

	bool Signal::operator<(const Signal& p_signal) const {
		if (object == p_signal.object) {
			return name < p_signal.name;
		}
		else {
			return object < p_signal.object;
		}
	}

}