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

void IRModule::evolveSplitSchedules(std::string axis, std::string axis1,
                                    std::string axis2) {
  {
    auto it = std::find_if(splitSchedules.begin(), splitSchedules.end(),
                           [&](const std::string &ax) { return ax == axis; });
    if (it != splitSchedules.end()) {
      splitSchedules.erase(it);
    }
    CreateSplitSchedule(axis1);
    CreateSplitSchedule(axis2);
  }
  {
    auto it = std::find_if(reorderSchedules.begin(), reorderSchedules.end(),
                           [&](const std::pair<std::string, std::string> &ax) {
                             return ax.first == axis || ax.second == axis;
                           });
    while (it != reorderSchedules.end()) {
      auto item = *it;
      reorderSchedules.push_back(
          {axis1, (item.first == axis) ? item.second : item.first});
      reorderSchedules.push_back(
          {axis2, (item.first == axis) ? item.second : item.first});
      it = std::find(reorderSchedules.begin(), reorderSchedules.end(), item);
      reorderSchedules.erase(it);
      it = std::find_if(reorderSchedules.begin(), reorderSchedules.end(),
                        [&](const std::pair<std::string, std::string> &ax) {
                          return ax.first == axis || ax.second == axis;
                        });
    }
  }
  {
    auto it = std::find_if(fuseSchedules.begin(), fuseSchedules.end(),
                           [&](const std::pair<std::string, std::string> &ax) {
                             return ax.first == axis || ax.second == axis;
                           });
    while (it != fuseSchedules.end()) {
      auto item = *it;
      fuseSchedules.push_back(
          {axis1, (item.first == axis) ? item.second : item.first});
      fuseSchedules.push_back(
          {axis2, (item.first == axis) ? item.second : item.first});
      it = std::find(fuseSchedules.begin(), fuseSchedules.end(), item);
      fuseSchedules.erase(it);
      it = std::find_if(fuseSchedules.begin(), fuseSchedules.end(),
                        [&](const std::pair<std::string, std::string> &ax) {
                          return ax.first == axis || ax.second == axis;
                        });
    }
  }
}

void IRModule::evolveReorderSchedules(std::string axis1, std::string axis2) {
  /// Pass
}

void IRModule::evolveFuseSchedules(std::string axis1, std::string axis2,
                                   std::string axis) {
  {
    auto it = std::find_if(
        splitSchedules.begin(), splitSchedules.end(),
        [&](const std::string &ax) { return ax == axis1 || ax == axis2; });
    while (it != splitSchedules.end()) {
      splitSchedules.erase(it);
      it = std::find_if(
          splitSchedules.begin(), splitSchedules.end(),
          [&](const std::string &ax) { return ax == axis1 || ax == axis2; });
    }
    CreateSplitSchedule(axis);
  }
  {
    auto it = std::find_if(reorderSchedules.begin(), reorderSchedules.end(),
                           [&](const std::pair<std::string, std::string> &ax) {
                             return ax.first == axis1 || ax.second == axis1 ||
                                    ax.first == axis2 || ax.second == axis2;
                           });
    while (it != reorderSchedules.end()) {
      auto item = *it;
      if (item.first != axis1 && item.first != axis2) {
        reorderSchedules.push_back({item.first, axis});
      }
      if (item.second != axis1 && item.second != axis2) {
        reorderSchedules.push_back({item.second, axis});
      }
      it = std::find(reorderSchedules.begin(), reorderSchedules.end(), item);
      reorderSchedules.erase(it);
      it = std::find_if(reorderSchedules.begin(), reorderSchedules.end(),
                        [&](const std::pair<std::string, std::string> &ax) {
                          return ax.first == axis1 || ax.second == axis1 ||
                                 ax.first == axis2 || ax.second == axis2;
                        });
    }
  }
  {
    auto it = std::find_if(fuseSchedules.begin(), fuseSchedules.end(),
                           [&](const std::pair<std::string, std::string> &ax) {
                             return ax.first == axis1 || ax.second == axis1 ||
                                    ax.first == axis2 || ax.second == axis2;
                           });
    while (it != fuseSchedules.end()) {
      auto item = *it;
      if (item.first != axis1 && item.first != axis2) {
        fuseSchedules.push_back({item.first, axis});
      }
      if (item.second != axis1 && item.second != axis2) {
        fuseSchedules.push_back({item.second, axis});
      }
      it = std::find(fuseSchedules.begin(), fuseSchedules.end(), item);
      fuseSchedules.erase(it);
      it = std::find_if(fuseSchedules.begin(), fuseSchedules.end(),
                        [&](const std::pair<std::string, std::string> &ax) {
                          return ax.first == axis1 || ax.second == axis1 ||
                                 ax.first == axis2 || ax.second == axis2;
                        });
    }
  }
}

bool IRModule::Reorder(const std::string i, const std::string j) {
  IRHandle i_loop = find_loop_var(i);
  IRHandle j_loop = find_loop_var(j);
  assert(i_loop != NullIRHandle);
  assert(j_loop != NullIRHandle);
  std::swap(i_loop.as<ForNode>()->looping_var_,
            j_loop.as<ForNode>()->looping_var_);
  evolveReorderSchedules(i, j);
  return true;
}

bool IRModule::Fuse(const std::string i, const std::string j,
                    const std::string fuse) {
  IRHandle outter_loop = find_loop_var(i);
  IRHandle inner_loop = find_loop_var(j);
  assert(outter_loop != NullIRHandle);
  assert(inner_loop != NullIRHandle);
  int inner_loop_pos = isNestedLoop(outter_loop, inner_loop);
  assert(inner_loop_pos >= 0);

  VarHandle outter = outter_loop.as<ForNode>()->looping_var_.as<VarNode>();
  VarHandle inner = inner_loop.as<ForNode>()->looping_var_.as<VarNode>();
  IRHandle outter_lower = outter->min, outter_upper = outter->max,
           outter_inc = outter->increment;
  IRHandle inner_lower = inner->min, inner_upper = inner->max,
           inner_inc = inner->increment;

  //   outter->name += inner->name;
  /// change the outter axis name to `fuse`
  outter->name = fuse;

  // get rid off the inner loop, now we only have the fused loop
  for (int i = 0; i < inner_loop.as<ForNode>()->body.size(); i++) {
    outter_loop.as<ForNode>()->body.insert(
        outter_loop.as<ForNode>()->body.begin() + inner_loop_pos++,
        inner_loop.as<ForNode>()->body[i]);
  }
  outter_loop.as<ForNode>()->body.erase(
      outter_loop.as<ForNode>()->body.begin() + inner_loop_pos);

  outter->min = IntNode::make(0);
  outter->max = MulNode::make(SubNode::make(outter_upper, outter_lower),
                              SubNode::make(inner_upper, inner_lower));
  outter->increment = MulNode::make(outter_inc, inner_inc);

  IRHandle common =
      MulNode::make(SubNode::make(inner_upper, outter_upper), outter_inc);
  IRMutatorVisitor outter_mutator(
      outter_loop.as<ForNode>()->looping_var_,
      AddNode::make(
          MulNode::make(
              outter_inc,
              DivNode::make(outter_loop.as<ForNode>()->looping_var_, common)),
          outter_lower));
  outter_mutator.visit(root_);

  IRMutatorVisitor inner_mutator(
      inner_loop.as<ForNode>()->looping_var_,
      AddNode::make(
          MulNode::make(
              inner_inc,
              ModNode::make(outter_loop.as<ForNode>()->looping_var_, common)),
          inner_lower));
  inner_mutator.visit(root_);

  evolveFuseSchedules(i, j, fuse);

  return true;
}

bool IRModule::Split(const std::string i, IRHandle tiles,
                     const std::string i_outter, const std::string i_inner) {
  IRHandle outter_loop = find_loop_var(i);
  assert(outter_loop != NullIRHandle);

  VarHandle originLoopVar =
      outter_loop.as<ForNode>()->looping_var_.as<VarNode>();

  //   originLoopVar->name = i + "_outter";
  // name the outter loop as `i_outter`
  originLoopVar->name = i_outter;
  IRHandle stride =
      DivNode::make(SubNode::make(originLoopVar->max, originLoopVar->min),
                    MulNode::make(tiles, originLoopVar->increment));
  IRHandle inner_loop =
      ForNode::make(VarNode::make(/* i + "_inner" */ i_inner, IntNode::make(0),
                                  stride, IntNode::make(1)) /* ,outter_loop */);

  originLoopVar->min = IntNode::make(0);
  originLoopVar->max = tiles;
  originLoopVar->increment = IntNode::make(1);

  std::swap(inner_loop.as<ForNode>()->body, outter_loop.as<ForNode>()->body);
  outter_loop.as<ForNode>()->Insert(inner_loop);

  // replace all the reference to `i` to `i_outter * tiles + i_inner`
  IRHandle intermediate = AddNode::make(
      MulNode::make(outter_loop.as<ForNode>()->looping_var_, stride),
      inner_loop.as<ForNode>()->looping_var_);

  IRMutatorVisitor mutator(outter_loop.as<ForNode>()->looping_var_,
                           intermediate);
  /// Root-Loop is a ForNode, so it cannot be replaced in any cases, so we
  /// always replace from the descendants of the Root-For-Loop-Node.

  mutator.visit(root_);
  evolveSplitSchedules(i, i_outter, i_inner);
  return true;
}

bool IRModule::Unroll() {
  LoopUnroll unroll(GetRoot());
  unroll.Transform();
}

int IRModule::isNestedLoop(IRHandle outter, IRHandle inner) {
  ForHandle outterFor = outter.as<ForNode>();
  ForHandle innerFor = inner.as<ForNode>();
  if (outterFor == nullptr) return false;
  if (innerFor == nullptr) return false;
  for (int i = 0; i < outterFor->body.size(); i++) {
    if (outterFor->body[i].Type() == IRNodeType::FOR) {
      if (outterFor->body[i].equals(inner)) return i;
    }
  }
  return -1;
}

IRHandle IRModule::_find_loop_var(IRHandle cur,
                                  const std::string loop_var_name) {
  ForHandle curFor = cur.as<ForNode>();
  if (curFor == nullptr) return NullIRHandle;
  if (curFor->looping_var_.as<VarNode>()->name == loop_var_name) {
    return cur;
  }
  for (int i = 0; i < curFor->body.size(); i++) {
    if (curFor->body[i].Type() == IRNodeType::FOR) {
      IRHandle ret = _find_loop_var(curFor->body[i], loop_var_name);
      if (ret != NullIRHandle) return ret;
    }
  }
  return NullIRHandle;
}

IRHandle IRModule::find_loop_var(const std::string loop_var_name) {
  for (int i = 0; i < root_.as<FuncNode>()->body.size(); i++) {
    auto handle = root_.as<FuncNode>()->body[i];
    if (handle.Type() == IRNodeType::FOR) {
      auto ret = _find_loop_var(handle, loop_var_name);
      if (ret != NullIRHandle) return ret;
    }
  }
  return NullIRHandle;
}

}  // namespace polly