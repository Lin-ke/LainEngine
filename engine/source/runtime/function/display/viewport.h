#pragma once
#ifndef __VIEWPORT_H__
#define __VIEWPORT_H__
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include "wtypes.h"	
struct GL_Viewport {
	HWND						winHandle;
	GLFWwindow* glfwHandle;
	char						name[256];
	int							x, y, width, height;
	RECT						area;
	// Camera* camera;
};
#endif // !__VIEWPORT_H__
