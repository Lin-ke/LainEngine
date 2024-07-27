#include "window_system.h"
#include "base.h"
#include "core/config/project_settings.h"
#include "function/render/common/rendering_device.h"
#include "platform/render/rendering_context_driver_vulkan_windows.h"
namespace lain {

WindowSystem *WindowSystem::p_singleton = nullptr;
int WindowSystem::m_windowid = WindowSystem::MAIN_WINDOW_ID; // counter

WindowSystem::~WindowSystem() {
	p_singleton = nullptr;
}

void WindowSystem::PollEvents() const {
	glfwPollEvents();
}

bool WindowSystem::ShouldClose() const {
	bool should_close = true;
	for (const KeyValue<WindowID, WindowData> &E : m_windows) {
		should_close &= static_cast<bool>(glfwWindowShouldClose(E.value.p_window));
	}
	return should_close;
}

typedef struct {
	int count;
	int screen;
	Point2 pos;
} EnumPosData;
typedef struct {
	int count;
	int screen;
	int dpi;
} EnumDpiData;

typedef struct {
	int count;
	int screen;
	Size2 size;
} EnumSizeData;

typedef struct {
	int count;
	int screen;
	Rect2i rect;
} EnumRectData;

static BOOL CALLBACK _MonitorEnumProcOrigin(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
	EnumPosData *data = (EnumPosData *)dwData;
	data->pos = data->pos.min(Point2(lprcMonitor->left, lprcMonitor->top));

	return TRUE;
}
static BOOL CALLBACK _MonitorEnumProcUsableSize(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
	EnumRectData *data = (EnumRectData *)dwData;
	if (data->count == data->screen) {
		MONITORINFO minfo;
		memset(&minfo, 0, sizeof(MONITORINFO));
		minfo.cbSize = sizeof(MONITORINFO);
		GetMonitorInfoA(hMonitor, &minfo);

		data->rect.position.x = minfo.rcWork.left;
		data->rect.position.y = minfo.rcWork.top;
		data->rect.size.x = minfo.rcWork.right - minfo.rcWork.left;
		data->rect.size.y = minfo.rcWork.bottom - minfo.rcWork.top;
	}

	data->count++;
	return TRUE;
}

Point2i WindowSystem::_get_screens_origin() const {
	_THREAD_SAFE_METHOD_

	EnumPosData data = { 0, 0, Point2() };
	EnumDisplayMonitors(nullptr, nullptr, _MonitorEnumProcOrigin, (LPARAM)&data);
	return data.pos;
}

/// <summary>
/// @TODO: 加入设置窗口初始化位置的代码
/// //@TODO 增加选择显示器的部分
///
/// </summary>
/// <param name="create_info"></param>
/// <returns> id </returns>
WindowSystem::WindowID WindowSystem::NewWindow(const WindowCreateInfo *create_info) {
	_THREAD_SAFE_METHOD_
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow *window = glfwCreateWindow(create_info->rect.size[0], create_info->rect.size[1], CSTR(create_info->title), nullptr, nullptr);
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

	auto p_mode = create_info->mode;

	{
		WindowData &wd = m_windows[id];
		HWND hwnd = glfwGetWin32Window(window);
		wd.hWnd = hwnd;
		if (!wd.hWnd) {
			MessageBoxW(nullptr, L"Window Creation Error.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
			m_windows.erase(id);
			ERR_FAIL_V_MSG(INVALID_WINDOW_ID, "Failed to create Windows OS window.");
		}
		if (p_mode == WINDOW_MODE_FULLSCREEN || p_mode == WINDOW_MODE_EXCLUSIVE_FULLSCREEN) {
			wd.fullscreen = true;
			if (p_mode == WINDOW_MODE_FULLSCREEN) {
				wd.multiwindow_fs = true;
			}
		}
		/*if (p_mode != WINDOW_MODE_FULLSCREEN && p_mode != WINDOW_MODE_EXCLUSIVE_FULLSCREEN) {
			wd.pre_fs_valid = true;
		}*/

		/*  if (is_dark_mode_supported() && dark_title_available) {
			  BOOL value = is_dark_mode();
			  ::DwmSetWindowAttribute(wd.hWnd, use_legacy_dark_mode_before_20H1 ? DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1 : DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
		  }*/
		// get windows handle;
		wd.p_window = window;
		wd.hWnd = hwnd;

		// 注意这里传入windowdata信息到context driver
		if (rendering_context) {
			union {
#ifdef VULKAN_ENABLED
				graphics::RenderingContextDriverVulkanWindows::WindowPlatformData vulkan;
#endif
#ifdef D3D12_ENABLED
				RenderingContextDriverD3D12::WindowPlatformData d3d12;
#endif
			} wpd;
#ifdef VULKAN_ENABLED
			if (rendering_driver == "vulkan") {
				wpd.vulkan.window = wd.hWnd;
				//wpd.vulkan.instance = GetModuleHandleA(NULL);
			}
#endif
#ifdef D3D12_ENABLED
			if (rendering_driver == "d3d12") {
				wpd.d3d12.window = wd.hWnd;
			}
#endif
			if (rendering_context->window_create(id, &wpd) != OK) {
				ERR_PRINT(vformat("Failed to create %s window.", rendering_driver));
				memdelete(rendering_context);
				rendering_context = nullptr;
				m_windows.erase(id);
				return INVALID_WINDOW_ID;
			}

			RECT WindowRect;
			GetClientRect(wd.hWnd, &WindowRect);
			rendering_context->window_set_size(id, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top);
			rendering_context->window_set_vsync_mode(id, create_info->vsync);
			wd.context_created = true;
		}

		if (p_mode == WINDOW_MODE_MAXIMIZED) {
			wd.maximized = true;
			wd.minimized = false;
		}

		if (p_mode == WINDOW_MODE_MINIMIZED) {
			wd.maximized = false;
			wd.minimized = true;
		}

		RECT r;
		GetClientRect(wd.hWnd, &r);
		ClientToScreen(wd.hWnd, (POINT *)&r.left);
		ClientToScreen(wd.hWnd, (POINT *)&r.right);
		wd.last_pos = Point2i(r.left, r.top) - _get_screens_origin();
		wd.width = r.right - r.left;
		wd.height = r.bottom - r.top;

		window_set_vsync_mode(create_info->vsync, id);
	}

	// update UserPointer
	for (auto iter = m_windows.begin(); iter != m_windows.end(); ++iter) {
		glfwSetWindowUserPointer(iter->value.p_window, &iter->value);
		//L_PRINT("bind window user pointer", iter->value.p_window, &iter->value);
	}
	m_windowid += 1;

	return id;
}
static BOOL CALLBACK _MonitorEnumProcCount(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
	int *data = (int *)dwData;
	(*data)++;
	return TRUE;
}
int WindowSystem::get_screen_count() const {
	_THREAD_SAFE_METHOD_

	int data = 0;
	EnumDisplayMonitors(nullptr, nullptr, _MonitorEnumProcCount, (LPARAM)&data);
	return data;
}
/// <summary>
/// monitor position backcall
/// </summary>
/// <param name="hMonitor"></param>
/// <param name="hdcMonitor"></param>
/// <param name="lprcMonitor"></param>
/// <param name="dwData"></param>
/// <returns></returns>
static BOOL CALLBACK _MonitorEnumProcPos(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
	EnumPosData *data = (EnumPosData *)dwData;
	if (data->count == data->screen) {
		data->pos.x = lprcMonitor->left;
		data->pos.y = lprcMonitor->top;
	}

	data->count++;
	return TRUE;
}

Point2i WindowSystem::screen_get_position(int p_screen) const {
	_THREAD_SAFE_METHOD_

	p_screen = _get_screen_index(p_screen);
	EnumPosData data = { 0, p_screen, Point2() };
	EnumDisplayMonitors(nullptr, nullptr, _MonitorEnumProcPos, (LPARAM)&data);
	return data.pos - _get_screens_origin();
}
WindowSystem::WindowID WindowSystem::GetWindowAtPos(const Point2i &p_position) const {
	POINT p;
	Point2i offset(0, 0);
	p.x = static_cast<LONG>(p_position.x + offset.x);
	p.y = static_cast<LONG>(p_position.y + offset.y);
	HWND hwnd = WindowFromPoint(p);
	for (const KeyValue<WindowID, WindowData> &E : m_windows) {
		if (E.value.hWnd == hwnd) {
			return E.key;
		}
	}

	return INVALID_WINDOW_ID;
}
Vector<int> WindowSystem::get_window_list() const {
	Vector<int> ret;
	for (const KeyValue<int, WindowData> &E : m_windows) {
		ret.push_back(E.key);
	}
	return ret;
}

WindowSystem::WindowSystem(const String &p_rendering_driver, WindowMode p_mode, VSyncMode p_vsync_mode, uint32_t p_flags, const Vector2i *p_position, const Vector2i &p_resolution, int p_screen, Error &r_error) {
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
	Point2i window_position;
	if (p_position != nullptr) {
		window_position = *p_position;
	} else {
		if (p_screen == SCREEN_OF_MAIN_WINDOW) {
			p_screen = SCREEN_PRIMARY;
		}
		Rect2i scr_rect = screen_get_usable_rect(p_screen);
		window_position = scr_rect.position + (scr_rect.size - p_resolution) / 2; // 设置窗口位置为scr_rect
	}

	{ // GLFW initialize
		if (!glfwInit()) {
			L_CORE_ERROR(__FUNCTION__, "failed to initialize GLFW");
			return;
		}

		if (!glfwVulkanSupported()) {
			L_CORE_ERROR("glfw Vulkan not supported.");
		}

		L_CORE_PRINT("window system initialized");
	}

	WindowCreateInfo create_info = {};
	create_info.rect = { window_position, p_resolution };
	create_info.vsync = p_vsync_mode;
	create_info.mode = p_mode;
	create_info.title = GLOBAL_GET("application/config/name");
	WindowID main_window = NewWindow(&create_info); // 这里window 的 need_resize 为true
	// 在构造时进行screen_creates()
	if (rendering_context) {
		rendering_device = memnew(graphics::RenderingDevice);
		if (rendering_device->initialize(rendering_context, MAIN_WINDOW_ID) != OK) {
			memdelete(rendering_device);
			ERR_PRINT("Failed to initialize rendering device.");
			memdelete(rendering_device);
			rendering_device = nullptr;
			r_error = ERR_UNAVAILABLE;
			return;
		};
		rendering_device->screen_create(MAIN_WINDOW_ID); // swap chain

		// RendererCompositorRD::make_current();
	}
}

Point2i WindowSystem::mouse_get_position() const {
	POINT p;
	GetCursorPos(&p);
	return Point2i(p.x, p.y) - _get_screens_origin();
}
static BOOL CALLBACK _MonitorEnumProcSize(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
	EnumSizeData *data = (EnumSizeData *)dwData;
	if (data->count == data->screen) {
		data->size.x = lprcMonitor->right - lprcMonitor->left;
		data->size.y = lprcMonitor->bottom - lprcMonitor->top;
	}

	data->count++;
	return TRUE;
}

Size2i WindowSystem::screen_get_size(int p_screen) const {
	_THREAD_SAFE_METHOD_

	p_screen = _get_screen_index(p_screen);
	EnumSizeData data = { 0, p_screen, Size2() };
	EnumDisplayMonitors(nullptr, nullptr, _MonitorEnumProcSize, (LPARAM)&data);
	return data.size;
}

int WindowSystem::get_screen_from_rect(const Rect2 &p_rect) const {
	int nearest_area = 0;
	int pos_screen = -1;
	for (int i = 0; i < get_screen_count(); i++) {
		Rect2i r;
		r.position = screen_get_position(i);
		r.size = screen_get_size(i);
		Rect2 inters = r.intersection(p_rect);
		int area = inters.size.x * inters.size.y;
		if (area > nearest_area) {
			pos_screen = i;
			nearest_area = area;
		}
	}
	return pos_screen;
}
typedef struct {
	int count;
	int screen;
	HMONITOR monitor;
} EnumScreenData;
static BOOL CALLBACK _MonitorEnumProcScreen(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
	EnumScreenData *data = (EnumScreenData *)dwData;
	if (data->monitor == hMonitor) {
		data->screen = data->count;
	}

	data->count++;
	return TRUE;
}

int WindowSystem::get_keyboard_focus_screen() const {
	HWND hwnd = GetForegroundWindow();
	if (hwnd) {
		EnumScreenData data = { 0, 0, MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST) };
		EnumDisplayMonitors(nullptr, nullptr, _MonitorEnumProcScreen, (LPARAM)&data);
		return data.screen;
	} else {
		return get_primary_screen();
	}
}
static BOOL CALLBACK _MonitorEnumProcPrim(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
	EnumScreenData *data = (EnumScreenData *)dwData;
	if ((lprcMonitor->left == 0) && (lprcMonitor->top == 0)) {
		data->screen = data->count;
		return FALSE;
	}

	data->count++;
	return TRUE;
}

int WindowSystem::get_primary_screen() const {
	EnumScreenData data = { 0, 0, nullptr };
	EnumDisplayMonitors(nullptr, nullptr, _MonitorEnumProcPrim, (LPARAM)&data);
	return data.screen;
}
int WindowSystem::window_get_current_screen(WindowID p_window) const {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V(!m_windows.has(p_window), -1);

	EnumScreenData data = { 0, 0, MonitorFromWindow(m_windows[p_window].hWnd, MONITOR_DEFAULTTONEAREST) };
	EnumDisplayMonitors(nullptr, nullptr, _MonitorEnumProcScreen, (LPARAM)&data);
	return data.screen;
}
int WindowSystem::_get_screen_index(int p_screen) const {
	switch (p_screen) {
		case SCREEN_WITH_MOUSE_FOCUS: {
			const Rect2i rect = Rect2i(mouse_get_position(), Vector2i(1, 1));
			return get_screen_from_rect(rect);
		} break;
		case SCREEN_WITH_KEYBOARD_FOCUS: {
			return get_keyboard_focus_screen();
		} break;
		case SCREEN_PRIMARY: {
			return get_primary_screen();
		} break;
		case SCREEN_OF_MAIN_WINDOW: {
			return window_get_current_screen(MAIN_WINDOW_ID);
		} break;
		default: {
			return p_screen;
		} break;
	}
}

Rect2i WindowSystem::screen_get_usable_rect(int p_screen) const {
	_THREAD_SAFE_METHOD_

	p_screen = _get_screen_index(p_screen);
	EnumRectData data = { 0, p_screen, Rect2i() };
	EnumDisplayMonitors(nullptr, nullptr, _MonitorEnumProcUsableSize, (LPARAM)&data);
	data.rect.position -= _get_screens_origin();
	return data.rect;
}
void WindowSystem::window_set_vsync_mode(VSyncMode p_vsync_mode, WindowID p_window) {
	_THREAD_SAFE_METHOD_
	if (rendering_context) {
		rendering_context->window_set_vsync_mode(p_window, p_vsync_mode);
	}

	// switch (p_vsync_mode) {
	// case VSYNC_DISABLED:
	// case VSYNC_ADAPTIVE:
	// case VSYNC_MAILBOX:
	//     glfwSwapInterval(0);
	//     break;
	// case VSYNC_ENABLED:
	//     glfwSwapInterval(1);
	// }
}

} // namespace lain
