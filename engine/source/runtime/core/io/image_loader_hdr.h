/**************************************************************************/
/*  image_loader_hdr.h                                                    */
/**************************************************************************/

#ifndef IMAGE_LOADER_HDR_H
#define IMAGE_LOADER_HDR_H

#include "core/io/image_loader.h"
namespace lain{

class ImageLoaderHDR : public ResourceLoaderImage {
public:
	virtual Error load_image(Ref<Image> p_image, Ref<FileAccess> f, BitField<ImageLoader::LoaderFlags> p_flags, float p_scale)override ; 
	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	ImageLoaderHDR();
};
}

#endif // IMAGE_LOADER_HDR_H
