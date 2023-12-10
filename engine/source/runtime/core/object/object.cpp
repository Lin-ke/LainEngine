#include "object.h"
namespace lain {
	void Object::_construct_object(bool p_is_ref) {
		m_type_is_reference = p_is_ref;
	}
}