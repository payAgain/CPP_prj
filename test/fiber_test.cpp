//
// Created by YiMing D on 2023/1/18.
//
#include "fiber.h"
#include "log.h"
#include "d_thread.h"

static dreamer::Logger::ptr g_logger = DREAMER_SYSTEM_LOGGER();
void func() {
    D_SLOG_INFO(g_logger) << "fiber begin";
    dreamer::Fiber::YieldToHold();
    D_SLOG_INFO(g_logger) << "fiber end";
    dreamer::Fiber::YieldToHold();
}

void func_thread() {
    dreamer::Fiber::GetThis();
//        auto p = new dreamer::Fiber(func);
    dreamer::Fiber::ptr fiber(new dreamer::Fiber(func));
    fiber->swapIn(); // 第一次进入协程
    D_SLOG_INFO(g_logger) << "main after swap in";
    fiber->swapIn(); // 第二次进入协程
    D_SLOG_INFO(g_logger) << "main after end";
    fiber->swapIn(); // 第三次进入协程
}

int main() {
    dreamer::Thread::SetThreadName("main");
    D_SLOG_INFO(g_logger) << "main start";
    std::vector<dreamer::Thread::ptr> threads;
    for (int i = 0; i < 3; ++i) {
        threads.push_back(std::shared_ptr<dreamer::Thread>(new dreamer::Thread(func_thread, "th" + std::to_string(i))));
    }
    for (int i = 0; i < 3; ++i) {
        threads[i]->join();
    }
    {   // 利用代码块析构fiber
//        dreamer::Fiber::GetThis();
////        auto p = new dreamer::Fiber(func);
//        dreamer::Fiber::ptr fiber(new dreamer::Fiber(func));
//        fiber->swapIn(); // 第一次进入协程
//        D_SLOG_INFO(g_logger) << "main after swap in";
//        fiber->swapIn(); // 第二次进入协程
//        D_SLOG_INFO(g_logger) << "main after end";
//        fiber->swapIn(); // 第三次进入协程
    }
    D_SLOG_INFO(g_logger) << "main end";

}