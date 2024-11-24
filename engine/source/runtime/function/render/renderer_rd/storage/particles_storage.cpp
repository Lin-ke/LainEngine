#include "particles_storage.h"
using namespace lain::RendererRD;
ParticlesStorage* ParticlesStorage::singleton = nullptr;

lain::RendererRD::ParticlesStorage::ParticlesStorage()
{
    singleton = this;
}

lain::RendererRD::ParticlesStorage::~ParticlesStorage()
{
    singleton = nullptr;
}
