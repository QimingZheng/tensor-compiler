#include "context.h"

namespace polly {

namespace isl {

std::unordered_map<isl_ctx*, std::weak_ptr<context::data>> context::m_store;

}
}