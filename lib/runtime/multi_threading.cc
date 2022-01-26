#include "multi_threading.h"
 
ThreadPool *ThreadPool::pool = nullptr;

void InitializeMultiThreading(std::vector<size_t> dim) {
  ThreadPool::Initialize(ThreadDim(dim));
}
