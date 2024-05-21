#include "event_loop.h"
EventLoop::EventLoop() {}

void EventLoop::event_process() {}

void EventLoop::add_io_event(int fd, int mask, io_call_back proc, void* args) {}

void EventLoop::del_io_event(int fd) {}

void EventLoop::del_io_event(int fd, int mask) {}
