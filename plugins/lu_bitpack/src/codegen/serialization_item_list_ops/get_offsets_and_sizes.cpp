#include "codegen/serialization_item_list_ops/get_offsets_and_sizes.h"
#include <algorithm> // std::min

namespace {
   using offset_size_pair = std::pair<size_t, size_t>;

   using segment_condition = codegen::serialization_items::condition_type;

   struct overall_state {
      std::vector<offset_size_pair> data;
   };

   struct branch_state {
      overall_state*    overall;
      segment_condition condition;
      size_t offset = 0;
      
      branch_state() {} // needed for std::vector
      branch_state(overall_state& o) : overall(&o) {}
      
      void insert(const codegen::serialization_item& item) {
         size_t size = item.size_in_bits();
         overall->data.push_back({ this->offset, size });
         this->offset += size;
      }
      
      void catch_up_to(const branch_state& o) {
         assert(o.offset >= this->offset);
         this->offset = o.offset;
      }
   };
}

namespace codegen::serialization_item_list_ops {
   extern std::vector<offset_size_pair> get_offsets_and_sizes(const std::vector<serialization_item>& items) {
      overall_state overall;
      //
      branch_state root(overall);
      std::vector<branch_state> branches;
      
      //
      // We need to account for conditional serialization items, including ones 
      // that are conditioned on multiple comparisons (i.e. multiple conditional 
      // segments). To do this, we use "branched" state objects: we compare the 
      // conditions of the current item to those of the previous item, tracking 
      // the offsets of the current branch individually.
      //
      // We have an "overall" state that we use to hold parameters and results; 
      // a "branched" state for unconditional content; and then we'll create and  
      // destroy branched states for each set of conditions that we deal with.
      //
      
      for(auto& item : items) {
         //
         // Start by grabbing a list of all conditions that pertain to this item.
         //
         std::vector<segment_condition> conditions;
         for(auto& segment : item.segments)
            if (segment.condition.has_value())
               conditions.push_back(*segment.condition);
         //
         // Check to see if these conditions differ from those that applied to the 
         // previous item(s).
         //
         if (!branches.empty()) {
            size_t common = 0; // count, not index
            size_t end    = (std::min)(conditions.size(), branches.size());
            for(; common < end; ++common)
               if (conditions[common] != branches[common].condition)
                  break;
            
            while (branches.size() > common) {
               //
               // We are no longer subject to (all of) the conditions we were 
               // previously subject to.
               //
               bool same_lhs = false;
               if (conditions.size() >= branches.size()) {
                  auto& a = branches.back().condition;
                  auto& b = conditions[branches.size() - 1];
                  same_lhs = a.lhs == b.lhs;
               }
               if (same_lhs) {
                  //
                  // If multiple objects have conditions with the same LHS, then 
                  // those items go in the same space.
                  //
                  branches.pop_back();
               } else {
                  //
                  // The LHS has changed, so this item doesn't go into the same 
                  // space as the previous. We need to advance the positions of 
                  // those conditions that we're keeping, to move them past the 
                  // previous item's position.
                  //
                  auto  ended = branches.back();
                  branches.pop_back();
                  auto& still = branches.empty() ? root : branches.back();
                  still.catch_up_to(ended);
               }
            }
         }
         while (conditions.size() > branches.size()) {
            //
            // Conditions have been added or changed; start tracking bit positions 
            // for them.
            //
            size_t j    = branches.size();
            auto&  prev = branches.empty() ? root : branches.back();
            auto&  next = branches.emplace_back(overall);
            next.condition = conditions[j];
            next.offset    = prev.offset;
         }
         
         //
         // End of condition-handling logic.
         //
         
         auto& current_branch = branches.empty() ? root : branches.back();
         
         current_branch.insert(item);
      }
      if (!branches.empty()) {
         //
         // This may not be strictly necessary unless we'd like easy access to the 
         // total serialized size of the condition list. We're taking any still-open 
         // branches and catching the overall state up to them.
         //
         for(ssize_t i = branches.size() - 2; i >= 0; --i) {
            branches[i].catch_up_to(branches[i + 1]);
         }
         root.catch_up_to(branches[0]);
      }
      
      return overall.data;
   }
}