#include "codegen.h"

namespace polly {
std::string CodeGenC::C_Runtime_Deps = R"(
#include <iostream>
#include <ostream>
#include <fstream>
#include <assert.h>
#include <algorithm>
#include <assert.h>
#include <vector>
#include <stack>
#include <map>
#include <memory>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <utility>
#include <atomic>
#include <cstdlib>
#include <sys/time.h>
#include <random>
#include <math.h>
#include <thread>
#include <mutex>
#include <future>
#include <queue>
// #include <barrier>

class ThreadDim {
 public:
  ThreadDim(std::vector<size_t> dim) : dim_(dim) {
    sz = 1;
    for (int i = 0; i < dim.size(); i++) sz *= dim[i];
  }

  size_t size() { return sz; }
  size_t operator[](int x) { return dim_[x]; }

  size_t sz;
  std::vector<size_t> dim_;
};

class TaskQueue {
 public:
  TaskQueue() {}

  bool Empty() { return tasks_.empty(); }
  void Enqueue(std::function<void(void)> f) { tasks_.push(f); }

  bool Dequeue(std::function<void(void)> &f) {
    if (tasks_.empty()) return false;
    f = tasks_.front();
    tasks_.pop();
    return true;
  }

 private:
  std::queue<std::function<void(void)>> tasks_;
};

class MultiThreading {
 public:
  MultiThreading(int thread_num) : thread_num(thread_num) {
    running = true;
    for (int i = 0; i < thread_num; i++) {
      threads_.push_back(std::thread(&MultiThreading::run, this));
    }
  }

  ~MultiThreading() {
    std::unique_lock<std::mutex> lock(mtx);
    running = false;
    cv.notify_all();
    lock.unlock();

    for (int i = 0; i < thread_num; i++) {
      threads_[i].join();
    }
    threads_.clear();
  }

  void run() {
    while (running) {
      bool found = false;
      std::function<void(void)> f;
      {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !(running && tasks_.Empty()); });
        if (!running) break;
        found = tasks_.Dequeue(f);
      }
      if (found) {
        f();
      }
    }
  }

  template <typename F, typename... Args>
  auto submit(F &&f, Args &&...args) -> std::future<decltype(f(args...))> {
    std::function<decltype(f(args...))()> func =
        std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    auto task_ptr =
        std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

    std::function<void()> wrapper_func = [task_ptr]() { (*task_ptr)(); };

    std::unique_lock<std::mutex> lock(mtx);

    tasks_.Enqueue(wrapper_func);

    cv.notify_one();

    return task_ptr->get_future();
  }

  std::mutex mtx;
  std::condition_variable cv;

  std::vector<std::thread> threads_;
  TaskQueue tasks_;
  bool running;
  int thread_num;
};

class ThreadPool {
  ThreadPool(ThreadDim dim) : dim_(dim) {
    threadingLevels = new MultiThreading *[dim.size()];
    size_t mult = 1;
    for (int i = 0; i < dim.size(); i++) {
      threadingLevels[i] = new MultiThreading(mult);
      mult *= dim[i];
    }
  }

  MultiThreading **threadingLevels;

  ThreadDim dim_;

 public:
  static ThreadPool *GetInstance() { return pool; }

  static void Initialize(ThreadDim dim) { pool = new ThreadPool(dim); }

  static MultiThreading *GetThreadingLevel(int level) {
    return pool->threadingLevels[level];
  }

  static ThreadPool *pool;
};
ThreadPool *ThreadPool::pool = nullptr;

)";

}  // namespace poly