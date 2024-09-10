#pragma once
#include "core/meta/serializer/serializer.h"
#include "core/config/project_settings.h"
#include "core/io/image.h"
#include "core/io/resource_loader.h"
namespace lain {
	namespace test {
		void test_image_load_from_path(String p_path);
		void test_image_io() {
			String res_path = "res://pic.jpg";
			String relative_path = "./pic.jpg";
			String abs_path = "D:\\LainEngine\\game\\pic.jpg";
			test_image_load_from_path(res_path);
			test_image_load_from_path(relative_path);
			test_image_load_from_path(abs_path);

		}
		void test_image_load_from_path(String p_path) {
			Ref<Image> p = ResourceLoader::load(p_path);
			L_PRINT(p->get_reference_count());
			if (p.is_null()) {
				L_PERROR("load error!!");
			}
			auto size = p->get_size();
			L_JSON(size);

			Ref<Image> p1 = Image::load_from_file(p_path);
			L_JSON(p1->get_data() == p->get_data());
		}


	}
}