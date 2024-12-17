#include "window_system.h"
#include "base.h"
#include "core/config/project_settings.h"
#include "function/render/rendering_device/rendering_device.h"
#include "platform/render/rendering_context_driver_vulkan_windows.h"
#include "function/render/renderer_rd/renderer_compositor_rd.h"
#include "core/input/input_event.h"
#include "platform/os/key_mapping_windows.h"
#include "core/input/input.h"

namespace lain {
WindowSystem *WindowSystem::p_singleton = nullptr;
int WindowSystem::m_windowid = WindowSystem::INVALID_WINDOW_ID;

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

static int _mouse_mode_to_glfw(WindowSystem::MouseMode p_mode){
	switch (p_mode) {
	case WindowSystem::MOUSE_MODE_VISIBLE:
		return GLFW_CURSOR_NORMAL;
	case WindowSystem::MOUSE_MODE_HIDDEN:
		return GLFW_CURSOR_HIDDEN;
	case WindowSystem::MOUSE_MODE_CAPTURED:
		return GLFW_CURSOR_DISABLED;
	case WindowSystem::MOUSE_MODE_CONFINED:
		return GLFW_CURSOR_DISABLED;
	case WindowSystem::MOUSE_MODE_CONFINED_HIDDEN:	
		return GLFW_CURSOR_HIDDEN;
	}
	return GLFW_CURSOR_NORMAL;
}
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
	id += 1; // start from 0
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
		wd.id = id;

		// 注意这里传入windowdata信息到context driver
		if (rendering_context) {
			union {
#ifdef VULKAN_ENABLED
				RenderingContextDriverVulkanWindows::WindowPlatformData vulkan;
#endif
#ifdef D3D12_ENABLED
				RenderingContextDriverD3D12::WindowPlatformData d3d12;
#endif
			} wpd;
#ifdef VULKAN_ENABLED
			if (rendering_driver == "vulkan") {
				wpd.vulkan.window = wd.hWnd;
				wpd.vulkan.instance = GetModuleHandleA(NULL);
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

		// 注册回调

		// 这样做肯定有些不对，因为不一定每个窗口都是这样的
		// registerOnResetFunc(std::bind(&WindowSystem::on_window_reset, this, std::placeholders::_1), id);
		registerOnCursorPosFunc(
				std::bind(&WindowSystem::on_cursor_pos, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3),id);
		registerOnCursorEnterFunc(
				std::bind(&WindowSystem::on_cursor_enter, this, std::placeholders::_1,std::placeholders::_2),id);
		registerOnScrollFunc(
				std::bind(&WindowSystem::on_scroll, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3),id);
		registerOnMouseButtonFunc(
				std::bind(&WindowSystem::on_mouse_button, this, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3,std::placeholders::_4),id);
		registerOnWindowCloseFunc(
				std::bind(&WindowSystem::on_window_close, this,std::placeholders::_1),id);
		registerOnKeyFunc(std::bind(&WindowSystem::on_key,
				this,
				std::placeholders::_1,
				std::placeholders::_2,
				std::placeholders::_3,
				std::placeholders::_4,
				std::placeholders::_5),id);

		m_windowid += 1;
	mouse_set_mode(m_mouse_mode);
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
	KeyMappingWindows::initialize();
	hInstance = GetModuleHandle(NULL);

	rendering_driver = p_rendering_driver;
	// 这个define应该在premake那里
#ifdef L_PLATFORM_WINDOWS
	if (rendering_driver == "vulkan") {
		rendering_context = memnew(RenderingContextDriverVulkanWindows);
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
		rendering_device = memnew(RenderingDevice);
		if (rendering_device->initialize(rendering_context, MAIN_WINDOW_ID) != OK) {
			memdelete(rendering_device);
			ERR_PRINT("Failed to initialize rendering device.");
			memdelete(rendering_device);
			rendering_device = nullptr;
			r_error = ERR_UNAVAILABLE;
			return;
		};
		rendering_device->screen_create(MAIN_WINDOW_ID); // swap chain

		RendererCompositorRD::make_current();
		// 在这里将_create_func 指向自己的memnew，这个函数将在
		// RendererCompositor::create()中调用 
		// 即在RenderingSystemDefault::_init() 时
		// 该函数在memnew(RenderingServerDefault)时调用， 在 main.cpp rendering_server = memnew(...)
		// 因此要求先进行 DisplayServer的初始化，再进行RenderingServer的构造

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
WindowSystem::VSyncMode WindowSystem::window_get_vsync_mode(WindowID p_window) const
{
	_THREAD_SAFE_METHOD_
	if (rendering_context) {
		return rendering_context->window_get_vsync_mode(p_window);
	}
	return VSYNC_DISABLED;
}

void WindowSystem::process_events() {
	_THREAD_SAFE_METHOD_
	
	Input::get_singleton()->flush_buffered_events();
	glfwPollEvents(); // 看起来是没buffer的，来一个回调一个，这里不知道什么

}

void WindowSystem::on_key(int id, int key, int scancode, int action, int p_mods) {
	GLFWwindow* window = m_windows[id].p_window;
	Ref<InputEventKey> k;
	k.instantiate();
	k->set_window_id(id);
	Key keycode = KeyMappingWindows::get_scansym(scancode,false);

	// scancode = (HIWORD(lParam) & (KF_EXTENDED | 0xff));
	if (keycode != Key::SHIFT) {
		int state = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) | glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT);
		k->set_shift_pressed(state);
	}
	if (keycode != Key::ALT) {
		int state = glfwGetKey(window, GLFW_KEY_LEFT_ALT) | glfwGetKey(window, GLFW_KEY_RIGHT_ALT);
		k->set_alt_pressed(state);
	}
	if (keycode != Key::CTRL) {
		int state = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) | glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL);
		k->set_ctrl_pressed(state);
	}
	if (keycode != Key::META) {
		int state = glfwGetKey(window, GLFW_KEY_LEFT_SUPER) | glfwGetKey(window, GLFW_KEY_RIGHT_SUPER);
		k->set_meta_pressed(state);
	}
	k->set_keycode(keycode);
	if(action == GLFW_PRESS) {
		k->set_pressed(true);
	} else if(action == GLFW_RELEASE) {
		k->set_pressed(false);
	}
	Input::get_singleton()->parse_input_event(k);
	// 解决shift 的问题
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

void WindowSystem::mouse_set_mode(MouseMode p_mode) {
	_THREAD_SAFE_METHOD_

	if (m_mouse_mode == p_mode) {
		return;
	}

	m_mouse_mode = p_mode;
	int mode = _mouse_mode_to_glfw(p_mode);
	for (const KeyValue<WindowID, WindowData> &E : m_windows) {
		glfwSetInputMode(E.value.p_window, GLFW_CURSOR, mode);
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

Size2i WindowSystem::window_get_size(WindowSystem::WindowID p_window) const {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V(!m_windows.has(p_window), Size2i());
	const WindowData &wd = m_windows[p_window];

	RECT r;
	if (GetWindowRect(wd.hWnd, &r)) { // Retrieves area inside of window border, including decoration.
		return Size2(r.right - r.left, r.bottom - r.top);
	}
	return Size2();
}
// keyboard godot的处理是送到缓存区里，再在 process_event 里处理
static BitField<WindowSystem::WinKeyModifierMask> _get_mods() {
	BitField<WindowSystem::WinKeyModifierMask> mask;
	static unsigned char keyboard_state[256];
	if (GetKeyboardState((PBYTE)&keyboard_state)) {
		if ((keyboard_state[VK_LSHIFT] & 0x80) || (keyboard_state[VK_RSHIFT] & 0x80)) {
			mask.set_flag(WindowSystem::WinKeyModifierMask::SHIFT);
		}
		if ((keyboard_state[VK_LCONTROL] & 0x80) || (keyboard_state[VK_RCONTROL] & 0x80)) {
			mask.set_flag(WindowSystem::WinKeyModifierMask::CTRL);
		}
		if ((keyboard_state[VK_LMENU] & 0x80) || (keyboard_state[VK_RMENU] & 0x80)) {
			mask.set_flag(WindowSystem::WinKeyModifierMask::ALT);
		}
		if ((keyboard_state[VK_RMENU] & 0x80)) {
			mask.set_flag(WindowSystem::WinKeyModifierMask::ALT_GR);
		}
		if ((keyboard_state[VK_LWIN] & 0x80) || (keyboard_state[VK_RWIN] & 0x80)) {
			mask.set_flag(WindowSystem::WinKeyModifierMask::META);
		}
	}

	return mask;
}

static BitField<MouseButtonMask> mouse_get_button_state() {
	BitField<MouseButtonMask> last_button_state = 0;

	if (GetKeyState(VK_LBUTTON) & (1 << 15)) {
		last_button_state.set_flag(MouseButtonMask::LEFT);
	}
	if (GetKeyState(VK_RBUTTON) & (1 << 15)) {
		last_button_state.set_flag(MouseButtonMask::RIGHT);
	}
	if (GetKeyState(VK_MBUTTON) & (1 << 15)) {
		last_button_state.set_flag(MouseButtonMask::MIDDLE);
	}
	if (GetKeyState(VK_XBUTTON1) & (1 << 15)) {
		last_button_state.set_flag(MouseButtonMask::MB_XBUTTON1);
	}
	if (GetKeyState(VK_XBUTTON2) & (1 << 15)) {
		last_button_state.set_flag(MouseButtonMask::MB_XBUTTON2);
	}

	return last_button_state;
}


void WindowSystem::on_cursor_pos(int id, double xpos, double ypos) {
	WindowID over_id = get_window_at_pos(mouse_get_position());
	if(window_mouseover_id != over_id)
	{	// mouse enter
				Input::CursorShape c = cursor_shape;
				cursor_shape = Input::CURSOR_ARROW;
				cursor_set_shape(c);
				window_mouseover_id = over_id;

				// Once-off notification, must call again.
				// track_mouse_leave_event(hWnd);
	}	
	if(m_mouse_mode == MOUSE_MODE_CAPTURED){
		return ;
	}
	const BitField<WinKeyModifierMask> &mods = _get_mods();
	// receiving_window_id 应该是最近一个被focus的窗口
	// 但是这里没有实现 @todo
	WindowID receiving_window_id = id;
	Ref<InputEventMouseMotion> mm;
	mm.instantiate();
	mm->set_window_id(receiving_window_id);
	mm->set_ctrl_pressed(mods.has_flag(WinKeyModifierMask::CTRL));
	mm->set_shift_pressed(mods.has_flag(WinKeyModifierMask::SHIFT));
	mm->set_alt_pressed(mods.has_flag(WinKeyModifierMask::ALT));
	mm->set_meta_pressed(mods.has_flag(WinKeyModifierMask::META));
	
	mm->set_button_mask(mouse_get_button_state());
	mm->set_position(Point2(xpos, ypos));
	mm->set_global_position(Point2(xpos, ypos));
	// 如果是hidden模式，glfw做了管理

	mm->set_velocity(Input::get_singleton()->get_last_mouse_velocity());
	mm->set_screen_velocity(mm->get_velocity());

}

void WindowSystem::on_cursor_enter(int id, int entered) {

}

void WindowSystem::on_scroll(int id, double xoffset, double yoffset) {}

void WindowSystem::on_drop(int id, int count, const char** paths) {}

void WindowSystem::on_window_size(int id, int width, int height) {}

void WindowSystem::on_window_close(int id) {}

WindowSystem::WindowID WindowSystem::get_window_at_pos(const Point2i& p_position) const {
	Point2i offset = _get_screens_origin();
	POINT p;
	p.x = p_position.x + offset.x;
	p.y = p_position.y + offset.y;
	HWND hwnd = WindowFromPoint(p);
	for (const KeyValue<WindowID, WindowData> &E : m_windows) {
		if (E.value.hWnd == hwnd) {
			return E.key;
		}
	}

	return INVALID_WINDOW_ID;
}

void WindowSystem::cursor_set_shape(Input::CursorShape p_shape)
{
		_THREAD_SAFE_METHOD_

	ERR_FAIL_INDEX(p_shape, Input::CURSOR_MAX);

	if (cursor_shape == p_shape) {
		return;
	}

	if (m_mouse_mode != MOUSE_MODE_VISIBLE && m_mouse_mode != MOUSE_MODE_CONFINED) {
		cursor_shape = p_shape;
		return;
	}

	static const LPCTSTR win_cursors[Input::CURSOR_MAX] = {
		IDC_ARROW,
		IDC_IBEAM,
		IDC_HAND, // Finger.
		IDC_CROSS,
		IDC_WAIT,
		IDC_APPSTARTING,
		IDC_SIZEALL,
		IDC_ARROW,
		IDC_NO,
		IDC_SIZENS,
		IDC_SIZEWE,
		IDC_SIZENESW,
		IDC_SIZENWSE,
		IDC_SIZEALL,
		IDC_SIZENS,
		IDC_SIZEWE,
		IDC_HELP
	};


	SetCursor(LoadCursor(hInstance, win_cursors[p_shape]));

	cursor_shape = p_shape;
}

void WindowSystem::on_char(int id, unsigned int codepoint) {}

void WindowSystem::on_char_mods(int id, unsigned int codepoint, int mods) {}

static BitField<MouseButtonMask> _get_button_mask(int button) {
	BitField<MouseButtonMask> mask;
	if(button & GLFW_MOUSE_BUTTON_LEFT) {
		mask.set_flag(MouseButtonMask::LEFT);
	}
	if(button & GLFW_MOUSE_BUTTON_RIGHT) {
		mask.set_flag(MouseButtonMask::RIGHT);
	}
	if(button & GLFW_MOUSE_BUTTON_MIDDLE) {
		mask.set_flag(MouseButtonMask::MIDDLE);
	}
	if(button & GLFW_MOUSE_BUTTON_LAST) {
		mask.set_flag(MouseButtonMask::MB_XBUTTON2);
	}
	return mask;
}
void WindowSystem::on_mouse_button(int id, int button, int action, int mod)
{ // 按下鼠标
// 左上角是(0,0)

	if(m_mouse_mode == MOUSE_MODE_CAPTURED){
		Ref<InputEventMouseMotion> mm;
		mm.instantiate();
		mm->set_window_id(id);
		mm->set_ctrl_pressed(mod & int32_t(WinKeyModifierMask::CTRL));
		mm->set_shift_pressed(mod & int32_t(WinKeyModifierMask::SHIFT));
		mm->set_alt_pressed(mod& int32_t(WinKeyModifierMask::ALT));
		mm->set_meta_pressed(mod& int32_t(WinKeyModifierMask::META));

		mm->set_pressure(1);
		mm->set_button_mask(_get_button_mask(button));
		double x, y;
		glfwGetCursorPos(m_windows[id].p_window, &x, &y);
		mm->set_position(Point2(x, y));
		Input::get_singleton()->parse_input_event(mm);
		L_PRINT(mm->to_string())
	}
	// 无事发生
}



}  // namespace lain
