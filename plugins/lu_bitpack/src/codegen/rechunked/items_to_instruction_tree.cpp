#include "codegen/rechunked/items_to_instruction_tree.h"
#include <algorithm> // std::min
#include "codegen/instructions/base.h"
#include "codegen/instructions/array_slice.h"
#include "codegen/instructions/padding.h"
#include "codegen/instructions/single.h"
#include "codegen/instructions/transform.h"
#include "codegen/instructions/union_switch.h"
#include "codegen/instructions/union_case.h"
#include "codegen/rechunked/chunks/array_slice.h"
#include "codegen/rechunked/chunks/base.h"
#include "codegen/rechunked/chunks/condition.h"
#include "codegen/rechunked/chunks/padding.h"
#include "codegen/rechunked/chunks/qualified_decl.h"
#include "codegen/rechunked/chunks/transform.h"
#include "codegen/rechunked/item.h"

namespace codegen::rechunked {
   
   //
   // A stack will typically contain one entry per chunk, pairing that chunk 
   // with the non-leaf node that it produced (if any).
   //
   // There is one exception: `condition` chunks produce up to two nodes: a 
   // "union switch" node and a "union case" node. Both are added to the stack. 
   // This is because we want to reuse "union switch" nodes when consecutive 
   // re-chunked items have conditions that compare the same LHS to different 
   // RHS values.
   //
   // There's also another detail worth noting: if an `array_slice` chunk has 
   // a count of 1, then we don't generate a node for it (but we do still add 
   // a stack entry). This ensures that we don't generate single-iteration `for` 
   // loops.
   //
   struct stack_entry {
      const chunks::base* chunk = nullptr;
      instructions::base* node  = nullptr;
   };
   
   // Take the stack and build a `value_path` representing the innermost node 
   // in that stack. This is "where we are:" it is the value that the next 
   // created node will be relevant to.
   //
   static value_path _current_value_for_stack(
      const std::vector<stack_entry>& stack,
      size_t end = std::numeric_limits<size_t>::max()
   ) {
      value_path value;
      
      end = (std::min)(end, stack.size());
      for(size_t i = 0; i < end; ++i) {
         const auto& entry = stack[i];
         
         if (const auto* casted = entry.chunk->as<rechunked::chunks::qualified_decl>()) {
            for(auto& desc : casted->descriptors)
               value.access_member(*desc);
            continue;
         }
         if (const auto* casted = entry.chunk->as<rechunked::chunks::array_slice>()) {
            if (entry.node != nullptr) {
               //
               // Slice. Use the loop counter.
               //
               auto* casted_node = entry.node->as<instructions::array_slice>();
               assert(casted_node != nullptr);
               value.access_array_element(casted_node->loop_index.descriptors);
            } else {
               //
               // Single element with fixed index. Use the index.
               //
               assert(casted->data.count == 1);
               value.access_array_element(casted->data.start);
            }
            continue;
         }
         
         assert(entry.node != nullptr);
         if (entry.node->as<instructions::union_switch>())
            continue;
         if (entry.node->as<instructions::union_case>())
            continue;
         
         if (const auto* casted = entry.node->as<instructions::transform>()) {
            value.replace(casted->transformed.descriptors);
            continue;
         }
         assert(!entry.node->as<instructions::array_slice>());
         assert(false && "unreachable");
      }
      return value;
   }
   
   extern std::unique_ptr<instructions::container> items_to_instruction_tree(
      const std::vector<item>& items
   ) {
      auto root = std::make_unique<instructions::container>();
      
      std::vector<stack_entry> stack;
      
      for(size_t i = 0; i < items.size(); ++i) {
         auto&  item      = items[i];
         size_t diverge_c = 0;
         if (i > 0) {
            auto& prev = items[i - 1];
            //
            // Let's compare the current item to the previous item, and pop any 
            // stack entries that no longer match.
            //
            size_t diverge_s = 0;
            size_t end       = (std::min)(item.chunks.size(), prev.chunks.size());
            for(; diverge_c < end; ++diverge_c, ++diverge_s) {
               auto& chunk_a_ptr = prev.chunks[diverge_c];
               auto& chunk_b_ptr = item.chunks[diverge_c];
               {
                  const auto* cnd_a = chunk_a_ptr->as<rechunked::chunks::condition>();
                  const auto* cnd_b = chunk_b_ptr->as<rechunked::chunks::condition>();
                  if (cnd_a && cnd_b) {
                     //
                     // We're comparing condition chunks. If the LHS and the RHS 
                     // both fail to match, then we want to pop the "switch" and 
                     // "case" nodes from the stack. We'll then later make a new 
                     // "switch" node and a new "case" node. However, if the LHS 
                     // compares equal, then we want to keep the "switch" node, 
                     // pop only the "case" node, and make a new "case" node 
                     // later on.
                     //
                     // The effect of this is that if two consecutive re-chunked 
                     // items compare the same LHS to different RHS values, we'll 
                     // reuse a single "switch" node for that LHS, and just make 
                     // the different "case" nodes for the RHS values.
                     //
                     if (cnd_a->compare_lhs(*cnd_b))
                        ++diverge_s;
                  }
               }
               if (!chunk_a_ptr->compare(*chunk_b_ptr))
                  break;
            }
            
            if (diverge_c < prev.chunks.size()) {
               stack.resize(diverge_s);
               if (!stack.empty()) {
                  //
                  // The stack should never end with a leaf node after we've clipped 
                  // off diverging entries. The only way that would ever happen is if 
                  // there were two consecutive QualifiedDecls in the re-chunked item, 
                  // which would imply a mistake made while re-chunking.
                  //
                  const auto* node = stack.back().node;
                  if (node) {
                     assert(node->as<instructions::container>() || node->as<instructions::union_switch>());
                  }
               }
            }
         }
         
         value_path value = _current_value_for_stack(stack);
         
         //
         // Spawn nodes onward from the point of divergence.
         //
         for(size_t j = diverge_c; j < item.chunks.size(); ++j) {
            instructions::base* parent = root.get();
            for(auto& entry : stack)
               if (entry.node)
                  if (entry.node->as<instructions::container>() || entry.node->as<instructions::union_switch>())
                     parent = entry.node;
            assert(parent != nullptr);
            
            const auto* chunk         = item.chunks[j].get();
            const bool  is_last_chunk = j == item.chunks.size() - 1;
            assert(chunk != nullptr);
            
            if (auto* casted = chunk->as<rechunked::chunks::condition>()) {
               instructions::union_switch* switch_node = nullptr;
               
               if (auto* casted_node = parent->as<instructions::union_switch>()) {
                  //
                  // We popped a "case" node off of the stack, but are reusing the 
                  // "switch" node; we just want to create a new "case."
                  //
                  switch_node = casted_node;
               } else {
                  //
                  // We're not reusing an existing "switch" node, so we need to make 
                  // one. First, we need to resolve the current chunk's condition 
                  // relative to the stack.
                  //
                  value_path lhs;
                  {
                     const auto& lhs_src = casted->lhs;
                     size_t common_c = 0; // number of chunks in common
                     size_t common_s = 0; // number of stack entries in common
                     //
                     // If we're branching on some member of an array of structs i.e. 
                     // `foo[0:2].bar == 2`, then we need to map the array index within 
                     // the condition to the generated VAR_DECL for our array loop.
                     //
                     // First, find a common stem between the condition and the current 
                     // stack.
                     //
                     for(; common_c < lhs_src.chunks.size(); ++common_s, ++common_c) {
                        if (common_c >= j)
                           break;
                        const auto* a_ptr = item.chunks[common_c].get();
                        const auto* b_ptr = lhs_src.chunks[common_c].get();
                        if (b_ptr->as<rechunked::chunks::condition>())
                           ++common_s;
                        if (!a_ptr->compare(*b_ptr))
                           break;
                     }
                     //
                     // Once we've found the common stem, access values via stack nodes.
                     //
                     lhs = _current_value_for_stack(stack, common_s);
                     //
                     // Finally, access values within the non-common tail of the condition.
                     //
                     for(; common_c < lhs_src.chunks.size(); ++common_c) {
                        const auto* chunk = lhs_src.chunks[common_c].get();
                        if (auto* casted = chunk->as<rechunked::chunks::qualified_decl>()) {
                           for(auto* desc : casted->descriptors)
                              lhs.access_member(*desc);
                           continue;
                        }
                        if (auto* casted = chunk->as<rechunked::chunks::array_slice>()) {
                           assert(casted->data.count == 1 && "unable to resolve loop variable for array slice in condition");
                           lhs.access_array_element(casted->data.start);
                           continue;
                        }
                        assert(false && "impossible node/chunk type here");
                     }
                  }
                  //
                  // Now that we've got our condition LHS, make the "switch" node.
                  //
                  auto node = std::make_unique<instructions::union_switch>();
                  node->condition_operand = lhs;
                  stack.push_back(stack_entry{
                     .chunk = chunk,
                     .node  = node.get(),
                  });
                  switch_node = node.get();
                  parent->as<instructions::container>()->instructions.push_back(std::move(node));
               }
               //
               // Make the "case" node.
               //
               auto node = std::make_unique<instructions::union_case>();
               stack.push_back(stack_entry{
                  .chunk = chunk,
                  .node  = node.get(),
               });
               if (casted->is_else) {
                  auto& uniq = switch_node->else_case;
                  assert(uniq.get() == nullptr);
                  uniq = std::move(node);
               } else {
                  auto& uniq = switch_node->cases[casted->rhs];
                  assert(uniq.get() == nullptr);
                  uniq = std::move(node);
               }
               continue;
            }
            //
            // Handle other chunk types.
            //
            assert(!parent->as<instructions::union_switch>());
            
            if (auto* casted = chunk->as<rechunked::chunks::array_slice>()) {
               if (casted->data.count == 1) {
                  //
                  // Don't create a node for a single array element, but do still create 
                  // a stack entry.
                  //
                  stack.push_back(stack_entry{
                     .chunk = chunk,
                     .node  = nullptr,
                  });
                  value.access_array_element(casted->data.start);
               } else {
                  auto node = std::make_unique<instructions::array_slice>();
                  node->array = {
                     .value = value,
                     .start = casted->data.start,
                     .count = casted->data.count,
                  };
                  value.access_array_element(node->loop_index.descriptors);
                  
                  stack.push_back(stack_entry{
                     .chunk = chunk,
                     .node  = node.get(),
                  });
                  parent->as<instructions::container>()->instructions.push_back(std::move(node));
               }
               //
               // Whether or not we generate a node, we have to check for the case of this 
               // being a leaf chunk:
               //
               if (is_last_chunk) {
                  //
                  // This is a non-expanded array. The `array_slice` node represents only 
                  // the for loop itself, so generate a `single` node to represent the 
                  // array elements.
                  //
                  if (auto* n = stack.back().node)
                     parent = n;
                  
                  auto node = std::make_unique<instructions::single>();
                  node->value = value;
                  parent->as<instructions::container>()->instructions.push_back(std::move(node));
               }
               continue;
            }
            if (auto* casted = chunk->as<rechunked::chunks::qualified_decl>()) {
               for(auto* desc : casted->descriptors)
                  value.access_member(*desc);
               
               if (is_last_chunk) {
                  //
                  // Only create a node for a `qualified_decl` chunk if the chunk is the 
                  // last chunk, to represent leaf nodes.
                  //
                  // Conditions will generally produce a path that looks like this:
                  //
                  //    a|data|(a.tag == 0) x
                  //
                  // Or, more straightforwardly:
                  //
                  //    if (a.tag == 0)
                  //       __stream(a.data.x);
                  //
                  // When that path is re-chunked, we end up with:
                  //
                  //  - qualified_decl: a.data
                  //  - condition:      a.tag == 0
                  //  - qualified_decl: x
                  //
                  // We need to stitch qualified_decl together to form the final leaf node 
                  // with path `a.data.x`.
                  //
                  auto node = std::make_unique<instructions::single>();
                  node->value = value;
                  parent->as<instructions::container>()->instructions.push_back(std::move(node));
               }
               stack.push_back(stack_entry{
                  .chunk = chunk,
                  .node  = nullptr,
               });
               continue;
            }
            if (auto* casted = chunk->as<rechunked::chunks::transform>()) {
               auto node = std::make_unique<instructions::transform>(casted->types);
               node->to_be_transformed_value = value;
               value.replace(node->transformed.descriptors);
               
               stack.push_back(stack_entry{
                  .chunk = chunk,
                  .node  = node.get(),
               });
               parent->as<instructions::container>()->instructions.push_back(std::move(node));
               
               if (is_last_chunk) {
                  //
                  // This is a leaf "transform" chunk, which means that the transformed 
                  // value will be serialized whole (rather than having individual fields 
                  // serialized one by one). The `transform` node represents only the 
                  // transformation itself, so generate a `single` node to represent the 
                  // array elements.
                  //
                  parent = stack.back().node;
                  
                  auto node = std::make_unique<instructions::single>();
                  node->value = value;
                  parent->as<instructions::container>()->instructions.push_back(std::move(node));
               }
               continue;
            }
            if (auto* casted = chunk->as<rechunked::chunks::padding>()) {
               assert(is_last_chunk);
               auto node = std::make_unique<instructions::padding>();
               node->bitcount = casted->bitcount;
               parent->as<instructions::container>()->instructions.push_back(std::move(node));
               stack.push_back(stack_entry{
                  .chunk = chunk,
                  .node  = nullptr,
               });
               continue;
            }
            
            assert(false && "unhandled chunk type");
         }
      }
      
      return root;
   }
}