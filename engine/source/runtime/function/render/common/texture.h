#pragma once
#ifndef TEXTURE_H
#define TEXTURE_H
#include "base.h"
namespace lain {
	enum class ETextureType
	{
		TEXTURE_2D,
		TEXTURE_VOLUME,
		TEXTURE_2D_ARRAY,
		TEXTURE_CUBE,
		TEXTURE_MAX_ENUM
	};


	struct SamplerState
	{
		enum WrapMode
		{
			WRAP_CLAMP,
			WRAP_CLAMP_ZERO,
			WRAP_CLAMP_ONE,
			WRAP_REPEAT,
			WRAP_MIRRORED_REPEAT,
			WRAP_MAX_ENUM
		};

		enum FilterMode
		{
			FILTER_LINEAR,
			FILTER_NEAREST,
			FILTER_MAX_ENUM
		};

		enum MipmapFilterMode
		{
			MIPMAP_FILTER_NONE,
			MIPMAP_FILTER_LINEAR,
			MIPMAP_FILTER_NEAREST,
			MIPMAP_FILTER_MAX_ENUM
		};

		FilterMode minFilter = FILTER_LINEAR;
		FilterMode magFilter = FILTER_LINEAR;
		MipmapFilterMode mipmapFilter = MIPMAP_FILTER_NONE;

		WrapMode wrapU = WRAP_CLAMP;
		WrapMode wrapV = WRAP_CLAMP;
		WrapMode wrapW = WRAP_CLAMP;

		float lodBias = 0.0f;

		uint8 maxAnisotropy = 1;

		uint8 minLod = 0;
		uint8 maxLod = 0xFF;

		Optional<CompareMode> depthSampleMode;

		uint64 toKey() const;
		static SamplerState fromKey(uint64 key);

		static bool isClampZeroOrOne(WrapMode w);

		static bool getConstant(const char* in, FilterMode& out);
		static bool getConstant(FilterMode in, const char*& out);
		static std::vector<std::string> getConstants(FilterMode);

		static bool getConstant(const char* in, MipmapFilterMode& out);
		static bool getConstant(MipmapFilterMode in, const char*& out);
		static std::vector<std::string> getConstants(MipmapFilterMode);

		static bool getConstant(const char* in, WrapMode& out);
		static bool getConstant(WrapMode in, const char*& out);
		static std::vector<std::string> getConstants(WrapMode);
	};

	class Texture {

	};
	

	enum class EPixelType
	{
		RGBA8, RGBA16, RGBA32, RG16, R16, R32
	};

	class Texture2D : public Texture {
		int m_width; int m_height; bool m_has_mipmaps;


	};
	class Texture3D : public Texture {

	};
	class TextureLayered : public Texture {

	};
}
#endif // !TEXTURE_H
