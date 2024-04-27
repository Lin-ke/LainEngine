#pragma once
#ifndef TEST_PROCESS_H
#define TEST_PROCESS_H
namespace lain {
	namespace test {
		void test_process() {
			Ref<PackedScene> s3 = ResourceLoader::load("3.tscn", "PackedScene");
			auto new_scene = s3->instantiate();
			
		}
	}
}
#endif
