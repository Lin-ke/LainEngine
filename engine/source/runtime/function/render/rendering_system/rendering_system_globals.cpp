#include "rendering_system_globals.h"
using namespace lain;
bool RSG::threaded = false;
RendererMeshStorage* RSG::mesh_storage = nullptr;
RendererMaterialStorage* RSG::material_storage = nullptr;
RendererUtilities* RSG::utilities = nullptr;
RendererLightStorage* RSG::light_storage = nullptr;
RendererParticlesStorage* RSG::particles_storage = nullptr;
RendererGI* RSG::gi = nullptr;
RendererFog* RSG::fog = nullptr;
RendererTextureStorage* RSG::texture_storage = nullptr;
RendererCameraAttributes* RSG::camera_attributes = nullptr;
RendererCompositor* RSG::rasterizer = nullptr;
RendererViewport* RSG::viewport = nullptr;
RenderingMethod* RSG::scene = nullptr;