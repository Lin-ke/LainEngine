#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTex;
layout(location = 2) in vec4 aCol;
layout(location = 3) in int aTexIndex;
layout(location = 4) in float aTilingFactor;
layout(location = 5) in int aInstanceId;

uniform mat4 u_ViewProjection;

out vec2 v_TexCoord;
out vec4 v_Color;
flat out int v_TexIndex;
out float v_TilingFactor;
flat out int v_InstanceId;

void main()
{
	gl_Position = u_ViewProjection * vec4(aPos, 1.0);
	v_TexCoord = aTex;
	v_Color = aCol;
	v_TexIndex = aTexIndex;
	v_TilingFactor = aTilingFactor;
	v_InstanceId = aInstanceId;
}
