//
// Created by YiMing D on 2023/1/23.
//
#include "scheduler.h"
#include "log.h"

static dreamer::Logger::ptr g_logger = DREAMER_SYSTEM_LOGGER();

void fiber_func() {
    D_SLOG_INFO(g_logger) << "协程开始运行: ";
    static int t = 10;
    if (--t >= 0) {
        dreamer::Scheduler::GetThis()->schedule(fiber_func);
    }
}

int main() {
    dreamer::Scheduler sc{3, true, "Sc"};
//    std::list<dreamer::Fiber::ptr> t;
//    t.emplace_back(new dreamer::Fiber(fiber_func));
//    t.emplace_back(new dreamer::Fiber(fiber_func));
    std::list<std::function<void()>> t;
    t.emplace_back(fiber_func);
    t.emplace_back(fiber_func);
    t.emplace_back(fiber_func);
    t.emplace_back(fiber_func);
//    sc.schedule(fiber_func, -1l);
    sc.start();
    sc.schedule(t.begin(), t.end());
    sc.stop();
}