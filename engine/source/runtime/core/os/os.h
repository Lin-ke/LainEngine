#pragma once
#ifndef __OS_H__
#define __OS_H__
#include "base.h"
#include "core/config/project_settings.h"
#include "core/os/main_loop.h"
#include "memory.h"
#include "thread_safe.h"
#include "time_enums.h"
#define ForwardRenderingMethodName "forward"
#define CompatiblilityRenderingMethodName "gl_compatiblitity"
#define VulkanDriver "vulkan"
#define OpenGLDriver "opengl"
// abstruct of the OS
namespace lain {

class OS {
private:
	// for the user interface we keep a record of the current display driver
	// so we can retrieve the rendering drivers available
	int _display_driver_id = -1;
	String _current_rendering_driver_name;
	String _current_rendering_method;
	static OS *p_singleton;

public:

	String m_execpath;
	virtual void Run() = 0;
	virtual void Finialize() = 0;
	virtual void Initialize() = 0;
	virtual void SetMainLoop(MainLoop *p_main_loop) = 0;

	// logger default

	virtual void Addlogger() {}
	OS() {
		p_singleton = this;
	}
	///*******TIME ********** */
	///*******TIME ********** */
	///*******TIME ********** */

	struct DateTime {
		int64_t year;
		Month month;
		uint8_t day;
		Weekday weekday;
		uint8_t hour;
		uint8_t minute;
		uint8_t second;
		bool dst;
	};

	virtual ui64 GetTimeUsec() const = 0;
	virtual ui64 GetTicksUsec() const = 0;
	virtual double GetUnixTime() const { return 0; }
	virtual DateTime GetDateTime(bool p_utc = false) const = 0;
	///*******TIME ********** */
	///*******TIME ********** */
	///*******TIME ********** */

	// Absolute path to res:// ( if not reload)
	virtual String GetResourceDir() const;

	// OS specific path for user://
	virtual String GetUserDataDir() const;
	virtual String GetCachePath() const;

	virtual String GetExecutablePath() const { return m_execpath; }

	static L_INLINE OS *GetSingleton() {
		return p_singleton;
	}
	virtual void DelayUsec(uint32_t p_usec) const = 0;
	virtual String GetDataPath() const { return "/"; }

	virtual String GetConfigPath() const { return "."; }
	virtual Error SetCwd(const String &path) { return ERR_CANT_OPEN; }
	// 加密
	virtual Error GetEntropy(uint8_t *r_buffer, size_t p_bytes) const = 0;

	virtual void SetEnvironment(const String &, const String &) const = 0;
	String GetSafeDirName(const String &p_dir_name, bool p_allow_paths) const;
	int GetProcessorCount() const;
	int GetDefaultThreadPoolSize() const { return GetProcessorCount(); }
	String GetCurrentRenderingMethod() const { return _current_rendering_method; }

	void EnsureUserDataDir();
	String GetCurrentRenderingDriverName() const { return _current_rendering_driver_name; }
	bool is_gl_rd() const {return _current_rendering_driver_name.begins_with("opengl"); }
	bool is_vulkan_rd() const {return _current_rendering_driver_name.begins_with("vulkan"); }
	bool is_gl_rdm() const {return _current_rendering_method == CompatiblilityRenderingMethodName; }
	bool is_forward_rdm() const {return _current_rendering_method == ForwardRenderingMethodName; }
	void set_current_rendering_driver_name(const String &p_driver_name) { _current_rendering_driver_name = p_driver_name; }
	void set_current_rendering_method(const String &p_name) { _current_rendering_method = p_name; }
	enum RenderThreadMode {
		RENDER_THREAD_UNSAFE,
		RENDER_THREAD_SAFE,
		RENDER_SEPARATE_THREAD
	};
	RenderThreadMode _render_thread_mode = RENDER_THREAD_SAFE;
	RenderThreadMode get_render_thread_mode() const { return _render_thread_mode; }
	void set_render_thread_mode(RenderThreadMode p_mode) { _render_thread_mode = p_mode; }
	// need virtual
	virtual ~OS() {
		p_singleton = nullptr;
	}
};

} //namespace lain

// It is customary to add virtual in subclasses to make it clearer ( no is OK)
#endif // !__OS_H__
