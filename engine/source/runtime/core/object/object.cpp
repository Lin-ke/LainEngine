#include "object.h"
#include "core/object/objectdb.h"
namespace lain {
	void Object::_construct_object(bool p_is_ref) {
		m_type_is_reference = p_is_ref;
		m_instance_id = ObjectDB::add_instance(this);
	}
	// reverse：先调父类再调子类
	void Object::notification(int p_notification, bool p_reversed) {
		if (p_reversed) {
			/*if (script_instance) {
				script_instance->notification(p_notification, p_reversed);
			}*/
		}
		else {
			_notificationv(p_notification, p_reversed);
		}
		if (p_reversed) {
			_notificationv(p_notification, p_reversed);
		}
		/*else {
			if (script_instance) {
				script_instance->notification(p_notification, p_reversed);
			}
		}*/
	}

}