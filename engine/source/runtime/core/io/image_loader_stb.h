#pragma once
#ifndef IMAGE_LOADER_STB_H
#define IMAGE_LOADER_STB_H
#include "image_loader.h"
namespace lain {
class StbLoader : public ResourceLoaderImage {
  friend class ResourceFormatLoaderImage;

 public:
  virtual void get_recognized_extensions(List<String>* p_list) const override {
    p_list->push_back("jpg");
    p_list->push_back("jpeg");
    p_list->push_back("bmp");
  }
  virtual Error load_image(Ref<Image> p_image, Ref<FileAccess> f, BitField<ImageLoader::LoaderFlags> p_flags, float p_scale) override;

  static StbLoader* singleton;
  StbLoader() { singleton = this; }

 private:
  Image::Format _get_format(String p_ext, int p_channels);
};

}  // namespace lain

#endif  // !IMAGE_LOADER_STB_H
