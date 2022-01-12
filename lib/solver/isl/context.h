#pragma once

#include <isl/ctx.h>
#include <isl/options.h>

#include <memory>
#include <unordered_map>
#include <string>
#include <exception>

namespace polly {

namespace isl {

class context {
 public:
  context() : d(new data()) { m_store.emplace(d.get()->ctx, d); }

  context(const context &other) : d(other.d) {}

  context(isl_ctx *ctx) {
    if (ctx == nullptr) return;

    auto iter = m_store.find(ctx);
    if (iter != m_store.end()) {
      d = iter->second.lock();
    } else {
      d = std::shared_ptr<data>(new data(ctx));
      m_store.emplace(ctx, d);
    }
  }

  isl_ctx *get() const { return d->ctx; }

 private:
  struct data {
    data(isl_ctx *ctx) : ctx(ctx) {}

    data() { ctx = isl_ctx_alloc(); }

    ~data() {
      context::m_store.erase(ctx);
      isl_ctx_free(ctx);
    }

    isl_ctx *ctx;
  };

  friend class data;

  std::shared_ptr<data> d;

  static std::unordered_map<isl_ctx *, std::weak_ptr<data>> m_store;
};

}  // namespace isl
}