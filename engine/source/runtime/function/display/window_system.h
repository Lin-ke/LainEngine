#pragma once
#ifndef __WINDOW_SYSTEM_H__
#define __WINDOW_SYSTEM_H__


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "window.h"
#include <array>
#include <functional>
#include "core/templates/vector.h"
#include "core/templates/rb_map.h"
namespace lain
{
    
    struct WindowCreateInfo
    {
        int         width{ 1280 };
        int         height{ 720 };
        const char* title{ "Piccolo" };
        bool        is_fullscreen{ false };
    };

    class WindowSystem
    {
    public:

        WindowSystem() {
            p_singleton = this;
        };
        ~WindowSystem();
        void               initialize();
        void               pollEvents() const;
        bool               shouldClose() const;
        void               setTitle(const char* title);
        GLFWwindow* getWindow() const;
        std::array<int, 2> getWindowSize() const;

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

        void registerOnResetFunc(onResetFunc func) { m_onResetFunc.push_back(func); }
        void registerOnKeyFunc(onKeyFunc func) { m_onKeyFunc.push_back(func); }
        void registerOnCharFunc(onCharFunc func) { m_onCharFunc.push_back(func); }
        void registerOnCharModsFunc(onCharModsFunc func) { m_onCharModsFunc.push_back(func); }
        void registerOnMouseButtonFunc(onMouseButtonFunc func) { m_onMouseButtonFunc.push_back(func); }
        void registerOnCursorPosFunc(onCursorPosFunc func) { m_onCursorPosFunc.push_back(func); }
        void registerOnCursorEnterFunc(onCursorEnterFunc func) { m_onCursorEnterFunc.push_back(func); }
        void registerOnScrollFunc(onScrollFunc func) { m_onScrollFunc.push_back(func); }
        void registerOnDropFunc(onDropFunc func) { m_onDropFunc.push_back(func); }
        void registerOnWindowSizeFunc(onWindowSizeFunc func) { m_onWindowSizeFunc.push_back(func); }
        void registerOnWindowCloseFunc(onWindowCloseFunc func) { m_onWindowCloseFunc.push_back(func); }
        int NewWindow(WindowCreateInfo create_info);

        bool isMouseButtonDown(int button) const
        {
            if (button < GLFW_MOUSE_BUTTON_1 || button > GLFW_MOUSE_BUTTON_LAST)
            {
                return false;
            }
            return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
        }
        bool getFocusMode() const { return m_is_focus_mode; }
        void setFocusMode(bool mode);
        L_INLINE static int WindowId() {
            return m_windowid++;
        }
    protected:

        // window event callbacks
        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            WindowSystem* app = (WindowSystem*)glfwGetWindowUserPointer(window);
            if (app)
            {
                app->onKey(key, scancode, action, mods);
            }
        }
        static void charCallback(GLFWwindow* window, unsigned int codepoint)
        {
            WindowSystem* app = (WindowSystem*)glfwGetWindowUserPointer(window);
            if (app)
            {
                app->onChar(codepoint);
            }
        }
        static void charModsCallback(GLFWwindow* window, unsigned int codepoint, int mods)
        {
            WindowSystem* app = (WindowSystem*)glfwGetWindowUserPointer(window);
            if (app)
            {
                app->onCharMods(codepoint, mods);
            }
        }
        static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
        {
            WindowSystem* app = (WindowSystem*)glfwGetWindowUserPointer(window);
            if (app)
            {
                app->onMouseButton(button, action, mods);
            }
        }
        static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
        {
            WindowSystem* app = (WindowSystem*)glfwGetWindowUserPointer(window);
            if (app)
            {
                app->onCursorPos(xpos, ypos);
            }
        }
        static void cursorEnterCallback(GLFWwindow* window, int entered)
        {
            WindowSystem* app = (WindowSystem*)glfwGetWindowUserPointer(window);
            if (app)
            {
                app->onCursorEnter(entered);
            }
        }
        static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
        {
            WindowSystem* app = (WindowSystem*)glfwGetWindowUserPointer(window);
            if (app)
            {
                app->onScroll(xoffset, yoffset);
            }
        }
        static void dropCallback(GLFWwindow* window, int count, const char** paths)
        {
            WindowSystem* app = (WindowSystem*)glfwGetWindowUserPointer(window);
            if (app)
            {
                app->onDrop(count, paths);
            }
        }
        static void windowSizeCallback(GLFWwindow* window, int width, int height)
        {
            WindowSystem* app = (WindowSystem*)glfwGetWindowUserPointer(window);
            if (app)
            {
                app->m_width = width;
                app->m_height = height;
            }
        }
        static void windowCloseCallback(GLFWwindow* window) { glfwSetWindowShouldClose(window, true); }

        void onReset()
        {
            for (auto& func : m_onResetFunc)
                func();
        }
        void onKey(int key, int scancode, int action, int mods)
        {
            for (auto& func : m_onKeyFunc)
                func(key, scancode, action, mods);
        }
        void onChar(unsigned int codepoint)
        {
            for (auto& func : m_onCharFunc)
                func(codepoint);
        }
        void onCharMods(int codepoint, unsigned int mods)
        {
            for (auto& func : m_onCharModsFunc)
                func(codepoint, mods);
        }
        void onMouseButton(int button, int action, int mods)
        {
            for (auto& func : m_onMouseButtonFunc)
                func(button, action, mods);
        }
        void onCursorPos(double xpos, double ypos)
        {
            for (auto& func : m_onCursorPosFunc)
                func(xpos, ypos);
        }
        void onCursorEnter(int entered)
        {
            for (auto& func : m_onCursorEnterFunc)
                func(entered);
        }
        void onScroll(double xoffset, double yoffset)
        {
            for (auto& func : m_onScrollFunc)
                func(xoffset, yoffset);
        }
        void onDrop(int count, const char** paths)
        {
            for (auto& func : m_onDropFunc)
                func(count, paths);
        }
        void onWindowSize(int width, int height)
        {
            for (auto& func : m_onWindowSizeFunc)
                func(width, height);
        }

    private:
        RBMap<int, WindowData> m_windows;
        static WindowSystem* p_singleton;
        static int m_windowid;
        Vector<onResetFunc>       m_onResetFunc;
        Vector<onKeyFunc>         m_onKeyFunc;
        Vector<onCharFunc>        m_onCharFunc;
        Vector<onCharModsFunc>    m_onCharModsFunc;
        Vector<onMouseButtonFunc> m_onMouseButtonFunc;
        Vector<onCursorPosFunc>   m_onCursorPosFunc;
        Vector<onCursorEnterFunc> m_onCursorEnterFunc;
        Vector<onScrollFunc>      m_onScrollFunc;
        Vector<onDropFunc>        m_onDropFunc;
        Vector<onWindowSizeFunc>  m_onWindowSizeFunc;
        Vector<onWindowCloseFunc> m_onWindowCloseFunc;



    };
    struct WindowData {
        HWND hWnd;


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

        // Used to transfer data between events using timer.
        WPARAM saved_wparam;
        LPARAM saved_lparam;

        // Timers.
        uint32_t move_timer_id = 0U;
        uint32_t focus_timer_id = 0U;

        HANDLE wtctx;
        int min_pressure;
        int max_pressure;
        bool tilt_supported;
        bool pen_inverted = false;
        bool block_mm = false;

        int last_pressure_update;
        float last_pressure;
        Vector2 last_tilt;
        bool last_pen_inverted = false;

        Size2 min_size;
        Size2 max_size;
        int width = 0, height = 0;

        Size2 window_rect;
        Point2 last_pos;

        ObjectID instance_id;

        // IME
        HIMC im_himc;
        Vector2 im_position;
        bool ime_active = false;
        bool ime_in_progress = false;
        bool ime_suppress_next_keyup = false;

        bool layered_window = false;

        Callable rect_changed_callback;
        Callable event_callback;
        Callable input_event_callback;
        Callable input_text_callback;
        Callable drop_files_callback;

        WindowID transient_parent = INVALID_WINDOW_ID;
        HashSet<WindowID> transient_children;

        bool is_popup = false;
        Rect2i parent_safe_rect;
    };
} // namespace lain

#endif
