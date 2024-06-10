#include "base.h"
#include "window_system.h"	
#include "platform/render/rendering_context_driver_vulkan_windows.h"

namespace lain {

    WindowSystem* WindowSystem::p_singleton = nullptr;
    int WindowSystem::m_windowid = WindowSystem::MAIN_WINDOW_ID;

    WindowSystem::~WindowSystem()
    {
        p_singleton = nullptr;
            
    }

    void WindowSystem::Initialize()
    {
        if (!glfwInit())
        {
            L_CORE_ERROR(__FUNCTION__, "failed to initialize GLFW");
            return;
        }

        if (!glfwVulkanSupported())
        {
            L_CORE_ERROR("glfw Vulkan not supported.");
        }

        L_CORE_PRINT("window system initialized");

    }

    void  WindowSystem::PollEvents() const {
        glfwPollEvents();
    }

    bool WindowSystem::ShouldClose() const {
        bool should_close = true;
        for (const KeyValue<WindowID, WindowData>& E : m_windows) {
            should_close&=static_cast<bool>(glfwWindowShouldClose(E.value.p_window));
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
        _THREAD_SAFE_METHOD_
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWwindow* window = glfwCreateWindow(create_info.width, create_info.height, CSTR(create_info.title), nullptr, nullptr);
        // add
        if (!window) {
            glfwTerminate();
            ERR_FAIL_V_MSG(-1, "create window error, check glfw");
        }
        // Setup input callbacks
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
        glfwSetWindowIconifyCallback(window, windowIconifyCallback);
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);


        int id = WindowSystem::m_windowid;
        
        
        
        {
            WindowData& wd = m_windows[id];
            HWND hwnd = glfwGetWin32Window(window);
            // get windows handle;
            wd.p_window = window;
            wd.hWnd = hwnd;
            wd.height = create_info.height;
            wd.width = create_info.width;
            // initialization
        }

        // update UserPointer
        for (auto iter = m_windows.begin(); iter != m_windows.end(); ++iter) {
            glfwSetWindowUserPointer(iter->value.p_window, &iter->value);
            //L_PRINT("bind window user pointer", iter->value.p_window, &iter->value);
        }
        m_windowid += 1;
        
        return id;
    }
    



    WindowSystem::WindowID WindowSystem::GetWindowAtPos(const Point2i& p_position) const {
        POINT p;
        Point2i offset(0, 0);
        p.x = static_cast<LONG>(p_position.x + offset.x);
        p.y = static_cast<LONG>(p_position.y + offset.y);
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

   

    WindowSystem::WindowSystem(const String& p_rendering_driver, WindowMode p_mode, VSyncMode p_vsync_mode, uint32_t p_flags, const Vector2i* p_position, const Vector2i& p_resolution, int p_screen, Error& r_error) {
        p_singleton = this;

        rendering_driver = p_rendering_driver;
        // 这个define应该在premake那里
#ifdef L_PLATFORM_WINDOWS
        if (rendering_driver == "vulkan") {
            rendering_context = memnew(graphics::RenderingContextDriverVulkanWindows);
        }
#endif
        if (rendering_context) {
            if (rendering_context->initialize() != OK) {
                memdelete(rendering_context);
                rendering_context = nullptr;
                r_error = ERR_UNAVAILABLE;
                return;
            }
        }
    }
}// namespace lain

