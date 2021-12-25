#pragma once

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


namespace polly {

template <typename T>
void acquire(const T *);
template <typename T>
void release(const T *);

template <class T>
class IntrusivePtr {
 public:
  T *ptr;

  /// Allocate an undefined IntrusivePtr
  IntrusivePtr() : ptr(nullptr) {}

  /// Allocate an IntrusivePtr with an object
  IntrusivePtr(T *p) : ptr(p) {
    if (ptr) {
      acquire(ptr);
    }
  }

  /// Copy constructor
  IntrusivePtr(const IntrusivePtr &other) : ptr(other.ptr) {
    if (ptr) {
      acquire(ptr);
    }
  }

  /// Move constructor
  IntrusivePtr(IntrusivePtr &&other) : ptr(other.ptr) { other.ptr = nullptr; }

  /// Copy assignment operator
  IntrusivePtr &operator=(const IntrusivePtr &other) {
    if (ptr) {
      release(ptr);
    }
    ptr = other.ptr;
    if (ptr) {
      acquire(ptr);
    }
    return *this;
  }

  // Special function needed in pybind11 for custom smart pointers
  const T *get() { return ptr; }

  /// Copy assignment operator for managed object
  IntrusivePtr &operator=(T *p) {
    if (ptr) {
      release(ptr);
    }
    this->ptr = p;
    if (ptr) {
      acquire(ptr);
    }
    return *this;
  }

  /// Move assignment operator
  IntrusivePtr &operator=(IntrusivePtr &&other) {
    if (ptr) {
      release(ptr);
    }
    ptr = other.ptr;
    other.ptr = nullptr;
    return *this;
  }

  /// Destroy the intrusive ptr.
  virtual ~IntrusivePtr() {
    if (ptr) {
      release(ptr);
    }
  }

  /// Check whether the pointer is defined (ptr is not null).
  bool defined() const { return ptr != nullptr; }

  friend inline bool operator==(const IntrusivePtr<T> &p1,
                                const IntrusivePtr<T> &p2) {
    return p1.ptr == p2.ptr;
  }

  friend inline bool operator!=(const IntrusivePtr<T> &p1,
                                const IntrusivePtr<T> &p2) {
    return p1.ptr != p2.ptr;
  }

  friend inline bool operator<(const IntrusivePtr<T> &p1,
                               const IntrusivePtr<T> &p2) {
    return p1.ptr < p2.ptr;
  }

  friend inline bool operator>(const IntrusivePtr<T> &p1,
                               const IntrusivePtr<T> &p2) {
    return p1.ptr > p2.ptr;
  }

  friend inline bool operator<=(const IntrusivePtr<T> &p1,
                                const IntrusivePtr<T> &p2) {
    return p1.ptr <= p2.ptr;
  }

  friend inline bool operator>=(const IntrusivePtr<T> &p1,
                                const IntrusivePtr<T> &p2) {
    return p1.ptr >= p2.ptr;
  }
};

template <class Data>
class Manageable {
  friend void acquire(const Data *data) { ++data->ref; }
  friend void release(const Data *data) {
    if (--data->ref == 0) delete data;
  }

  mutable long ref = 0;
};

class Uncopyable {
 protected:
  Uncopyable() = default;
  ~Uncopyable() = default;

 private:
  Uncopyable(const Uncopyable &) = delete;
  Uncopyable &operator=(const Uncopyable &) = delete;
};

}  // namespace polly