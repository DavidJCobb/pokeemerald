#pragma once
#include "codegen/instructions/base.h"
#include "codegen/instructions/union_switch.h"

namespace codegen::instructions::utils {
   template<typename Functor>
   void walk(Functor&& functor, instructions::base& node) {
      functor(node);
      if (auto* cont = node.as<instructions::container>()) {
         for(auto& child_ptr : cont->instructions)
            walk(functor, *child_ptr.get());
      } else if (auto* us = node.as<instructions::union_switch>()) {
         for(const auto& pair : us->cases)
            walk(functor, *pair.second.get());
      }
   }
   
   template<typename Functor>
   void walk(Functor&& functor, const instructions::base& node) {
      functor(node);
      if (auto* cont = node.as<instructions::container>()) {
         for(auto& child_ptr : cont->instructions)
            walk(functor, *child_ptr.get());
      } else if (auto* us = node.as<instructions::union_switch>()) {
         for(const auto& pair : us->cases)
            walk(functor, *pair.second.get());
      }
   }
}