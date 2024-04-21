#ifndef MAIN_LOOP_H
#define MAIN_LOOP_H
#include "core/object/object.h"
//#include "core/input/input_event.h"
//#include "core/object/gdvirtual.gen.inc"
#include "core/object/refcounted.h"
namespace lain {
	// processº¯Êý(tickº¯Êý)
class MainLoop : public Object {
	LCLASS(MainLoop, Object);

protected:
public:
	enum {
		//make sure these are replicated in Node
		NOTIFICATION_OS_MEMORY_WARNING = 2009,
		NOTIFICATION_TRANSLATION_CHANGED = 2010,
		NOTIFICATION_WM_ABOUT = 2011,
		NOTIFICATION_CRASH = 2012,
		NOTIFICATION_OS_IME_UPDATE = 2013,
		NOTIFICATION_APPLICATION_RESUMED = 2014,
		NOTIFICATION_APPLICATION_PAUSED = 2015,
		NOTIFICATION_APPLICATION_FOCUS_IN = 2016,
		NOTIFICATION_APPLICATION_FOCUS_OUT = 2017,
		NOTIFICATION_TEXT_SERVER_CHANGED = 2018,
	};

	virtual void initialize() {}
	virtual void iteration_prepare() {}
	virtual bool physics_process(double p_time) { return true; }
	virtual bool process(double p_time) { return false; }
	virtual void finalize() {}

	MainLoop() {}
	virtual ~MainLoop() {}
};
}

#endif // MAIN_LOOP_H