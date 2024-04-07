#include "core/register_core_types.h"
#include "core/string/string_name.h"
#include "core/thread/worker_thread_pool.h"
#include "core/io/resource_uid.h"
#include "core/io/resource_loader.h"
#include "core/object/objectdb.h"

// scene
#include "resource/io/resource_format_text.h"
namespace lain {
	static WorkerThreadPool* worker_thread_pool = nullptr;
	static ResourceUID* resource_uid = nullptr;
	// resource loaders
	//static Ref<ResourceFormatSaverBinary> resource_saver_binary;
	//static Ref<ResourceFormatLoaderBinary> resource_loader_binary;
	static Ref<ResourceFormatImporter> resource_format_importer;

	//static Ref<ResourceFormatImporterSaver> resource_format_importer_saver;
	//static Ref<ResourceFormatLoaderImage> resource_format_image;
	//static Ref<TranslationLoaderPO> resource_format_po;
	//static Ref<ResourceFormatSaverCrypto> resource_format_saver_crypto;
	//static Ref<ResourceFormatLoaderCrypto> resource_format_loader_crypto;
	//static Ref<GDExtensionResourceLoader> resource_loader_gdextension;
	//static Ref<ResourceFormatSaverJSON> resource_saver_json;
	//static Ref<ResourceFormatLoaderJSON> resource_loader_json;

	// godot: register scene types
	static Ref<ResourceFormatLoaderText> resource_format_loader_text;

	void register_core_types() {

		///class setup
		ObjectDB::setup();
		StringName::setup();
		ResourceLoader::initialize();

		worker_thread_pool = memnew(WorkerThreadPool);

		/// loader
		//resource_format_po.instantiate();
		//ResourceLoader::add_resource_format_loader(resource_format_po);

		//resource_saver_binary.instantiate();
		//ResourceSaver::add_resource_format_saver(resource_saver_binary);
		//resource_loader_binary.instantiate();
		//ResourceLoader::add_resource_format_loader(resource_loader_binary);

		resource_format_importer.instantiate();
		ResourceLoader::add_resource_format_loader(resource_format_importer);

		//resource_format_importer_saver.instantiate();
		//ResourceSaver::add_resource_format_saver(resource_format_importer_saver);

		//resource_format_image.instantiate();
		//ResourceLoader::add_resource_format_loader(resource_format_image);

		resource_format_loader_text.instantiate();
		ResourceLoader::add_resource_format_loader(resource_format_loader_text);

	}
}

