#include "rect2i.h"

#include "core/math/rect2.h"
#include "core/string/ustring.h"
namespace lain {

Rect2i::operator String() const {
	return "[P: " + position.operator String() + ", S: " + size + "]";
}

Rect2i::operator Rect2() const {
	return Rect2(position, size);
}
}
