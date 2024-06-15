#pragma once
#ifndef __WINDOW_SYSTEM_H__
#define __WINDOW_SYSTEM_H__

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include "function/render/vulkan/vulkan_header.h"
#include "core/math/vector2i.h"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include "window.h"
#include <array>
#include <functional>
#include "core/templates/vector.h"
#include "core/templates/rb_map.h"
#include "core/os/thread_safe.h"
#include "core/math/rect2i.h"
namespace lain
{
    // @ TODO 把他做成接口和实现的类型

    struct WindowCreateInfo;

    typedef std::function<void()>                   onResetFunc;
    typedef std::function<void(int, int, int, int)> onKeyFunc;
    typedef std::function<void(unsigned int)>       onCharFunc;
    typedef std::function<void(int, unsigned int)>  onCharModsFunc;
    typedef std::function<void(int, int, int)>      onMouseButtonFunc;
    typedef std::function<void(double, double)>     onCursorPosFunc;
    typedef std::function<void(int)>                onCursorEnterFunc;
    typedef std::function<void(double, double)>     onScrollFunc;
    typedef std::function<void(int, const char**)>  onDropFunc;
    typedef std::function<void(int, int)>           onWindowSizeFunc;
    typedef std::function<void()>                   onWindowCloseFunc;
    struct WindowData {

        HWND hWnd = nullptr; // do not modify the window by HWND, using GLFW
        GLFWwindow* p_window = nullptr;

        bool maximized = false;
        bool minimized = false;
        bool fullscreen = false;
        bool multiwindow_fs = false;
        bool borderless = false;
        bool resizable = true;
        bool window_focused = false;
        bool was_maximized = false;
        bool always_on_top = false;
        bool no_focus = false;
        bool window_has_focus = false;
        bool exclusive = false;
        bool context_created = false;
        bool mpass = false;
        bool m_is_focus_mode = true;

        // Used to transfer data between events using timer.
        //WPARAM saved_wparam;
        //LPARAM saved_lparam;

        // Timers.
        uint32_t move_timer_id = 0U;
        uint32_t focus_timer_id = 0U;

       /* HANDLE wtctx;
        int min_pressure;
        int max_pressure;
        bool tilt_supported;
        bool pen_inverted = false;
        bool block_mm = false;

        int last_pressure_update;
        float last_pressure;
        Vector2 last_tilt;
        bool last_pen_inverted = false;*/

        Size2i min_size;
        Size2i max_size;

        Size2i window_rect;
        Point2i last_pos;
        int width = 0, height = 0;


        // IME
      /*  HIMC im_himc;
        Vector2 im_position;
        bool ime_active = false;
        bool ime_in_progress = false;
        bool ime_suppress_next_keyup = false;

        bool layered_window = false;*/
        // 把这里变成vector的可以降低耦合
        // 这样不同的函数可以承担不同的任务
        // 执行列表
        Vector<onResetFunc>       m_onResetFunc;
        Vector < onKeyFunc>        m_onKeyFunc;
        Vector < onCharFunc>     m_onCharFunc;
        Vector < onCharModsFunc >   m_onCharModsFunc;
        Vector < onMouseButtonFunc >m_onMouseButtonFunc;
        Vector < onCursorPosFunc >  m_onCursorPosFunc;
        Vector < onCursorEnterFunc> m_onCursorEnterFunc;
        Vector < onScrollFunc  >    m_onScrollFunc;
        Vector < onDropFunc   >     m_onDropFunc;
        Vector < onWindowSizeFunc>  m_onWindowSizeFunc;
        Vector < onWindowCloseFunc> m_onWindowCloseFunc;

        //WindowID transient_parent = INVALID_WINDOW_ID;
        //HashSet<WindowID> transient_children;

        //bool is_popup = false;
        //Rect2i parent_safe_rect;
    };



    namespace graphics {
        class RenderingContextDriver;
        class RenderingDevice;
    }
    class WindowSystem
    {

        _THREAD_SAFE_CLASS_
        String rendering_driver;
        graphics::RenderingContextDriver* rendering_context = nullptr;
        graphics::RenderingDevice* rendering_device = nullptr;

    public:
        typedef int WindowID;

        enum VSyncMode {
            VSYNC_DISABLED,
            VSYNC_ENABLED,
            VSYNC_ADAPTIVE,
            VSYNC_MAILBOX
        };

        enum WindowMode {
            WINDOW_MODE_WINDOWED,
            WINDOW_MODE_MINIMIZED,
            WINDOW_MODE_MAXIMIZED,
            WINDOW_MODE_FULLSCREEN,
            WINDOW_MODE_EXCLUSIVE_FULLSCREEN,
        };

        enum {
            SCREEN_WITH_MOUSE_FOCUS = -4,
            SCREEN_WITH_KEYBOARD_FOCUS = -3,
            SCREEN_PRIMARY = -2,
            SCREEN_OF_MAIN_WINDOW = -1, // Note: for the main window, determine screen from position.
        };

        WindowSystem(const String& p_rendering_driver, WindowMode p_mode, VSyncMode p_vsync_mode, uint32_t p_flags, const Vector2i* p_position, const Vector2i& p_resolution, int p_screen, Error& r_error);
        ~WindowSystem();
        L_INLINE static WindowSystem* GetSingleton() {
            return p_singleton;
        }
        void               Initialize();
        void               PollEvents() const;
        bool               ShouldClose() const;
        void window_set_vsync_mode(VSyncMode p_vsync_mode, WindowID p_window);
        GLFWwindow* getWindow() const {};
        std::array<int, 2> getWindowsize() const {};

        Point2i mouse_get_position() const;
        int get_screen_count() const;
        Point2i screen_get_position(int p_screen) const;
        int get_screen_from_rect(const Rect2& p_rect) const;
        Size2i screen_get_size(int p_screen) const;
        int get_keyboard_focus_screen() const;
        int get_primary_screen() const;
        int window_get_current_screen(WindowID) const;

    private:
        


        Point2i _get_screens_origin() const;
        Rect2i  screen_get_usable_rect(int p_screen) const;
        int     _get_screen_index(int p_screen) const;
    public:
        void registerOnResetFunc(onResetFunc func, int wid) {
            ERR_FAIL_COND(!m_windows.has(wid));
            m_windows[wid].m_onResetFunc.push_back(func);
        }
        void registerOnKeyFunc(onKeyFunc func, int wid) {
            ERR_FAIL_COND(!m_windows.has(wid));
            m_windows[wid].m_onKeyFunc.push_back(func);
        }
        void registerOnCharFunc(onCharFunc func, int wid) {
            ERR_FAIL_COND(!m_windows.has(wid));
            m_windows[wid].m_onCharFunc.push_back(func);
        }
        void registerOnCharModsFunc(onCharModsFunc func, int wid) {
            ERR_FAIL_COND(!m_windows.has(wid));
            m_windows[wid].m_onCharModsFunc.push_back(func);
        }
        void registerOnMouseButtonFunc(onMouseButtonFunc func, int wid) {
            ERR_FAIL_COND(!m_windows.has(wid));
            m_windows[wid].m_onMouseButtonFunc.push_back(func);
        }
        void registerOnCursorPosFunc(onCursorPosFunc func, int wid) {
            ERR_FAIL_COND(!m_windows.has(wid));
            m_windows[wid].m_onCursorPosFunc.push_back(func);
        }
        void registerOnCursorEnterFunc(onCursorEnterFunc func, int wid) {
            ERR_FAIL_COND(!m_windows.has(wid));
            m_windows[wid].m_onCursorEnterFunc.push_back(func);
        }
        void registerOnScrollFunc(onScrollFunc func, int wid) {
            ERR_FAIL_COND(!m_windows.has(wid));
            m_windows[wid].m_onScrollFunc.push_back(func);
        }
        void registerOnDropFunc(onDropFunc func, int wid) {
            ERR_FAIL_COND(!m_windows.has(wid));
            m_windows[wid].m_onDropFunc.push_back(func);
        }
        void registerOnWindowSizeFunc(onWindowSizeFunc func, int wid) {
            ERR_FAIL_COND(!m_windows.has(wid));
            m_windows[wid].m_onWindowSizeFunc.push_back(func);
        }
        void registerOnWindowCloseFunc(onWindowCloseFunc func, int wid) {
            ERR_FAIL_COND(!m_windows.has(wid));
            m_windows[wid].m_onWindowCloseFunc.push_back(func);
        }
        WindowID NewWindow(const WindowCreateInfo* create_info);

        bool isMouseButtonDown(int button, int wid = MAIN_WINDOW_ID) const
        {
            ERR_FAIL_COND(!m_windows.has(wid));
            if (button < GLFW_MOUSE_BUTTON_1 || button > GLFW_MOUSE_BUTTON_LAST)
            {
                return false;
            }
            return glfwGetMouseButton(m_windows[wid].p_window, button) == GLFW_PRESS;
        }
        bool getFocusMode(int wid) const {
            ERR_FAIL_COND(!m_windows.has(wid));
            return m_windows[wid].m_is_focus_mode;
        }
        void setFocusMode(bool mode) {};

        WindowID GetWindowAtPos(const Point2i& point) const;

        bool CanAnyWindowDraw() {
            for (const KeyValue<WindowID, WindowData>& E : m_windows) {
                if (!E.value.minimized) {
                    return true;
                }
            }
            return false;
        }

        void SwapBuffers() {
            for (const KeyValue<WindowID, WindowData>& E : m_windows) {
                auto p_window = E.value.p_window;
                glfwMakeContextCurrent(p_window);
                // do the rendering
                // 这样设计合理吗？
                glfwSwapBuffers(p_window);

            }
        }


        // window event callbacks
        // 这些glfw绑定keycallback，然后发生这一事件后调用在vector里的callback。
        // TODO：满足多窗口
        // 需要static来绑定。
        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            WindowData* app = (WindowData*)glfwGetWindowUserPointer(window);
            if (app)
            {
                for (auto& func : app->m_onKeyFunc) {
                    func(key, scancode, action, mods);
                }

            }
        }
        static void charCallback(GLFWwindow* window, unsigned int codepoint)
        {
            WindowData* app = (WindowData*)glfwGetWindowUserPointer(window);
            if (app)
            {
                for (auto& func : app->m_onCharFunc) { func(codepoint); }
            }
        }
        static void charModsCallback(GLFWwindow* window, unsigned int codepoint, int mods)
        {
            WindowData* app = (WindowData*)glfwGetWindowUserPointer(window);
            if (app)
            {
                for (auto& func : app->m_onCharModsFunc) { func(codepoint, mods); }
            }
        }
        static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
        {
            WindowData* app = (WindowData*)glfwGetWindowUserPointer(window);
            if (app)
            {
                for (auto& func : app->m_onMouseButtonFunc) { func(button, action, mods); }
            }
        }
        static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
        {
            WindowData* app = (WindowData*)glfwGetWindowUserPointer(window);
            if (app)
            {
                for (auto& func : app->m_onCursorPosFunc) { func(xpos, ypos); }
            }
        }
        static void cursorEnterCallback(GLFWwindow* window, int entered)
        {
            WindowData* app = (WindowData*)glfwGetWindowUserPointer(window);
            if (app)
            {
                for (auto& func : app->m_onCursorEnterFunc) { func(entered); }
            }
        }
        static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
        {
            WindowData* app = (WindowData*)glfwGetWindowUserPointer(window);
            if (app)
            {
                for (auto& func : app->m_onScrollFunc) { func(xoffset, yoffset); }
            }
        }
        static void dropCallback(GLFWwindow* window, int count, const char** paths)
        {
            WindowData* app = (WindowData*)glfwGetWindowUserPointer(window);
            if (app)
            {
                for (auto& func : app->m_onDropFunc) { func(count, paths); }
            }
        }
        static void windowSizeCallback(GLFWwindow* window, int width, int height)
        {
            WindowData* app = (WindowData*)glfwGetWindowUserPointer(window);
            if (app)
            {
                app->width = width;
                app->height =height ;
                for (auto& func : app->m_onWindowSizeFunc) { func(width, height); }
            }

        }
        static void windowCloseCallback(GLFWwindow* window) {
            glfwSetWindowShouldClose(window, true);
            WindowData* app = (WindowData*)glfwGetWindowUserPointer(window);
            if (app)
            {
                for (auto& func : app->m_onWindowCloseFunc) { func(); }
            }
            //L_PRINT("close callback called", window);
        }

        static void windowIconifyCallback(GLFWwindow* window,int flag) {
            
            WindowData* app = (WindowData*)glfwGetWindowUserPointer(window);
            if (app) {
                if (flag) // GL_TRUE, iconified
                    app->minimized = true;
                else{ app->minimized = false; }
            }
        }
        Vector<int> get_window_list() const;
       
    private:
        RBMap<int, WindowData> m_windows;
        static WindowSystem* p_singleton;
        static int m_windowid; // windows_counter
    public:
        enum {
            MAIN_WINDOW_ID = 0,
            INVALID_WINDOW_ID = -1
        };

    };


    struct WindowCreateInfo
    {
        Rect2i      rect{ {0,0}, {720,640} };
        String      title{ "Window" };
        WindowSystem::WindowMode mode  = WindowSystem::WindowMode::WINDOW_MODE_WINDOWED;
        WindowSystem::VSyncMode  vsync = WindowSystem::VSyncMode::VSYNC_ADAPTIVE;
        uint32_t    flags = 0; // flags: 在哪个屏幕；是否全屏

        WindowCreateInfo() {}
    };

} // namespace lain

#endif
