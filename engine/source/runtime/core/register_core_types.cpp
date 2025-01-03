#include "core/register_core_types.h"
#include "core/string/string_name.h"
#include "core/thread/worker_thread_pool.h"
#include "core/io/resource_loader.h"
#include "core/io/resource_uid.h"
#include "core/object/objectdb.h"
#include "core/meta/serializer/serializer.h"
#include "core/io/image_loader_stb.h"
#include "core/io/image_loader_hdr.h"
#include "core/io/image_loader_png.h"

// scene
#include "core/scene/scene_stringnames.h"
#include "core/scene/object/gobject.h"

#include "core/meta/class_db.h"
#include "core/math/triangle_mesh.h"
#include "core/scene/component/component.h"

#include "core/scene/object/testnode.h"
namespace lain {
	static WorkerThreadPool* worker_thread_pool = nullptr;
	static ResourceUID* resource_uid = nullptr;
	//static ResourceLoader* _resource_loader = nullptr; // core_bind?
	// resource loaders
	//static Ref<ResourceFormatSaverBinary> resource_saver_binary;
	//static Ref<ResourceFormatLoaderBinary> resource_loader_binary;
	//static Ref<ResourceFormatImporter> resource_format_importer;

	//static Ref<ResourceFormatImporterSaver> resource_format_importer_saver;
	//static Ref<ResourceFormatLoaderImage> resource_format_image;
	//static Ref<TranslationLoaderPO> resource_format_po;
	//static Ref<ResourceFormatSaverCrypto> resource_format_saver_crypto;
	//static Ref<ResourceFormatLoaderCrypto> resource_format_loader_crypto;
	//static Ref<GDExtensionResourceLoader> resource_loader_gdextension;
	//static Ref<ResourceFormatSaverJSON> resource_saver_json;
	//static Ref<ResourceFormatLoaderJSON> resource_loader_json;

	// godot: register scene types
	static Ref<ResourceFormatLoaderImage> resource_format_loader_image;


	static Ref<StbLoader> stbloader ;
	static Ref<ImageLoaderPNG> pngloader;
	static Ref<ImageLoaderHDR> hdrloader;

	void register_core_types() {

		///class setup
		ObjectDB::setup();
		StringName::setup();
		ResourceLoader::initialize();
		//ResourceSaver::initialize();

		SceneStringNames::create(); // register stringnames

		worker_thread_pool = memnew(WorkerThreadPool);

		/// loader
		//resource_format_po.instantiate();
		//ResourceLoader::add_resource_format_loader(resource_format_po);

		//resource_saver_binary.instantiate();
		//ResourceSaver::add_resource_format_saver(resource_saver_binary);
		//resource_loader_binary.instantiate();
		//ResourceLoader::add_resource_format_loader(resource_loader_binary);

		/*resource_format_importer.instantiate();
		ResourceLoader::add_resource_format_loader(resource_format_importer);*/

		//resource_format_importer_saver.instantiate();
		//ResourceSaver::add_resource_format_saver(resource_format_importer_saver);

		//resource_format_image.instantiate();
		//ResourceLoader::add_resource_format_loader(resource_format_image);
		resource_format_loader_image.instantiate();
		ResourceLoader::add_resource_format_loader(resource_format_loader_image);

		// built in loaders
			stbloader.instantiate();
			ResourceFormatLoaderImage::get_singleton()->add_image_format_loader(stbloader);
			pngloader.instantiate();
			ResourceFormatLoaderImage::get_singleton()->add_image_format_loader(pngloader);
			hdrloader.instantiate();
			ResourceFormatLoaderImage::get_singleton()->add_image_format_loader(hdrloader);
		GObject::init_gobj_hrcr();


		GDREGISTER_CLASS(Object);
	GDREGISTER_CLASS(RefCounted);
	GDREGISTER_CLASS(WeakRef);
	GDREGISTER_CLASS(Resource);
	GDREGISTER_CLASS(Image);
	GDREGISTER_CLASS(TriangleMesh);
		GDREGISTER_CLASS(TickObject);
		GDREGISTER_CLASS(GObject);
		GDREGISTER_CLASS(Component);
		GDREGISTER_CLASS(TestNode);
		GDREGISTER_CLASS(TestComponent);

	GDREGISTER_CLASS(PackedScene);
	// GDREGISTER_ABSTRACT_CLASS(SceneState);



	}
}

