#include "refcount.h"
bool RefCounted::init_ref() {
}
bool RefCounted::reference() {
	refcount.refval();
}
bool RefCounted::unreference() {

}
int RefCounted::get_reference_count() const {

}

RefCounted::RefCounted() {

}