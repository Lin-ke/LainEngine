#pragma once

#ifndef __WINDOW_H__
#define __WINDOW_H__
#include "core/string/ustring.h"
#include <GLFW/glfw3.h>
// 感觉不需要window的抽象 只需要windowdata的抽象

namespace lain{
	class WindowSystem;

	class Window {
		friend class WindowSystem;
	public:
		Window() {
		}
		~Window() {
			glfwDestroyWindow(m_window);
			glfwTerminate();
		}

	private:
		int m_id = 0;
		String title;
		GLFWwindow* m_window{ nullptr };
		int         m_width{ 0 };
		int         m_height{ 0 };
		// window configs
		bool m_is_focus_mode{ false };
		

	};
}
#endif