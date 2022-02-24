#include "ir_module.h"
#include "ir_visitor.h"
#include "pass/transform/unroll.h"

namespace polly {

bool IRModule::CreateSplitSchedule(std::string axis) {
  auto it = std::find_if(splitSchedules.begin(), splitSchedules.end(),
                         [&](const std::string &ax) { return ax == axis; });
  if (it == splitSchedules.end()) {
    splitSchedules.push_back(axis);
    return true;
  }
  return false;
}
bool IRModule::CreateReorderSchedule(std::string axis1, std::string axis2) {
  auto it = std::find_if(reorderSchedules.begin(), reorderSchedules.end(),
                         [&](const std::pair<std::string, std::string> &ax) {
                           return ax.first == axis1 && ax.second == axis2;
                         });
  if (it == reorderSchedules.end()) {
    reorderSchedules.push_back({axis1, axis2});
    return true;
  }
  return false;
}
bool IRModule::CreateFuseSchedule(std::string axis1, std::string axis2) {
  auto it = std::find_if(fuseSchedules.begin(), fuseSchedules.end(),
                         [&](const std::pair<std::string, std::string> &ax) {
                           return ax.first == axis1 && ax.second == axis2;
                         });
  if (it == fuseSchedules.end()) {
    fuseSchedules.push_back({axis1, axis2});
    return true;
  }
  return false;
}

IRHandle IRModule::_find_loop_var(IRHandle cur, const std::string loop_var_id) {
  ForHandle curFor = cur.as<ForNode>();
  if (curFor == nullptr) return NullIRHandle;
  if (curFor->looping_var_.as<VarNode>()->id == loop_var_id) {
    return cur;
  }
  for (int i = 0; i < curFor->body.size(); i++) {
    if (curFor->body[i].Type() == IRNodeType::FOR) {
      IRHandle ret = _find_loop_var(curFor->body[i], loop_var_id);
      if (ret != NullIRHandle) return ret;
    }
  }
  return NullIRHandle;
}

IRHandle IRModule::find_loop_var(const std::string loop_var_id) {
  for (int i = 0; i < root_.as<FuncNode>()->body.size(); i++) {
    auto handle = root_.as<FuncNode>()->body[i];
    if (handle.Type() == IRNodeType::FOR) {
      auto ret = _find_loop_var(handle, loop_var_id);
      if (ret != NullIRHandle) return ret;
    }
  }
  return NullIRHandle;
}

class GetIRNodesHelper : public IRRecursiveVisitor {
 public:
  std::unordered_set<IRHandle, IRHandleHash> dict;

  void enter(IRHandle handle) override {
    dict.insert(handle);
    return;
  }
};

std::unordered_set<IRHandle, IRHandleHash> IRModule::GetIRNodes() {
  std::unordered_set<IRHandle, IRHandleHash> ret;
  GetIRNodesHelper helper;
  helper.visit(GetRoot());
  ret = helper.dict;
  return ret;
}

}  // namespace polly