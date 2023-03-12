//
// Created by YiMing D on 2023/1/23.
//
#include "basic_log.h"
#include "fiber.h"
#include "log.h"
#include "scheduler.h"
#include <unistd.h>

static dreamer::Logger::ptr g_logger = DREAMER_SYSTEM_LOGGER();

void fiber_func() {
  D_SLOG_INFO(g_logger) << "协程开始运行: ";
  static int t = 10;
  while (--t >= 0) {
    D_SLOG_INFO(g_logger) << t;
  }
}

int main() {
  dreamer::Scheduler sc{1, false, "Sc"};
  std::list<std::function<void()>> t;
  t.emplace_back(fiber_func);
  sc.schedule(t.begin(), t.end());
  sc.start();
  sc.stop();
}