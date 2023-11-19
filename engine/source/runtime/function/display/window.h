#pragma once

#ifndef __WINDOW_H__
#define __WINDOW_H__
#include "core/string/lstring.h"
#include <GLFW/glfw3.h>
class WindowSystem;
namespace lain{

	class Window {
		friend class WindowSystem;
	public:
		Window() {
			m_id = WindowSystem::WindowId();
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

		bool m_is_focus_mode{ false };
		

	};
}
#endif