#pragma once
#ifndef TEST_SCENE_H
#define TEST_SCENE_H
#include "core/scene/object/gobject.h"
#include "core/os/thread.h"
#include "core/os/os.h"

namespace lain {
	

	namespace test {
		void draw_tree(GObject* root);
		void test_scene();
	}
}
#endif