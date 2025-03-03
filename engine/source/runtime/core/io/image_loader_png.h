/**************************************************************************/
/*  image_loader_png.h                                                    */
/**************************************************************************/

#ifndef IMAGE_LOADER_PNG_H
#define IMAGE_LOADER_PNG_H

#include "core/io/image_loader.h"
namespace lain{

class ImageLoaderPNG : public ResourceLoaderImage {
private:
	static Vector<uint8_t> lossless_pack_png(const Ref<Image> &p_image);
	static Ref<Image> lossless_unpack_png(const Vector<uint8_t> &p_data);
	static Ref<Image> unpack_mem_png(const uint8_t *p_png, int p_size);
	static Ref<Image> load_mem_png(const uint8_t *p_png, int p_size);

public:
	virtual Error load_image(Ref<Image> p_image, Ref<FileAccess> f, BitField<ImageLoader::LoaderFlags> p_flags, float p_scale) override;
	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	ImageLoaderPNG();
};
}

#endif // IMAGE_LOADER_PNG_H
