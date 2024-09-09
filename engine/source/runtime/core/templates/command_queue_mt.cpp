#include "command_queue_mt.h"

void lain::CommandQueueMT::lock() {
	mutex.lock();
}

void lain::CommandQueueMT::unlock() {
	mutex.unlock();
}

lain::CommandQueueMT::CommandQueueMT() {
	command_mem.reserve(DEFAULT_COMMAND_MEM_SIZE_KB * 1024);
}
lain::CommandQueueMT::~CommandQueueMT() {
}
