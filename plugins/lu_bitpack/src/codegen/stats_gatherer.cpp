#include "codegen/stats_gatherer.h"
#include "codegen/serialization_item_list_ops/get_total_serialized_size.h"
#include "codegen/decl_descriptor.h"
#include "codegen/decl_dictionary.h"
#include "gcc_wrappers/type/container.h"
namespace gw {
   using namespace gcc_wrappers;
}

namespace codegen {
   
   //
   // count_context
   //
   
   void stats_gatherer::count_context::enter_array(const decl_descriptor& desc) {
      for(auto extent : desc.array.extents)
         this->containing_array_count *= extent;
   }
   void stats_gatherer::count_context::enter_array_slice(const std::vector<array_access_info>& slices, const decl_descriptor& desc) {
      for(const auto& slice : slices) {
         this->containing_array_count *= slice.count;
      }
      const auto&  ranks = desc.array.extents;
      const size_t from  = slices.size();
      for(size_t i = from; i < ranks.size(); ++i) {
         this->containing_array_count *= ranks[i];
      }
   }
   
   void stats_gatherer::count_context::on_single_serializable_seen(stats::serializable& info) const {
      info.counts.total += this->containing_array_count;
      info.add_count_in_sector(this->containing_sector_index, this->containing_array_count);
      if (this->containing_variable) {
         auto decl_name = this->containing_variable->name();
         info.add_count_in_top_level_identifier(decl_name, this->containing_array_count);
      }
   }
   
   //
   // stats_gatherer
   //
   
   void stats_gatherer::_seen_stats_category_annotation(const count_context& context, std::string_view name, const decl_descriptor& desc) {
      size_t count = context.containing_array_count;
      auto&  info  = this->categories[std::string(name)];
      context.on_single_serializable_seen(info);
      
      info.bitcounts.total_packed   += desc.serialized_type_size_in_bits() * count;
      info.bitcounts.total_unpacked += desc.unpacked_single_size_in_bits() * count;
   }
   
   void stats_gatherer::_gather_leaf(const count_context& context, const decl_descriptor& desc) {
      auto type = *desc.types.serialized;
      if (!type.is_container())
         return;
      for(const auto* memb : desc.members_of_serialized()) {
         auto memb_ctx = context;
         memb_ctx.enter_array(*memb);
         
         this->_seen_decl(memb_ctx, *memb);
         
         this->_gather_leaf(memb_ctx, *memb);
      }
   }
   
   void stats_gatherer::_seen_decl(const count_context& context, const decl_descriptor& desc) {
      const size_t count = context.containing_array_count;
      for(const auto& name : desc.options.stat_categories) {
         this->_seen_stats_category_annotation(context, name, desc);
      }
      
      //
      // Log stats for the non-transformed type.
      //
      auto type = *desc.types.innermost;
      if (type.is_container() && !type.name().empty()) {
         auto  pair = this->types.emplace(type, type);
         auto& info = pair.first->second;
         context.on_single_serializable_seen(info);
         info.bitcounts.total_packed   += desc.serialized_type_size_in_bits() * count;
         info.bitcounts.total_unpacked += info.c_info.size_of * 8 * count;
      }
   }
   
   void stats_gatherer::gather_from_sectors(const std::vector<std::vector<serialization_item>>& items_by_sector) {
      for(const auto& sector : items_by_sector) {
         auto  size = serialization_item_list_ops::get_total_serialized_size(sector);
         auto& dst  = this->sectors.emplace_back();
         dst.total_packed_size = size;
      }
      
      //
      // We need to count structs even if we don't serialize them directly, 
      // but rather serialize their members.
      //
      using stack_entry = serialization_items::basic_segment;
      std::vector<stack_entry> stack;
      //
      for(size_t i = 0; i < items_by_sector.size(); ++i) {
         const auto& sector = items_by_sector[i];
         for(const auto& item : sector) {
            if (item.is_padding())
               continue;
            if (item.is_defaulted && item.is_omitted)
               continue;
            
            if (!stack.empty()) {
               //
               // Check to see if we're accessing a different (potentially 
               // nested) DECL from the previous item.
               //
               size_t common_count = 0;
               size_t end          = (std::min)(stack.size(), item.segments.size());
               for(; common_count < end; ++common_count) {
                  const auto& segm = item.segments[common_count];
                  assert(segm.is_basic());
                  if (stack[common_count] != segm.as_basic())
                     break;
               }
               //
               // Remove any DECLs we're no longer inside of.
               //
               if (stack.size() > common_count)
                  stack.resize(common_count);
            }
            
            count_context context;
            context.containing_sector_index = i;
            {
               auto decl = item.segments.front().as_basic().desc->decl;
               assert(decl.is<gw::decl::variable>());
               context.containing_variable = decl.as<gw::decl::variable>();
            }
            for(const auto& entry : stack) {
               context.enter_array_slice(entry.array_accesses, *entry.desc);
               //this->_seen_decl(context, *entry.desc);
            }
            
            while (stack.size() < item.segments.size()) {
               size_t j = stack.size();
               const auto& segm = item.segments[j];
               assert(segm.is_basic());
               const auto& data = segm.as_basic();
               const auto* desc = data.desc;
               assert(desc != nullptr);
               stack.push_back(data);
               //
               // Update stats.
               //
               context.enter_array_slice(data.array_accesses, *desc);
               this->_seen_decl(context, *desc);
            }
            
            if (stack.empty()) {
               //
               // This item is empty.
               //
               continue;
            }
            
            //
            // See if we can expand the trailing segment.
            //
            // TODO: This will be wrong for opaque buffers: we do want to count 
            // nested members, but their serialized size would actually equal 
            // their non-packed size because... well, they aren't packed! They're 
            // in opaque buffers (i.e. memcpy).
            //
            const auto* desc = stack.back().desc;
            assert(desc != nullptr);
            this->_gather_leaf(context, *desc);
         }
      }
   }
}