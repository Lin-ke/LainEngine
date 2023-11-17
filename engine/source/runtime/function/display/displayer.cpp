#include "displayer.h"
Displayer* Displayer::p_singleton = nullptr;
// static 变量的初始化会在程序进入main之前进行。但是这些变量的初始化顺序是不能确定的。