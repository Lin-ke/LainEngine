#pragma once
#ifndef __DISPLAYER_H__
#define __DISPLAYER_H__
// control the windows
// send event to the application.
class Displayer {
private:
	static Displayer* p_singleton;
public:
	Displayer() {
		p_singleton = this;
	}
	~Displayer() {
		p_singleton = nullptr;
	}

};

#endif // !__DISPLAYER_H__
