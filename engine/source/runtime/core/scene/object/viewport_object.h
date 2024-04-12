#pragma once
#ifndef VIEWPORT_OBJECT_H
#define VIEWPORT_OBJECT_H
#include "core/scene/object/gobject.h"
namespace lain{
	class ViewPort :public GObject {

        virtual void _physics_interpolated_changed() override {}

        virtual void add_child_notify(GObject* p_child)override {}
        virtual void remove_child_notify(GObject* p_child) override {}
        virtual void move_child_notify(GObject* p_child)override {}
        virtual void owner_changed_notify() override{}

	};
}
#endif
