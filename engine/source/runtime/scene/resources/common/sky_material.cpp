#include "sky_material.h"
using namespace lain;
void PhysicalSkyMaterial::set_rayleigh_coefficient(float p_rayleigh) {
	rayleigh = p_rayleigh;
	RS::get_singleton()->material_set_param(_get_material(), "rayleigh", rayleigh);
}

float PhysicalSkyMaterial::get_rayleigh_coefficient() const {
	return rayleigh;
}

void PhysicalSkyMaterial::set_rayleigh_color(Color p_rayleigh_color) {
	rayleigh_color = p_rayleigh_color;
	RS::get_singleton()->material_set_param(_get_material(), "rayleigh_color", rayleigh_color);
}

Color PhysicalSkyMaterial::get_rayleigh_color() const {
	return rayleigh_color;
}

void PhysicalSkyMaterial::set_mie_coefficient(float p_mie) {
	mie = p_mie;
	RS::get_singleton()->material_set_param(_get_material(), "mie", mie);
}

float PhysicalSkyMaterial::get_mie_coefficient() const {
	return mie;
}

void PhysicalSkyMaterial::set_mie_eccentricity(float p_eccentricity) {
	mie_eccentricity = p_eccentricity;
	RS::get_singleton()->material_set_param(_get_material(), "mie_eccentricity", mie_eccentricity);
}

float PhysicalSkyMaterial::get_mie_eccentricity() const {
	return mie_eccentricity;
}

void PhysicalSkyMaterial::set_mie_color(Color p_mie_color) {
	mie_color = p_mie_color;
	RS::get_singleton()->material_set_param(_get_material(), "mie_color", mie_color);
}

Color PhysicalSkyMaterial::get_mie_color() const {
	return mie_color;
}

void PhysicalSkyMaterial::set_turbidity(float p_turbidity) {
	turbidity = p_turbidity;
	RS::get_singleton()->material_set_param(_get_material(), "turbidity", turbidity);
}

float PhysicalSkyMaterial::get_turbidity() const {
	return turbidity;
}

void PhysicalSkyMaterial::set_sun_disk_scale(float p_sun_disk_scale) {
	sun_disk_scale = p_sun_disk_scale;
	RS::get_singleton()->material_set_param(_get_material(), "sun_disk_scale", sun_disk_scale);
}

float PhysicalSkyMaterial::get_sun_disk_scale() const {
	return sun_disk_scale;
}

void PhysicalSkyMaterial::set_ground_color(Color p_ground_color) {
	ground_color = p_ground_color;
	RS::get_singleton()->material_set_param(_get_material(), "ground_color", ground_color);
}

Color PhysicalSkyMaterial::get_ground_color() const {
	return ground_color;
}

void PhysicalSkyMaterial::set_energy_multiplier(float p_multiplier) {
	energy_multiplier = p_multiplier;
	RS::get_singleton()->material_set_param(_get_material(), "exposure", energy_multiplier);
}

float PhysicalSkyMaterial::get_energy_multiplier() const {
	return energy_multiplier;
}

void PhysicalSkyMaterial::set_use_debanding(bool p_use_debanding) {
	use_debanding = p_use_debanding;
	_update_shader();
	// Only set if shader already compiled
	if (shader_set) {
		RS::get_singleton()->material_set_shader(_get_material(), shader_cache[int(use_debanding)]);
	}
}

bool PhysicalSkyMaterial::get_use_debanding() const {
	return use_debanding;
}

void PhysicalSkyMaterial::set_night_sky(const Ref<Texture2D> &p_night_sky) {
	night_sky = p_night_sky;
	if (p_night_sky.is_valid()) {
		RS::get_singleton()->material_set_param(_get_material(), "night_sky", p_night_sky->GetRID());
	} else {
		RS::get_singleton()->material_set_param(_get_material(), "night_sky", Variant());
	}
}

Ref<Texture2D> PhysicalSkyMaterial::get_night_sky() const {
	return night_sky;
}

Shader::Mode PhysicalSkyMaterial::get_shader_mode() const {
	return Shader::MODE_SKY;
}

RID PhysicalSkyMaterial::GetRID() const {
	_update_shader();
	if (!shader_set) {
		RS::get_singleton()->material_set_shader(_get_material(), shader_cache[1 - int(use_debanding)]);
		RS::get_singleton()->material_set_shader(_get_material(), shader_cache[int(use_debanding)]);
		shader_set = true;
	}
	return _get_material();
}

RID PhysicalSkyMaterial::get_shader_rid() const {
	_update_shader();
	return shader_cache[int(use_debanding)];
}

void PhysicalSkyMaterial::_validate_property(PropertyInfo &p_property) const {
	if (p_property.name == "exposure_value" && !GLOBAL_GET("rendering/lights_and_shadows/use_physical_light_units")) {
		p_property.usage = PROPERTY_USAGE_NO_EDITOR;
	}
}

Mutex PhysicalSkyMaterial::shader_mutex;
RID PhysicalSkyMaterial::shader_cache[2];

void PhysicalSkyMaterial::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_rayleigh_coefficient", "rayleigh"), &PhysicalSkyMaterial::set_rayleigh_coefficient);
	ClassDB::bind_method(D_METHOD("get_rayleigh_coefficient"), &PhysicalSkyMaterial::get_rayleigh_coefficient);

	ClassDB::bind_method(D_METHOD("set_rayleigh_color", "color"), &PhysicalSkyMaterial::set_rayleigh_color);
	ClassDB::bind_method(D_METHOD("get_rayleigh_color"), &PhysicalSkyMaterial::get_rayleigh_color);

	ClassDB::bind_method(D_METHOD("set_mie_coefficient", "mie"), &PhysicalSkyMaterial::set_mie_coefficient);
	ClassDB::bind_method(D_METHOD("get_mie_coefficient"), &PhysicalSkyMaterial::get_mie_coefficient);

	ClassDB::bind_method(D_METHOD("set_mie_eccentricity", "eccentricity"), &PhysicalSkyMaterial::set_mie_eccentricity);
	ClassDB::bind_method(D_METHOD("get_mie_eccentricity"), &PhysicalSkyMaterial::get_mie_eccentricity);

	ClassDB::bind_method(D_METHOD("set_mie_color", "color"), &PhysicalSkyMaterial::set_mie_color);
	ClassDB::bind_method(D_METHOD("get_mie_color"), &PhysicalSkyMaterial::get_mie_color);

	ClassDB::bind_method(D_METHOD("set_turbidity", "turbidity"), &PhysicalSkyMaterial::set_turbidity);
	ClassDB::bind_method(D_METHOD("get_turbidity"), &PhysicalSkyMaterial::get_turbidity);

	ClassDB::bind_method(D_METHOD("set_sun_disk_scale", "scale"), &PhysicalSkyMaterial::set_sun_disk_scale);
	ClassDB::bind_method(D_METHOD("get_sun_disk_scale"), &PhysicalSkyMaterial::get_sun_disk_scale);

	ClassDB::bind_method(D_METHOD("set_ground_color", "color"), &PhysicalSkyMaterial::set_ground_color);
	ClassDB::bind_method(D_METHOD("get_ground_color"), &PhysicalSkyMaterial::get_ground_color);

	ClassDB::bind_method(D_METHOD("set_energy_multiplier", "multiplier"), &PhysicalSkyMaterial::set_energy_multiplier);
	ClassDB::bind_method(D_METHOD("get_energy_multiplier"), &PhysicalSkyMaterial::get_energy_multiplier);

	ClassDB::bind_method(D_METHOD("set_use_debanding", "use_debanding"), &PhysicalSkyMaterial::set_use_debanding);
	ClassDB::bind_method(D_METHOD("get_use_debanding"), &PhysicalSkyMaterial::get_use_debanding);

	ClassDB::bind_method(D_METHOD("set_night_sky", "night_sky"), &PhysicalSkyMaterial::set_night_sky);
	ClassDB::bind_method(D_METHOD("get_night_sky"), &PhysicalSkyMaterial::get_night_sky);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "rayleigh_coefficient", PROPERTY_HINT_RANGE, "0,64,0.01"), "set_rayleigh_coefficient", "get_rayleigh_coefficient");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "rayleigh_color", PROPERTY_HINT_COLOR_NO_ALPHA), "set_rayleigh_color", "get_rayleigh_color");

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "mie_coefficient", PROPERTY_HINT_RANGE, "0,1,0.001"), "set_mie_coefficient", "get_mie_coefficient");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "mie_eccentricity", PROPERTY_HINT_RANGE, "-1,1,0.01"), "set_mie_eccentricity", "get_mie_eccentricity");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "mie_color", PROPERTY_HINT_COLOR_NO_ALPHA), "set_mie_color", "get_mie_color");

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "turbidity", PROPERTY_HINT_RANGE, "0,1000,0.01"), "set_turbidity", "get_turbidity");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sun_disk_scale", PROPERTY_HINT_RANGE, "0,360,0.01"), "set_sun_disk_scale", "get_sun_disk_scale");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "ground_color", PROPERTY_HINT_COLOR_NO_ALPHA), "set_ground_color", "get_ground_color");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "energy_multiplier", PROPERTY_HINT_RANGE, "0,128,0.01"), "set_energy_multiplier", "get_energy_multiplier");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_debanding"), "set_use_debanding", "get_use_debanding");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "night_sky", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_night_sky", "get_night_sky");
}

void PhysicalSkyMaterial::cleanup_shader() {
	if (shader_cache[0].is_valid()) {
		RS::get_singleton()->free(shader_cache[0]);
		RS::get_singleton()->free(shader_cache[1]);
	}
}

void PhysicalSkyMaterial::_update_shader() {
	shader_mutex.lock();
	if (shader_cache[0].is_null()) {
		for (int i = 0; i < 2; i++) {
			shader_cache[i] = RS::get_singleton()->shader_create();

			// Add a comment to describe the shader origin (useful when converting to ShaderMaterial).
			RS::get_singleton()->shader_set_code(shader_cache[i], vformat(R"(

shader_type sky;
%s

uniform float rayleigh : hint_range(0, 64) = 2.0;
uniform vec4 rayleigh_color : source_color = vec4(0.3, 0.405, 0.6, 1.0);
uniform float mie : hint_range(0, 1) = 0.005;
uniform float mie_eccentricity : hint_range(-1, 1) = 0.8;
uniform vec4 mie_color : source_color = vec4(0.69, 0.729, 0.812, 1.0);

uniform float turbidity : hint_range(0, 1000) = 10.0;
uniform float sun_disk_scale : hint_range(0, 360) = 1.0;
uniform vec4 ground_color : source_color = vec4(0.1, 0.07, 0.034, 1.0);
uniform float exposure : hint_range(0, 128) = 1.0;

uniform sampler2D night_sky : filter_linear, source_color, hint_default_black;

const vec3 UP = vec3( 0.0, 1.0, 0.0 );

// Optical length at zenith for molecules.
const float rayleigh_zenith_size = 8.4e3;
const float mie_zenith_size = 1.25e3;

float henyey_greenstein(float cos_theta, float g) {
	const float k = 0.0795774715459;
	return k * (1.0 - g * g) / (pow(1.0 + g * g - 2.0 * g * cos_theta, 1.5));
}

void sky() {
	if (LIGHT0_ENABLED) {
		float zenith_angle = clamp( dot(UP, normalize(LIGHT0_DIRECTION)), -1.0, 1.0 );
		float sun_energy = max(0.0, 1.0 - exp(-((PI * 0.5) - acos(zenith_angle)))) * LIGHT0_ENERGY;
		float sun_fade = 1.0 - clamp(1.0 - exp(LIGHT0_DIRECTION.y), 0.0, 1.0);

		// Rayleigh coefficients.
		float rayleigh_coefficient = rayleigh - ( 1.0 * ( 1.0 - sun_fade ) );
		vec3 rayleigh_beta = rayleigh_coefficient * rayleigh_color.rgb * 0.0001;
		// mie coefficients from Preetham
		vec3 mie_beta = turbidity * mie * mie_color.rgb * 0.000434;

		// Optical length.
		float zenith = acos(max(0.0, dot(UP, EYEDIR)));
		float optical_mass = 1.0 / (cos(zenith) + 0.15 * pow(93.885 - degrees(zenith), -1.253));
		float rayleigh_scatter = rayleigh_zenith_size * optical_mass;
		float mie_scatter = mie_zenith_size * optical_mass;

		// Light extinction based on thickness of atmosphere.
		vec3 extinction = exp(-(rayleigh_beta * rayleigh_scatter + mie_beta * mie_scatter));

		// In scattering.
		float cos_theta = dot(EYEDIR, normalize(LIGHT0_DIRECTION));

		float rayleigh_phase = (3.0 / (16.0 * PI)) * (1.0 + pow(cos_theta * 0.5 + 0.5, 2.0));
		vec3 betaRTheta = rayleigh_beta * rayleigh_phase;

		float mie_phase = henyey_greenstein(cos_theta, mie_eccentricity);
		vec3 betaMTheta = mie_beta * mie_phase;

		vec3 Lin = pow(sun_energy * ((betaRTheta + betaMTheta) / (rayleigh_beta + mie_beta)) * (1.0 - extinction), vec3(1.5));
		// Hack from https://github.com/mrdoob/three.js/blob/master/examples/jsm/objects/Sky.js
		Lin *= mix(vec3(1.0), pow(sun_energy * ((betaRTheta + betaMTheta) / (rayleigh_beta + mie_beta)) * extinction, vec3(0.5)), clamp(pow(1.0 - zenith_angle, 5.0), 0.0, 1.0));

		// Hack in the ground color.
		Lin  *= mix(ground_color.rgb, vec3(1.0), smoothstep(-0.1, 0.1, dot(UP, EYEDIR)));

		// Solar disk and out-scattering.
		float sunAngularDiameterCos = cos(LIGHT0_SIZE * sun_disk_scale);
		float sunAngularDiameterCos2 = cos(LIGHT0_SIZE * sun_disk_scale*0.5);
		float sundisk = smoothstep(sunAngularDiameterCos, sunAngularDiameterCos2, cos_theta);
		vec3 L0 = (sun_energy * extinction) * sundisk * LIGHT0_COLOR;
		L0 += texture(night_sky, SKY_COORDS).xyz * extinction;

		vec3 color = Lin + L0;
		COLOR = pow(color, vec3(1.0 / (1.2 + (1.2 * sun_fade))));
		COLOR *= exposure;
	} else {
		// There is no sun, so display night_sky and nothing else.
		COLOR = texture(night_sky, SKY_COORDS).xyz;
		COLOR *= exposure;
	}
}
)",
																		  i ? "render_mode use_debanding;" : ""));
		}
	}

	shader_mutex.unlock();
}

PhysicalSkyMaterial::PhysicalSkyMaterial() {
	set_rayleigh_coefficient(2.0);
	set_rayleigh_color(Color(0.3, 0.405, 0.6));
	set_mie_coefficient(0.005);
	set_mie_eccentricity(0.8);
	set_mie_color(Color(0.69, 0.729, 0.812));
	set_turbidity(10.0);
	set_sun_disk_scale(1.0);
	set_ground_color(Color(0.1, 0.07, 0.034));
	set_energy_multiplier(1.0);
	set_use_debanding(true);
}

PhysicalSkyMaterial::~PhysicalSkyMaterial() {
}

/* PanoramaSkyMaterial */

void PanoramaSkyMaterial::set_panorama(const Ref<Texture2D> &p_panorama) {
	panorama = p_panorama;
	if (p_panorama.is_valid()) {
		RS::get_singleton()->material_set_param(_get_material(), "source_panorama", p_panorama->GetRID());
	} else {
		RS::get_singleton()->material_set_param(_get_material(), "source_panorama", Variant());
	}
}

Ref<Texture2D> PanoramaSkyMaterial::get_panorama() const {
	return panorama;
}

void PanoramaSkyMaterial::set_filtering_enabled(bool p_enabled) {
	filter = p_enabled;
	notify_property_list_changed();
	_update_shader();
	// Only set if shader already compiled
	if (shader_set) {
		RS::get_singleton()->material_set_shader(_get_material(), shader_cache[int(filter)]);
	}
}

bool PanoramaSkyMaterial::is_filtering_enabled() const {
	return filter;
}

void PanoramaSkyMaterial::set_energy_multiplier(float p_multiplier) {
	energy_multiplier = p_multiplier;
	RS::get_singleton()->material_set_param(_get_material(), "exposure", energy_multiplier);
}

float PanoramaSkyMaterial::get_energy_multiplier() const {
	return energy_multiplier;
}

Shader::Mode PanoramaSkyMaterial::get_shader_mode() const {
	return Shader::MODE_SKY;
}

RID PanoramaSkyMaterial::GetRID() const {
	_update_shader();
	// Don't compile shaders until first use, then compile both
	if (!shader_set) {
		RS::get_singleton()->material_set_shader(_get_material(), shader_cache[1 - int(filter)]);
		RS::get_singleton()->material_set_shader(_get_material(), shader_cache[int(filter)]);
		shader_set = true;
	}
	return _get_material();
}

RID PanoramaSkyMaterial::get_shader_rid() const {
	_update_shader();
	return shader_cache[int(filter)];
}

void PanoramaSkyMaterial::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_panorama", "texture"), &PanoramaSkyMaterial::set_panorama);
	ClassDB::bind_method(D_METHOD("get_panorama"), &PanoramaSkyMaterial::get_panorama);

	ClassDB::bind_method(D_METHOD("set_filtering_enabled", "enabled"), &PanoramaSkyMaterial::set_filtering_enabled);
	ClassDB::bind_method(D_METHOD("is_filtering_enabled"), &PanoramaSkyMaterial::is_filtering_enabled);

	ClassDB::bind_method(D_METHOD("set_energy_multiplier", "multiplier"), &PanoramaSkyMaterial::set_energy_multiplier);
	ClassDB::bind_method(D_METHOD("get_energy_multiplier"), &PanoramaSkyMaterial::get_energy_multiplier);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "panorama", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_panorama", "get_panorama");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "filter"), "set_filtering_enabled", "is_filtering_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "energy_multiplier", PROPERTY_HINT_RANGE, "0,128,0.01"), "set_energy_multiplier", "get_energy_multiplier");
}

Mutex PanoramaSkyMaterial::shader_mutex;
RID PanoramaSkyMaterial::shader_cache[2];

void PanoramaSkyMaterial::cleanup_shader() {
	if (shader_cache[0].is_valid()) {
		RS::get_singleton()->free(shader_cache[0]);
		RS::get_singleton()->free(shader_cache[1]);
	}
}

void PanoramaSkyMaterial::_update_shader() {
	shader_mutex.lock();
	if (shader_cache[0].is_null()) {
		for (int i = 0; i < 2; i++) {
			shader_cache[i] = RS::get_singleton()->shader_create();

			// Add a comment to describe the shader origin (useful when converting to ShaderMaterial).
			RS::get_singleton()->shader_set_code(shader_cache[i], vformat(R"(
shader_type sky;

uniform sampler2D source_panorama : %s, source_color, hint_default_black;
uniform float exposure : hint_range(0, 128) = 1.0;

void sky() {
	COLOR = texture(source_panorama, SKY_COORDS).rgb * exposure;
}
)",
																		  i ? "filter_linear" : "filter_nearest"));
		}
	}

	shader_mutex.unlock();
}

PanoramaSkyMaterial::PanoramaSkyMaterial() {
	set_energy_multiplier(1.0);
}

PanoramaSkyMaterial::~PanoramaSkyMaterial() {
}

