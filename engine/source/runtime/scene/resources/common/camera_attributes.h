#ifndef CAMERA_ATTRIBUTES_H
#define CAMERA_ATTRIBUTES_H

#include "core/io/resource.h"
#include "core/io/rid.h"
namespace lain{

class CameraAttributes : public Resource {
  LCLASS(CameraAttributes, Resource);
  private:
	RID camera_attributes;
  public:
  virtual RID GetRID() const { return camera_attributes; }
  CameraAttributes();
  ~CameraAttributes();
};
}


#endif 