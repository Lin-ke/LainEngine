#include "core/string/string_name.h"
#include "core/thread/worker_thread_pool.h"
#include "core/io/resource_uid.h"
#include "core/io/resource_loader.h"
#include "core/object/objectdb.h"
namespace lain {
	static WorkerThreadPool* worker_thread_pool = nullptr;
	static ResourceUID* resource_uid = nullptr;

void register_core_types() {

	///class setup
	ObjectDB::setup();
	StringName::setup();
	ResourceLoader::initialize();

	worker_thread_pool = memnew(WorkerThreadPool);

}
}

