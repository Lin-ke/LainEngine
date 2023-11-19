#pragma once

#ifndef __WINDOW_H__
#define __WINDOW_H__
#include "core/string/lstring.h"
#include <GLFW/glfw3.h>
// �о�����Ҫwindow�ĳ��� ֻ��Ҫwindowdata�ĳ���
class WindowSystem;
namespace lain{

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