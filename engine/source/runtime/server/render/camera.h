#pragma once
#include <core/math/matrix4.h>
namespace lain{
	typedef Matrix4x4 mat4;
	class Camera
	{
	public:
		Camera() = default;
		Camera(const mat4& projection)
			: m_Projection(projection) {}

		virtual ~Camera() = default;

		const mat4& GetProjection() const { return m_Projection; }
	protected:
		mat4 m_Projection = mat4(1.0f);
	};
}
