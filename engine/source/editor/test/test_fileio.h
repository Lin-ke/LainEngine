#pragma once
#ifndef TEST_FILEIO_H
#define TEST_FILEIO_H
#include "core/meta/serializer/serializer.h"
#include "core/io/file_access.h"
namespace lain{
	namespace test {
		int test_fileio() {
			auto p = Vector2(1, 4);
			Ref<FileAccess> fileacc = FileAccess::open("res://1.txt", FileAccess::ModeFlags::WRITE_READ);
			L_STRPRINT(fileacc->get_path(), fileacc->get_path_absolute());
			fileacc->store_string(lain::Serializer::write(p).dump());
			L_STRPRINT(ProjectSettings::GetSingleton()->GetProjectDataPath());
			L_STRPRINT(ProjectSettings::GetSingleton()->GetResourcePath());

			return 0;
		}
	}
}
#endif