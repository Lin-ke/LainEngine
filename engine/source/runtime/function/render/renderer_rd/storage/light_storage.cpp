#include "light_storage.h"
#include "../renderer_scene_render_rd.h"
#include "texture_storage.h"
using namespace lain::RendererRD;
using namespace lain;
LightStorage* LightStorage::singleton = nullptr;

LightStorage::~LightStorage(){
  singleton = nullptr;
  	if (directional_light_buffer.is_valid()) {
		RD::get_singleton()->free(directional_light_buffer);
		directional_light_buffer = RID();
	}

	if (omni_light_buffer.is_valid()) {
		RD::get_singleton()->free(omni_light_buffer);
		omni_light_buffer = RID();
	}

	if (spot_light_buffer.is_valid()) {
		RD::get_singleton()->free(spot_light_buffer);
		spot_light_buffer = RID();
	}

	if (directional_lights != nullptr) {
		memdelete_arr(directional_lights);
		directional_lights = nullptr;
	}

	if (omni_lights != nullptr) {
		memdelete_arr(omni_lights);
		omni_lights = nullptr;
	}

	if (spot_lights != nullptr) {
		memdelete_arr(spot_lights);
		spot_lights = nullptr;
	}

	if (omni_light_sort != nullptr) {
		memdelete_arr(omni_light_sort);
		omni_light_sort = nullptr;
	}

	if (spot_light_sort != nullptr) {
		memdelete_arr(spot_light_sort);
		spot_light_sort = nullptr;
	}
	for (const KeyValue<int, ShadowCubemap> &E : shadow_cubemaps) {
		RD::get_singleton()->free(E.value.cubemap);
	}
}

bool lain::RendererRD::LightStorage::owns_reflection_probe(RID p_rid) {
  return false;
}

bool lain::RendererRD::LightStorage::owns_light(RID p_rid) {
  return light_owner.owns(p_rid);
}

bool lain::RendererRD::LightStorage::owns_lightmap(RID p_rid) {
  return false;
}

bool lain::RendererRD::LightStorage::free(RID p_rid) {
  if(owns_light(p_rid)){
    light_free(p_rid);
    return true;
  }
  return false;
}

RS::LightDirectionalSkyMode LightStorage::light_directional_get_sky_mode(RID p_light) const {
	const Light *light = light_owner.get_or_null(p_light);
	ERR_FAIL_NULL_V(light, RS::LIGHT_DIRECTIONAL_SKY_MODE_LIGHT_AND_SKY);

	return light->directional_sky_mode;
}

RID lain::RendererRD::LightStorage::shadow_atlas_create()
{
    return shadow_atlas_owner.make_rid(ShadowAtlas());
}
void LightStorage::shadow_atlas_free(RID p_atlas) {
  shadow_atlas_set_size(p_atlas, 0);
  shadow_atlas_owner.free(p_atlas);
}

void lain::RendererRD::LightStorage::shadow_atlas_set_size(RID p_atlas, int p_size, bool p_16_bits) {
  ShadowAtlas* shadow_atlas = shadow_atlas_owner.get_or_null(p_atlas);
  ERR_FAIL_NULL(shadow_atlas);
  ERR_FAIL_COND(p_size < 0);
  p_size = next_power_of_2(p_size);
  if (p_size == shadow_atlas->size && p_16_bits == shadow_atlas->use_16_bits) {
    return;
  }

  // erasing atlas
  if (shadow_atlas->depth.is_valid()) {
    RD::get_singleton()->free(shadow_atlas->depth);
    shadow_atlas->depth = RID();
  }
  for (int i = 0; i < 4; i++) {
    //clear subdivisions
    shadow_atlas->quadrants[i].shadows.clear();
    shadow_atlas->quadrants[i].shadows.resize(int64_t(Math::sqr(shadow_atlas->quadrants[i].subdivision)));
  }
  //erase shadow atlas reference from lights
  for (const KeyValue<RID, uint32_t>& E : shadow_atlas->shadow_owners) {
    LightInstance* li = light_instance_owner.get_or_null(E.key);
    ERR_CONTINUE(!li);
    li->shadow_atlases.erase(p_atlas);
  }

  //clear owners
  shadow_atlas->shadow_owners.clear();

  shadow_atlas->size = p_size;
  shadow_atlas->use_16_bits = p_16_bits;
}

void LightStorage::shadow_atlas_set_quadrant_subdivision(RID p_atlas, int p_quadrant, int p_subdivision) {
  ShadowAtlas* shadow_atlas = shadow_atlas_owner.get_or_null(p_atlas);
  ERR_FAIL_NULL(shadow_atlas);
  ERR_FAIL_INDEX(p_quadrant, 4);
  ERR_FAIL_INDEX(p_subdivision, 16384);  // 最多

  uint32_t subdiv = next_power_of_2(p_subdivision);
  if (subdiv & 0xaaaaaaaa) {  //sqrt(subdiv) must be integer
    subdiv <<= 1;
  }

  subdiv = int(Math::sqrt((float)subdiv));

  //obtain the number that will be x*x

  if (shadow_atlas->quadrants[p_quadrant].subdivision == subdiv) {
    return;
  }

  //erase all data from quadrant
  // 擦除 shadow_owners中对应light的shadow
  // 擦除
  for (int i = 0; i < shadow_atlas->quadrants[p_quadrant].shadows.size(); i++) {
    const ShadowAtlas::Quadrant::Shadow& shadow_to_erase = shadow_atlas->quadrants[p_quadrant].shadows[i];
    if (shadow_to_erase.owner.is_valid()) {
      shadow_atlas->shadow_owners.erase(shadow_to_erase.owner);
      LightInstance* li = light_instance_owner.get_or_null(shadow_to_erase.owner);
      ERR_CONTINUE(!li);
      // 在灯光的引用中删除此atlas
      li->shadow_atlases.erase(p_atlas);
    }
  }

  shadow_atlas->quadrants[p_quadrant].shadows.clear();
  shadow_atlas->quadrants[p_quadrant].shadows.resize(subdiv * subdiv);
  shadow_atlas->quadrants[p_quadrant].subdivision = subdiv;

  //cache the smallest subdiv (for faster allocation in light update)

  shadow_atlas->smallest_subdiv = 1 << 30;  // 一个比较大的数

  for (int i = 0; i < 4; i++) {
    if (shadow_atlas->quadrants[i].subdivision) {
      shadow_atlas->smallest_subdiv = MIN(shadow_atlas->smallest_subdiv, shadow_atlas->quadrants[i].subdivision);
    }
  }

  if (shadow_atlas->smallest_subdiv == 1 << 30) {
    shadow_atlas->smallest_subdiv = 0;
  }

  //resort the size orders, simple bublesort for 4 elements..

  int swaps = 0;
  do {
    swaps = 0;  // bubble stop

    for (int i = 0; i < 3; i++) {
      if (shadow_atlas->quadrants[shadow_atlas->size_order[i]].subdivision < shadow_atlas->quadrants[shadow_atlas->size_order[i + 1]].subdivision) {
        SWAP(shadow_atlas->size_order[i], shadow_atlas->size_order[i + 1]);
        swaps++;
      }
    }
  } while (swaps > 0);
}

bool LightStorage::shadow_atlas_update_light(RID p_atlas, RID p_light_instance, float p_coverage, uint64_t p_light_version) {
  ShadowAtlas* shadow_atlas = shadow_atlas_owner.get_or_null(p_atlas);
  ERR_FAIL_NULL_V(shadow_atlas, false);

  LightInstance* li = light_instance_owner.get_or_null(p_light_instance);
  ERR_FAIL_NULL_V(li, false);

  if (shadow_atlas->size == 0 || shadow_atlas->smallest_subdiv == 0) {
    return false;
  }

  uint32_t quad_size = shadow_atlas->size >> 1;
  int desired_fit = MIN(quad_size / shadow_atlas->smallest_subdiv, next_power_of_2(quad_size * p_coverage));

  int valid_quadrants[4];
  int valid_quadrant_count = 0;
  int best_size = -1;    //best size found
  int best_subdiv = -1;  //subdiv for the best size

  //find the quadrants this fits into, and the best possible size it can fit into
  // 感觉像是错误的逻辑，因为 samllest 是最小的，说明desired_fit是最大的，
  // 除非 desired_fit = next_power_of_2(quad_size * p_coverage)
  for (int i = 0; i < 4; i++) {
    int q = shadow_atlas->size_order[i];
    int sd = shadow_atlas->quadrants[q].subdivision;
    if (sd == 0) {
      continue;  //unused
    }

    int max_fit = quad_size / sd;  // 格子分辨率

    if (best_size != -1 && max_fit > best_size) {
      break;  //too large
    }

    valid_quadrants[valid_quadrant_count++] = q;
    best_subdiv = sd;

    if (max_fit >= desired_fit) {
      best_size = max_fit;
    }
  }

  ERR_FAIL_COND_V(valid_quadrant_count == 0, false);

  uint64_t tick = OS::GetSingleton()->GetTicksUsec();

  uint32_t old_key = SHADOW_INVALID;
  uint32_t old_quadrant = SHADOW_INVALID;
  uint32_t old_shadow = SHADOW_INVALID;
  int old_subdivision = -1;

  bool should_realloc = false;
  bool should_redraw = false;

  if (shadow_atlas->shadow_owners.has(p_light_instance)) {
    old_key = shadow_atlas->shadow_owners[p_light_instance];
    old_quadrant = (old_key >> QUADRANT_SHIFT) & 0x3;
    old_shadow = old_key & SHADOW_INDEX_MASK;
    // 为什么一个光源会导致整个shadow atlas的重新alloc?
    should_realloc = shadow_atlas->quadrants[old_quadrant].subdivision != (uint32_t)best_subdiv &&
                     (shadow_atlas->quadrants[old_quadrant].shadows[old_shadow].alloc_tick - tick > shadow_atlas_realloc_tolerance_msec);
    should_redraw = shadow_atlas->quadrants[old_quadrant].shadows[old_shadow].version != p_light_version;

    if (!should_realloc) {
      shadow_atlas->quadrants[old_quadrant].shadows.write[old_shadow].version = p_light_version;
      //already existing, see if it should redraw or it's just OK
      return should_redraw;
    }

    old_subdivision = shadow_atlas->quadrants[old_quadrant].subdivision;
  }

  bool is_omni = li->light_type == RS::LIGHT_OMNI;
  bool found_shadow = false;
  int new_quadrant = -1;
  int new_shadow = -1;

  if (is_omni) {
    found_shadow = _shadow_atlas_find_omni_shadows(shadow_atlas, valid_quadrants, valid_quadrant_count, old_subdivision, tick, new_quadrant, new_shadow);
  } else {
    found_shadow = _shadow_atlas_find_shadow(shadow_atlas, valid_quadrants, valid_quadrant_count, old_subdivision, tick, new_quadrant, new_shadow);
  }

  if (found_shadow) { // 直接替换
    if (old_quadrant != SHADOW_INVALID) {
      shadow_atlas->quadrants[old_quadrant].shadows.write[old_shadow].version = 0;
      shadow_atlas->quadrants[old_quadrant].shadows.write[old_shadow].owner = RID();

      if (old_key & OMNI_LIGHT_FLAG) {
        shadow_atlas->quadrants[old_quadrant].shadows.write[old_shadow + 1].version = 0;
        shadow_atlas->quadrants[old_quadrant].shadows.write[old_shadow + 1].owner = RID();
      }
    }

    uint32_t new_key = new_quadrant << QUADRANT_SHIFT;
    new_key |= new_shadow;

    ShadowAtlas::Quadrant::Shadow* sh = &shadow_atlas->quadrants[new_quadrant].shadows.write[new_shadow];
    _shadow_atlas_invalidate_shadow(sh, p_atlas, shadow_atlas, new_quadrant, new_shadow);

    sh->owner = p_light_instance;
    sh->alloc_tick = tick;
    sh->version = p_light_version;

    if (is_omni) {
      new_key |= OMNI_LIGHT_FLAG;

      int new_omni_shadow = new_shadow + 1;
      ShadowAtlas::Quadrant::Shadow* extra_sh = &shadow_atlas->quadrants[new_quadrant].shadows.write[new_omni_shadow];
      _shadow_atlas_invalidate_shadow(extra_sh, p_atlas, shadow_atlas, new_quadrant, new_omni_shadow);

      extra_sh->owner = p_light_instance;
      extra_sh->alloc_tick = tick;
      extra_sh->version = p_light_version;
    }

    li->shadow_atlases.insert(p_atlas);

    //update it in map
    shadow_atlas->shadow_owners[p_light_instance] = new_key;
    //make it dirty, as it should redraw anyway
    return true;
  }

  return should_redraw;
}
void lain::RendererRD::LightStorage::shadow_atlas_update(RID p_atlas) {
  	ShadowAtlas *shadow_atlas = shadow_atlas_owner.get_or_null(p_atlas);
	ERR_FAIL_NULL(shadow_atlas);

	_update_shadow_atlas(shadow_atlas);
}
void lain::RendererRD::LightStorage::directional_shadow_atlas_set_size(int p_size, bool p_16_bits) {
  p_size = nearest_power_of_2_templated(p_size);

	if (directional_shadow.size == p_size && directional_shadow.use_16_bits == p_16_bits) {
		return;
	}

	directional_shadow.size = p_size;
	directional_shadow.use_16_bits = p_16_bits;

	if (directional_shadow.depth.is_valid()) {
		RD::get_singleton()->free(directional_shadow.depth);
		directional_shadow.depth = RID();
		// RendererSceneRenderRD::get_singleton()->base_uniforms_changed(); // @todo
	}
}

void LightStorage::set_directional_shadow_count(int p_count) {
	directional_shadow.light_count = p_count;
	directional_shadow.current_light = 0;
}
  // 从总size 和count计算出 第index 个的大小 
static Rect2i _get_directional_shadow_rect(int p_size, int p_shadow_count, int p_shadow_index) {
	int split_h = 1;
	int split_v = 1;

	while (split_h * split_v < p_shadow_count) {
		if (split_h == split_v) {
			split_h <<= 1;
		} else {
			split_v <<= 1;
		}
	}

	Rect2i rect(0, 0, p_size, p_size);
	rect.size.width() /= split_h;
	rect.size.height() /= split_v;
  // index 按行排列
	rect.position.x = rect.size.width() * (p_shadow_index % split_h);
	rect.position.y = rect.size.height() * (p_shadow_index / split_h);

	return rect;
}


int LightStorage::get_directional_light_shadow_size(RID p_light_intance) {
	ERR_FAIL_COND_V(directional_shadow.light_count == 0, 0);
  // 从总阴影图的 size和count计算出 第index个的大小 
	Rect2i r = _get_directional_shadow_rect(directional_shadow.size, directional_shadow.light_count, 0);

	LightInstance *light_instance = light_instance_owner.get_or_null(p_light_intance);
	ERR_FAIL_NULL_V(light_instance, 0);

	switch (light_directional_get_shadow_mode(light_instance->light)) {
		case RS::LIGHT_DIRECTIONAL_SHADOW_ORTHOGONAL:
			break; //none
		case RS::LIGHT_DIRECTIONAL_SHADOW_PARALLEL_2_SPLITS:
			r.size.height() /= 2; // csm的阴影贴图分辨率相同
			break;
		case RS::LIGHT_DIRECTIONAL_SHADOW_PARALLEL_4_SPLITS:
			r.size /= 2;
			break;
	}

	return MAX(r.size.width(), r.size.height());
}


void lain::RendererRD::LightStorage::_light_initialize(RID p_light, RS::LightType p_type) {
  Light light;
  light.type = p_type;
  // 这里的参数与其他地方的反射机制不同
  // 默认的字面量是double还是float？
  light.param[RS::LIGHT_PARAM_ENERGY] = 1.0;
  light.param[RS::LIGHT_PARAM_INDIRECT_ENERGY] = 1.0;
  light.param[RS::LIGHT_PARAM_VOLUMETRIC_FOG_ENERGY] = 1.0;
  light.param[RS::LIGHT_PARAM_SPECULAR] = 0.5;
  light.param[RS::LIGHT_PARAM_RANGE] = 1.0;
  light.param[RS::LIGHT_PARAM_SIZE] = 0.0;
  light.param[RS::LIGHT_PARAM_ATTENUATION] = 1.0;
  light.param[RS::LIGHT_PARAM_SPOT_ANGLE] = 45;
  light.param[RS::LIGHT_PARAM_SPOT_ATTENUATION] = 1.0;
  light.param[RS::LIGHT_PARAM_SHADOW_MAX_DISTANCE] = 0;
  light.param[RS::LIGHT_PARAM_SHADOW_SPLIT_1_OFFSET] = 0.1;
  light.param[RS::LIGHT_PARAM_SHADOW_SPLIT_2_OFFSET] = 0.3;
  light.param[RS::LIGHT_PARAM_SHADOW_SPLIT_3_OFFSET] = 0.6;
  light.param[RS::LIGHT_PARAM_SHADOW_FADE_START] = 0.8;
  light.param[RS::LIGHT_PARAM_SHADOW_NORMAL_BIAS] = 1.0;
  light.param[RS::LIGHT_PARAM_SHADOW_BIAS] = 0.02;
  light.param[RS::LIGHT_PARAM_SHADOW_OPACITY] = 1.0;
  light.param[RS::LIGHT_PARAM_SHADOW_BLUR] = 0;
  light.param[RS::LIGHT_PARAM_SHADOW_PANCAKE_SIZE] = 20.0;
  light.param[RS::LIGHT_PARAM_TRANSMITTANCE_BIAS] = 0.05;
  light.param[RS::LIGHT_PARAM_INTENSITY] = p_type == RS::LIGHT_DIRECTIONAL ? 100000.0 : 1000.0;

  light_owner.initialize_rid(p_light, light);
}

void lain::RendererRD::LightStorage::_shadow_atlas_invalidate_shadow(ShadowAtlas::Quadrant::Shadow* p_shadow, RID p_atlas, ShadowAtlas* p_shadow_atlas, uint32_t p_quadrant,
                                                                     uint32_t p_shadow_idx) {
  if (p_shadow->owner.is_valid()) {
    LightInstance* sli = light_instance_owner.get_or_null(p_shadow->owner);
    uint32_t old_key = p_shadow_atlas->shadow_owners[p_shadow->owner];

    if (old_key & OMNI_LIGHT_FLAG) {
      uint32_t s = old_key & SHADOW_INDEX_MASK;
      uint32_t omni_shadow_idx = p_shadow_idx + (s == (uint32_t)p_shadow_idx ? 1 : -1); // 如果记录是index就在前面否则在后面
      ShadowAtlas::Quadrant::Shadow* omni_shadow = &p_shadow_atlas->quadrants[p_quadrant].shadows.write[omni_shadow_idx];
      omni_shadow->version = 0;
      omni_shadow->owner = RID();
    }

    p_shadow_atlas->shadow_owners.erase(p_shadow->owner);
    p_shadow->version = 0;
    p_shadow->owner = RID();
    sli->shadow_atlases.erase(p_atlas);
  }
}

bool LightStorage::_shadow_atlas_find_shadow(ShadowAtlas *shadow_atlas, int *p_in_quadrants, int p_quadrant_count, int p_current_subdiv, uint64_t p_tick, int &r_quadrant, int &r_shadow) {
	for (int i = p_quadrant_count - 1; i >= 0; i--) {
		int qidx = p_in_quadrants[i];

		if (shadow_atlas->quadrants[qidx].subdivision == (uint32_t)p_current_subdiv) {
			return false;
		}

		//look for an empty space
		int sc = shadow_atlas->quadrants[qidx].shadows.size();
		const ShadowAtlas::Quadrant::Shadow *sarr = shadow_atlas->quadrants[qidx].shadows.ptr();

		int found_free_idx = -1; //found a free one
		int found_used_idx = -1; //found existing one, must steal it
		uint64_t min_pass = 0; // pass of the existing one, try to use the least recently used one (LRU fashion)
    // 最近最少使用是最靠前的一个pass
    // 为了快点遍历都用的指针操作
		for (int j = 0; j < sc; j++) {
			if (!sarr[j].owner.is_valid()) {
				found_free_idx = j; // 如果无效说明直接找到了
				break;
			}

			LightInstance *sli = light_instance_owner.get_or_null(sarr[j].owner);
			ERR_CONTINUE(!sli);
      // 有效（被用过），但是已经超时
			if (sli->last_scene_pass != RendererSceneRenderRD::get_singleton()->get_scene_pass()) {
				//was just allocated, don't kill it so soon, wait a bit..
				if (p_tick - sarr[j].alloc_tick < shadow_atlas_realloc_tolerance_msec) {
					continue;
				}

				if (found_used_idx == -1 || sli->last_scene_pass < min_pass) {
					found_used_idx = j;
					min_pass = sli->last_scene_pass;
				}
			}
		}

		if (found_free_idx == -1 && found_used_idx == -1) {
			continue; //nothing found
		}

		if (found_free_idx == -1 && found_used_idx != -1) {
			found_free_idx = found_used_idx; // 没找到free就用
		}

		r_quadrant = qidx;
		r_shadow = found_free_idx;

		return true;
	}

	return false;
}

bool LightStorage::_shadow_atlas_find_omni_shadows(ShadowAtlas *shadow_atlas, int *p_in_quadrants, int p_quadrant_count, int p_current_subdiv, uint64_t p_tick, int &r_quadrant, int &r_shadow) {
	for (int i = p_quadrant_count - 1; i >= 0; i--) {
		int qidx = p_in_quadrants[i];

		if (shadow_atlas->quadrants[qidx].subdivision == (uint32_t)p_current_subdiv) {
			return false;
		}

		//look for an empty space
		int sc = shadow_atlas->quadrants[qidx].shadows.size();
		const ShadowAtlas::Quadrant::Shadow *sarr = shadow_atlas->quadrants[qidx].shadows.ptr();

		int found_idx = -1;
		uint64_t min_pass = 0; // sum of currently selected spots, try to get the least recently used pair

		for (int j = 0; j < sc - 1; j++) {
			uint64_t pass = 0;

			if (sarr[j].owner.is_valid()) {
				LightInstance *sli = light_instance_owner.get_or_null(sarr[j].owner);
				ERR_CONTINUE(!sli);

				if (sli->last_scene_pass == RendererSceneRenderRD::get_singleton()->get_scene_pass()) {
					continue;
				}

				//was just allocated, don't kill it so soon, wait a bit..
				if (p_tick - sarr[j].alloc_tick < shadow_atlas_realloc_tolerance_msec) {
					continue;
				}
				pass += sli->last_scene_pass;
			}

			if (sarr[j + 1].owner.is_valid()) {
				LightInstance *sli = light_instance_owner.get_or_null(sarr[j + 1].owner);
				ERR_CONTINUE(!sli);

				if (sli->last_scene_pass == RendererSceneRenderRD::get_singleton()->get_scene_pass()) {
					continue;
				}

				//was just allocated, don't kill it so soon, wait a bit..
				if (p_tick - sarr[j + 1].alloc_tick < shadow_atlas_realloc_tolerance_msec) {
					continue;
				}
				pass += sli->last_scene_pass;
			}

			if (found_idx == -1 || pass < min_pass) {
				found_idx = j;
				min_pass = pass;

				// we found two empty spots, no need to check the rest
				if (pass == 0) {
					break;
				}
			}
		}

		if (found_idx == -1) {
			continue; //nothing found
		}

		r_quadrant = qidx;
		r_shadow = found_idx;

		return true;
	}

	return false;
}

void lain::RendererRD::LightStorage::_update_shadow_atlas(ShadowAtlas* shadow_atlas) {
  if (shadow_atlas->size > 0 && shadow_atlas->depth.is_null()) {
		RD::TextureFormat tf;
		tf.format = shadow_atlas->use_16_bits ? RD::DATA_FORMAT_D16_UNORM : RD::DATA_FORMAT_D32_SFLOAT;
		tf.width = shadow_atlas->size; // 整个大纹理
		tf.height = shadow_atlas->size;
		tf.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		shadow_atlas->depth = RD::get_singleton()->texture_create(tf, RD::TextureView());
		Vector<RID> fb_tex;
		fb_tex.push_back(shadow_atlas->depth);
		shadow_atlas->fb = RD::get_singleton()->framebuffer_create(fb_tex);
	}
}

lain::RendererRD::LightStorage::LightStorage() {
  singleton = this;
  TextureStorage *texture_storage = TextureStorage::get_singleton();

	directional_shadow.size = GLOBAL_GET("rendering/lights_and_shadows/directional_shadow/size");
	directional_shadow.use_16_bits = GLOBAL_GET("rendering/lights_and_shadows/directional_shadow/16_bits");

	

}

RID LightStorage::directional_light_allocate() {
  return light_owner.allocate_rid();
}

void LightStorage::directional_light_initialize(RID p_light) {
  _light_initialize(p_light, RS::LIGHT_DIRECTIONAL);
}

RID LightStorage::omni_light_allocate() {
  return light_owner.allocate_rid();
}

void LightStorage::omni_light_initialize(RID p_light) {
  _light_initialize(p_light, RS::LIGHT_OMNI);
}

RID LightStorage::spot_light_allocate() {
  return light_owner.allocate_rid();
}

void LightStorage::spot_light_initialize(RID p_light) {
  _light_initialize(p_light, RS::LIGHT_SPOT);
}

void LightStorage::light_free(RID p_rid) {
  light_set_projector(p_rid, RID());  //clear projector

  // delete the texture
  Light* light = light_owner.get_or_null(p_rid);
  light->dependency.deleted_notify(p_rid);
  light_owner.free(p_rid);
}

void LightStorage::light_set_color(RID p_light, const Color& p_color) {
  Light* light = light_owner.get_or_null(p_light);
  ERR_FAIL_NULL(light);

  light->color = p_color;
}

void LightStorage::light_set_param(RID p_light, RS::LightParam p_param, float p_value) {
  Light* light = light_owner.get_or_null(p_light);
  ERR_FAIL_NULL(light);
  ERR_FAIL_INDEX(p_param, RS::LIGHT_PARAM_MAX);

  if (light->param[p_param] == p_value) {
    return;
  }

  switch (p_param) {
    case RS::LIGHT_PARAM_RANGE:
    case RS::LIGHT_PARAM_SPOT_ANGLE:
    case RS::LIGHT_PARAM_SHADOW_MAX_DISTANCE:
    case RS::LIGHT_PARAM_SHADOW_SPLIT_1_OFFSET:
    case RS::LIGHT_PARAM_SHADOW_SPLIT_2_OFFSET:
    case RS::LIGHT_PARAM_SHADOW_SPLIT_3_OFFSET:
    case RS::LIGHT_PARAM_SHADOW_NORMAL_BIAS:
    case RS::LIGHT_PARAM_SHADOW_PANCAKE_SIZE:
    case RS::LIGHT_PARAM_SHADOW_BIAS: {
      light->version++;
      light->dependency.changed_notify(Dependency::DEPENDENCY_CHANGED_LIGHT);
    } break;
    case RS::LIGHT_PARAM_SIZE: {
      if ((light->param[p_param] > CMP_EPSILON) != (p_value > CMP_EPSILON)) {
        //changing from no size to size and the opposite
        light->dependency.changed_notify(Dependency::DEPENDENCY_CHANGED_LIGHT_SOFT_SHADOW_AND_PROJECTOR);
      }
    } break;
    default: {
    }
  }

  light->param[p_param] = p_value;
}

void LightStorage::light_set_shadow(RID p_light, bool p_enabled) {
  Light* light = light_owner.get_or_null(p_light);
  ERR_FAIL_NULL(light);
  light->shadow = p_enabled;

  light->version++;
  light->dependency.changed_notify(Dependency::DEPENDENCY_CHANGED_LIGHT);
}

void LightStorage::light_set_projector(RID p_light, RID p_texture) {
  // TextureStorage *texture_storage = TextureStorage::get_singleton();
  // Light *light = light_owner.get_or_null(p_light);
  // ERR_FAIL_NULL(light);

  // if (light->projector == p_texture) {
  // 	return;
  // }

  // ERR_FAIL_COND(p_texture.is_valid() && !texture_storage->owns_texture(p_texture));

  // if (light->type != RS::LIGHT_DIRECTIONAL && light->projector.is_valid()) {
  // 	texture_storage->texture_remove_from_decal_atlas(light->projector, light->type == RS::LIGHT_OMNI);
  // }

  // light->projector = p_texture;

  // if (light->type != RS::LIGHT_DIRECTIONAL) {
  // 	if (light->projector.is_valid()) {
  // 		texture_storage->texture_add_to_decal_atlas(light->projector, light->type == RS::LIGHT_OMNI);
  // 	}
  // 	light->dependency.changed_notify(Dependency::DEPENDENCY_CHANGED_LIGHT_SOFT_SHADOW_AND_PROJECTOR);
  // }
}

void LightStorage::light_set_negative(RID p_light, bool p_enable) {
  Light* light = light_owner.get_or_null(p_light);
  ERR_FAIL_NULL(light);

  light->negative = p_enable;
}

void LightStorage::light_set_cull_mask(RID p_light, uint32_t p_mask) {
  Light* light = light_owner.get_or_null(p_light);
  ERR_FAIL_NULL(light);

  light->cull_mask = p_mask;

  light->version++;
  light->dependency.changed_notify(Dependency::DEPENDENCY_CHANGED_LIGHT);
}

void LightStorage::light_set_distance_fade(RID p_light, bool p_enabled, float p_begin, float p_shadow, float p_length) {
  Light* light = light_owner.get_or_null(p_light);
  ERR_FAIL_NULL(light);

  light->distance_fade = p_enabled;
  light->distance_fade_begin = p_begin;
  light->distance_fade_shadow = p_shadow;
  light->distance_fade_length = p_length;
}

void LightStorage::light_set_reverse_cull_face_mode(RID p_light, bool p_enabled) {
  Light* light = light_owner.get_or_null(p_light);
  ERR_FAIL_NULL(light);

  light->reverse_cull = p_enabled;

  light->version++;
  light->dependency.changed_notify(Dependency::DEPENDENCY_CHANGED_LIGHT);
}

void LightStorage::light_set_bake_mode(RID p_light, RS::LightBakeMode p_bake_mode) {
  Light* light = light_owner.get_or_null(p_light);
  ERR_FAIL_NULL(light);

  light->bake_mode = p_bake_mode;

  light->version++;
  light->dependency.changed_notify(Dependency::DEPENDENCY_CHANGED_LIGHT);
}

void LightStorage::light_set_max_sdfgi_cascade(RID p_light, uint32_t p_cascade) {
  Light* light = light_owner.get_or_null(p_light);
  ERR_FAIL_NULL(light);

  light->max_sdfgi_cascade = p_cascade;

  light->version++;
  light->dependency.changed_notify(Dependency::DEPENDENCY_CHANGED_LIGHT);
}

void LightStorage::light_omni_set_shadow_mode(RID p_light, RS::LightOmniShadowMode p_mode) {
  Light* light = light_owner.get_or_null(p_light);
  ERR_FAIL_NULL(light);

  light->omni_shadow_mode = p_mode;

  light->version++;
  light->dependency.changed_notify(Dependency::DEPENDENCY_CHANGED_LIGHT);
}

RS::LightOmniShadowMode LightStorage::light_omni_get_shadow_mode(RID p_light) {
  const Light* light = light_owner.get_or_null(p_light);
  ERR_FAIL_NULL_V(light, RS::LIGHT_OMNI_SHADOW_CUBE);

  return light->omni_shadow_mode;
}

bool lain::RendererRD::LightStorage::light_has_shadow(RID p_light) const {
  const Light* light = light_owner.get_or_null(p_light);
  ERR_FAIL_NULL_V(light, RS::LIGHT_DIRECTIONAL);
  return light->shadow;
}

bool lain::RendererRD::LightStorage::light_has_projector(RID p_light) const {
  const Light* light = light_owner.get_or_null(p_light);
  ERR_FAIL_NULL_V(light, RS::LIGHT_DIRECTIONAL);
  return TextureStorage::get_singleton()->owns_texture(light->projector);
}
void LightStorage::light_directional_set_shadow_mode(RID p_light, RS::LightDirectionalShadowMode p_mode) {
  Light* light = light_owner.get_or_null(p_light);
  ERR_FAIL_NULL(light);

  light->directional_shadow_mode = p_mode;
  light->version++;
  light->dependency.changed_notify(Dependency::DEPENDENCY_CHANGED_LIGHT);
}

void LightStorage::light_directional_set_blend_splits(RID p_light, bool p_enable) {
  Light* light = light_owner.get_or_null(p_light);
  ERR_FAIL_NULL(light);

  light->directional_blend_splits = p_enable;
  light->version++;
  light->dependency.changed_notify(Dependency::DEPENDENCY_CHANGED_LIGHT);
}

void LightStorage::light_directional_set_sky_mode(RID p_light, RS::LightDirectionalSkyMode p_mode) {
  Light* light = light_owner.get_or_null(p_light);
  ERR_FAIL_NULL(light);

  light->directional_sky_mode = p_mode;
}

bool LightStorage::light_directional_get_blend_splits(RID p_light) const {
  const Light* light = light_owner.get_or_null(p_light);
  ERR_FAIL_NULL_V(light, false);

  return light->directional_blend_splits;
}

RS::LightDirectionalShadowMode LightStorage::light_directional_get_shadow_mode(RID p_light) {
  const Light* light = light_owner.get_or_null(p_light);
  ERR_FAIL_NULL_V(light, RS::LIGHT_DIRECTIONAL_SHADOW_ORTHOGONAL);

  return light->directional_shadow_mode;
}

Rect2i lain::RendererRD::LightStorage::get_directional_shadow_rect()
{
   	return _get_directional_shadow_rect(directional_shadow.size, directional_shadow.light_count, directional_shadow.current_light);
}

uint32_t LightStorage::light_get_max_sdfgi_cascade(RID p_light) {
  const Light* light = light_owner.get_or_null(p_light);
  ERR_FAIL_NULL_V(light, 0);

  return light->max_sdfgi_cascade;
}

RS::LightBakeMode LightStorage::light_get_bake_mode(RID p_light) {
  const Light* light = light_owner.get_or_null(p_light);
  ERR_FAIL_NULL_V(light, RS::LIGHT_BAKE_DISABLED);

  return light->bake_mode;
}

uint64_t LightStorage::light_get_version(RID p_light) const {
  const Light* light = light_owner.get_or_null(p_light);
  ERR_FAIL_NULL_V(light, 0);

  return light->version;
}

uint32_t LightStorage::light_get_cull_mask(RID p_light) const {
  const Light* light = light_owner.get_or_null(p_light);
  ERR_FAIL_NULL_V(light, 0);

  return light->cull_mask;
}

RS::LightType lain::RendererRD::LightStorage::light_get_type(RID p_light) const {
  const Light* light = light_owner.get_or_null(p_light);
  ERR_FAIL_NULL_V(light, RS::LIGHT_DIRECTIONAL);

  return light->type;
}

/// light的AABB是计算出来的
AABB LightStorage::light_get_aabb(RID p_light) const {
  const Light* light = light_owner.get_or_null(p_light);
  ERR_FAIL_NULL_V(light, AABB());

  switch (light->type) {
    case RS::LIGHT_SPOT: {
      float len = light->param[RS::LIGHT_PARAM_RANGE];
      float size = Math::tan(Math::deg_to_rad(light->param[RS::LIGHT_PARAM_SPOT_ANGLE])) * len;
      return AABB(Vector3(-size, -size, -len), Vector3(size * 2, size * 2, len));
    };
    case RS::LIGHT_OMNI: {
      float r = light->param[RS::LIGHT_PARAM_RANGE];
      return AABB(-Vector3(r, r, r), Vector3(r, r, r) * 2);
    };
    case RS::LIGHT_DIRECTIONAL: {
      return AABB();
    };
  }

  ERR_FAIL_V(AABB());
}

float lain::RendererRD::LightStorage::light_get_param(RID p_light, RS::LightParam p_param) {
  const Light* light = light_owner.get_or_null(p_light);
  ERR_FAIL_NULL_V(light, 0);

  return light->param[p_param];
}

Color lain::RendererRD::LightStorage::light_get_color(RID p_light) {
  const Light* light = light_owner.get_or_null(p_light);
  ERR_FAIL_NULL_V(light, Color());

  return light->color;
}

bool lain::RendererRD::LightStorage::light_get_reverse_cull_face_mode(RID p_light) const {
  const Light* light = light_owner.get_or_null(p_light);
  ERR_FAIL_NULL_V(light, false);

  return light->reverse_cull;
}

Dependency* LightStorage::light_get_dependency(RID p_light) const {
  Light* light = light_owner.get_or_null(p_light);
  ERR_FAIL_NULL_V(light, nullptr);

  return &light->dependency;
}

// 根据 light 构造light instance
RID LightStorage::light_instance_create(RID p_light) {
  RID li = light_instance_owner.make_rid(LightInstance());

  LightInstance* light_instance = light_instance_owner.get_or_null(li);

  light_instance->self = li;
  light_instance->light = p_light;
  light_instance->light_type = light_get_type(p_light);
  if (light_instance->light_type != RS::LIGHT_DIRECTIONAL) {
    // light_instance->forward_id = ForwardIDStorage::get_singleton()->allocate_forward_id(light_instance->light_type == RS::LIGHT_OMNI ? FORWARD_ID_TYPE_OMNI_LIGHT : FORWARD_ID_TYPE_SPOT_LIGHT);
  }

  return li;
}

void LightStorage::light_instance_free(RID p_light) {
  LightInstance* light_instance = light_instance_owner.get_or_null(p_light);

  //remove from shadow atlases..
  for (const RID& E : light_instance->shadow_atlases) {
    ShadowAtlas* shadow_atlas = shadow_atlas_owner.get_or_null(E);
    ERR_CONTINUE(!shadow_atlas->shadow_owners.has(p_light));
    uint32_t key = shadow_atlas->shadow_owners[p_light];
    uint32_t q = (key >> QUADRANT_SHIFT) & 0x3;
    uint32_t s = key & SHADOW_INDEX_MASK;
    // DEV_ASSERT(shadow_atlas->quadrants[q].shadows.write[s].owner == p_light)
    shadow_atlas->quadrants[q].shadows.write[s].owner = RID();

    if (key & OMNI_LIGHT_FLAG) {
      // Omni lights use two atlas spots, make sure to clear the other as well
      shadow_atlas->quadrants[q].shadows.write[s + 1].owner = RID();
    }  // @?什么时候回收？

    shadow_atlas->shadow_owners.erase(p_light);
  }

  if (light_instance->light_type != RS::LIGHT_DIRECTIONAL) {
    // ForwardIDStorage::get_singleton()->free_forward_id(light_instance->light_type == RS::LIGHT_OMNI ? FORWARD_ID_TYPE_OMNI_LIGHT : FORWARD_ID_TYPE_SPOT_LIGHT, light_instance->forward_id);
  }
  light_instance_owner.free(p_light);
}

void LightStorage::light_instance_set_transform(RID p_light_instance, const Transform3D& p_transform) {
  LightInstance* light_instance = light_instance_owner.get_or_null(p_light_instance);
  ERR_FAIL_NULL(light_instance);

  light_instance->transform = p_transform;
}

void LightStorage::light_instance_set_aabb(RID p_light_instance, const AABB& p_aabb) {
  LightInstance* light_instance = light_instance_owner.get_or_null(p_light_instance);
  ERR_FAIL_NULL(light_instance);

  light_instance->aabb = p_aabb;
}

void LightStorage::light_instance_set_shadow_transform(RID p_light_instance, const Projection& p_projection, const Transform3D& p_transform, float p_far, float p_split,
                                                       int p_pass, float p_shadow_texel_size, float p_bias_scale, float p_range_begin, const Vector2& p_uv_scale) {
  LightInstance* light_instance = light_instance_owner.get_or_null(p_light_instance);
  ERR_FAIL_NULL(light_instance);

  ERR_FAIL_INDEX(p_pass, 6);

  light_instance->shadow_transform[p_pass].camera = p_projection;

  light_instance->shadow_transform[p_pass].transform = p_transform;
  light_instance->shadow_transform[p_pass].farplane = p_far;
  light_instance->shadow_transform[p_pass].split = p_split;
  light_instance->shadow_transform[p_pass].bias_scale = p_bias_scale;
  light_instance->shadow_transform[p_pass].range_begin = p_range_begin;
  light_instance->shadow_transform[p_pass].shadow_texel_size = p_shadow_texel_size;
  light_instance->shadow_transform[p_pass].uv_scale = p_uv_scale;
}

void LightStorage::light_instance_mark_visible(RID p_light_instance) {
  LightInstance* light_instance = light_instance_owner.get_or_null(p_light_instance);
  ERR_FAIL_NULL(light_instance);
  // @? mark visible和last_scene_pass有什么关系？
  light_instance->last_scene_pass = RendererSceneRenderRD::get_singleton()->get_scene_pass();
}
// 只计算fade的距离，不是视锥什么的
bool lain::RendererRD::LightStorage::light_instance_is_shadow_visible_at_position(RID p_light_instance, const Vector3& p_position) const {
  const LightInstance* light_instance = light_instance_owner.get_or_null(p_light_instance);
  ERR_FAIL_NULL_V(light_instance, false);
  const Light* light = light_owner.get_or_null(light_instance->light);
  ERR_FAIL_NULL_V(light, false);

  if (!light->shadow) {
    return false;
  }

  if (!light->distance_fade) {
    return true;
  }

  real_t distance = p_position.distance_to(light_instance->transform.origin);
  // fade的距离过远
  if (distance > light->distance_fade_shadow + light->distance_fade_length) {
    return false;
  }

  return true;
}

/* SHADOW CUBEMAPS */

LightStorage::ShadowCubemap *LightStorage::_get_shadow_cubemap(int p_size) {
	if (!shadow_cubemaps.has(p_size)) {
		ShadowCubemap sc;
		{
			RD::TextureFormat tf;
			tf.format = RD::get_singleton()->texture_is_format_supported_for_usage(RD::DATA_FORMAT_D32_SFLOAT, RD::TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) ? RD::DATA_FORMAT_D32_SFLOAT : RD::DATA_FORMAT_X8_D24_UNORM_PACK32;
			tf.width = p_size;
			tf.height = p_size;
			tf.texture_type = RD::TEXTURE_TYPE_CUBE;
			tf.array_layers = 6; // cubemap
			tf.usage_bits = RD::TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | RD::TEXTURE_USAGE_SAMPLING_BIT;
			sc.cubemap = RD::get_singleton()->texture_create(tf, RD::TextureView());
		}

		for (int i = 0; i < 6; i++) {
			RID side_texture = RD::get_singleton()->texture_create_shared_from_slice(RD::TextureView(), sc.cubemap, i, 0);
			Vector<RID> fbtex;
			fbtex.push_back(side_texture);
			sc.side_fb[i] = RD::get_singleton()->framebuffer_create(fbtex);
		}

		shadow_cubemaps[p_size] = sc;
	}

	return &shadow_cubemaps[p_size];
}
RD::DataFormat LightStorage::get_shadow_atlas_depth_format(bool p_16_bits) {
	return p_16_bits ? RD::DATA_FORMAT_D16_UNORM : RD::DATA_FORMAT_D32_SFLOAT;
}
uint32_t lain::RendererRD::LightStorage::get_shadow_atlas_depth_usage_bits()
{
	return RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
}

void lain::RendererRD::LightStorage::update_directional_shadow_atlas()
{
	if (directional_shadow.depth.is_null() && directional_shadow.size > 0) {
		RD::TextureFormat tf;
		tf.format = get_shadow_atlas_depth_format(directional_shadow.use_16_bits);
		tf.width = directional_shadow.size;
		tf.height = directional_shadow.size;
		tf.usage_bits = get_shadow_atlas_depth_usage_bits();

		directional_shadow.depth = RD::get_singleton()->texture_create(tf, RD::TextureView());
		Vector<RID> fb_tex;
		fb_tex.push_back(directional_shadow.depth);
		directional_shadow.fb = RD::get_singleton()->framebuffer_create(fb_tex);
	}
}

RID LightStorage::get_cubemap(int p_size)
{
    ShadowCubemap *cubemap = _get_shadow_cubemap(p_size);

	return cubemap->cubemap;
}

RID LightStorage::get_cubemap_fb(int p_size, int p_pass) {
	ShadowCubemap *cubemap = _get_shadow_cubemap(p_size);

	return cubemap->side_fb[p_pass];
}