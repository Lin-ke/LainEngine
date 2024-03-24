#pragma once
#ifndef TEXTURE_H
#define TEXTURE_H

namespace lain {
	enum class ETextureType
	{
		TEXTURE_2D,
		TEXTURE_VOLUME,
		TEXTURE_2D_ARRAY,
		TEXTURE_CUBE,
		TEXTURE_MAX_ENUM
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
