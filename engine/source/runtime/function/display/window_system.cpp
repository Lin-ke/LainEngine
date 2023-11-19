#include "base.h"
#include "window_system.h"	

namespace lain {
    WindowSystem* WindowSystem::p_singleton = nullptr;
    int WindowSystem::m_windowid = 0;

    WindowSystem::~WindowSystem()
    {
        p_singleton = nullptr;
            
    }

    void WindowSystem::initialize()
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

           
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        //m_window = glfwCreateWindow(create_info.width, create_info.height, create_info.title, nullptr, nullptr);
        //if (!m_window)
        //{
        //    L_PRINT(__FUNCTION__, "failed to create window");
        //    glfwTerminate();
        //    return;
        //}

        //// Setup input callbacks
        //glfwSetWindowUserPointer(m_window, this);
        //glfwSetKeyCallback(m_window, keyCallback);
        //glfwSetCharCallback(m_window, charCallback);
        //glfwSetCharModsCallback(m_window, charModsCallback);
        //glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
        //glfwSetCursorPosCallback(m_window, cursorPosCallback);
        //glfwSetCursorEnterCallback(m_window, cursorEnterCallback);
        //glfwSetScrollCallback(m_window, scrollCallback);
        //glfwSetDropCallback(m_window, dropCallback);
        //glfwSetWindowSizeCallback(m_window, windowSizeCallback);
        //glfwSetWindowCloseCallback(m_window, windowCloseCallback);

        //glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
    }

    int WindowSystem::NewWindow(WindowCreateInfo create_info) {
        GLFWwindow* window = glfwCreateWindow(create_info.width, create_info.height, create_info.title, nullptr, nullptr);
        // bind call_backs
        // add
        if (!window) {
            L_ERROR(__FUNCTION__, "create window error, check glfw");
        }

        int id = WindowSystem::m_windowid;
        WindowData& wd = m_windows[id];
        {
            // get windows handle;
            wd.p_window = window;
            wd.height = create_info.height;
            wd.width = create_info.width;
        }
        
        return id;
    }
    template<typename T>



    /*void WindowSystem::pollEvents() const { glfwPollEvents(); }

    bool WindowSystem::shouldClose() const { return glfwWindowShouldClose(m_window); }

    void WindowSystem::setTitle(const char* title) { glfwSetWindowTitle(m_window, title); }

    GLFWwindow* WindowSystem::getWindow() const { return m_window; }

    std::array<int, 2> WindowSystem::getWindowSize() const { return std::array<int, 2>({ m_width, m_height }); }
    
    void WindowSystem::setFocusMode(bool mode)
    {
        m_is_focus_mode = mode;
        glfwSetInputMode(m_window, GLFW_CURSOR, m_is_focus_mode ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }*/
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

