#include "base.h"
#include "window_system.h"	
namespace lain {
    WindowSystem* WindowSystem::p_singleton = nullptr;
    int WindowSystem::m_windowid = WindowSystem::MAIN_WINDOW_ID;

    WindowSystem::~WindowSystem()
    {
        p_singleton = nullptr;
            
    }

    void WindowSystem::Initialize()
    {
        if (!glfwVulkanSupported())
        {
            L_CORE_ERROR("glfw Vulkan not supported.");
        }
        if (!glfwInit())
        {
            L_PRINT(__FUNCTION__, "failed to initialize GLFW");
            return;
        }
    }

    void  WindowSystem::PollEvents() const {
        glfwPollEvents();
    }
    bool WindowSystem::ShouldClose() const {
        bool should_close = true;
        for (const KeyValue<WindowID, WindowData>& E : m_windows) {
            should_close&=glfwWindowShouldClose(E.value.p_window);
        }
        return should_close;
    }
    /// <summary>
    /// TODO:在create_info里加入
    /// 1. 窗口初始化位置信息。窗口占屏幕信息
    /// 
    /// </summary>
    /// <param name="create_info"></param>
    /// <returns> id </returns>
    int WindowSystem::NewWindow(WindowCreateInfo create_info) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWwindow* window = glfwCreateWindow(create_info.width, create_info.height, create_info.title, nullptr, nullptr);
        // add
        if (!window) {
            L_ERROR(__FUNCTION__, "create window error, check glfw");
            glfwTerminate();
        }
        // Setup input callbacks
        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, keyCallback);
        glfwSetCharCallback(window, charCallback);
        glfwSetCharModsCallback(window, charModsCallback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetCursorPosCallback(window, cursorPosCallback);
        glfwSetCursorEnterCallback(window, cursorEnterCallback);
        glfwSetScrollCallback(window, scrollCallback);
        glfwSetDropCallback(window, dropCallback);
        glfwSetWindowSizeCallback(window, windowSizeCallback);
        glfwSetWindowCloseCallback(window, windowCloseCallback);

        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);


        int id = WindowSystem::m_windowid;
        WindowData& wd = m_windows[id];
        {
            // get windows handle;
            wd.p_window = window;
            wd.height = create_info.height;
            wd.width = create_info.width;
            // initialization
        }
        // update UserPointer
        for (auto iter = m_windows.begin(); iter != m_windows.end(); ++iter) {
            glfwSetWindowUserPointer(iter->value.p_window, &iter->value);
        }
        m_windowid += 1;
        
        return id;
    }
    



    WindowSystem::WindowID WindowSystem::GetWindowAtPos(const Point2& p_position) const {
        POINT p;
        Point2 offset(0, 0);
        p.x = p_position.x + offset.x;
        p.y = p_position.y + offset.y;
        HWND hwnd = WindowFromPoint(p);
        for (const KeyValue<WindowID, WindowData>& E : m_windows) {
            if (E.value.hWnd == hwnd) {
                return E.key;
            }
        }

        return INVALID_WINDOW_ID;
    }
    Vector<int> WindowSystem::get_window_list() const {

            Vector<int> ret;
        for (const KeyValue<int, WindowData>& E : m_windows) {
            ret.push_back(E.key);
        }
        return ret;
    }
}// namespace lain

