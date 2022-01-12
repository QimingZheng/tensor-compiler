#pragma once

#include "common.h"
#include "context.h"

namespace polly {

namespace isl {

template <typename T>
struct object_behavior {};

template <typename T>
class object {
 public:
  virtual ~object() { object_behavior<T>::destroy(m_object); }

  const context& ctx() const { return m_ctx; }

  bool is_valid() const { return m_object != nullptr; }
  T* get() const { return m_object; }
  T* copy() const { return object_behavior<T>::copy(m_object); }

  void dump() const { object_behavior<T>::dump(m_object); }

  object<T>& operator=(const object<T>& other) {
    if (m_object != other.m_object) {
      object_behavior<T>::destroy(m_object);
      m_ctx = other.m_ctx;
      m_object = other.copy();
    }
    return *this;
  }

  object(const context& ctx, T* obj) : m_ctx(ctx), m_object(obj) {}

  object(const object<T>& other) : m_ctx(other.m_ctx), m_object(other.copy()) {}

  object(T* other_obj)
      : m_ctx(object_behavior<T>::get_context(other_obj)),
        m_object(other_obj) {}

  struct copy_of {};

  object(copy_of, T* p)
      : m_ctx(object_behavior<T>::get_context(p)),
        m_object(object_behavior<T>::copy(p)) {}

 protected:
  context m_ctx;
  T* m_object;
};

}
}