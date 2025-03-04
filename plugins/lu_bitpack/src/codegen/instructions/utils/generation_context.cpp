#include "lu/stringf.h"
#include <memory> // GCC headers fuck up string-related STL identifiers that <memory> depends on
#include "codegen/instructions/utils/generation_context.h"
#include "gcc_wrappers/decl/field.h"
#include "gcc_wrappers/decl/function.h"
#include "gcc_wrappers/decl/result.h"
#include "gcc_wrappers/expr/local_block.h"
#include "gcc_wrappers/type/function.h"
#include "gcc_wrappers/builtin_types.h"
#include "bitpacking/global_options.h"
#include "basic_global_state.h"
#include "codegen/instructions/utils/walk.h"
#include "codegen/instructions/array_slice.h"
#include "codegen/instructions/base.h"
#include "codegen/instructions/single.h"
#include "codegen/instructions/transform.h"
#include "codegen/instructions/union_switch.h"
#include "codegen/serialization_item_list_ops/fold_sequential_array_elements.h"
#include "codegen/serialization_item_list_ops/force_expand_unions_and_anonymous.h"
#include "codegen/serialization_item_list_ops/force_expand_omitted_and_defaulted.h"
#include "codegen/decl_descriptor.h"
#include "codegen/decl_dictionary.h"
#include "codegen/rechunked/item.h"
#include "codegen/rechunked/items_to_instruction_tree.h"
#include "codegen/serialization_item.h"
#include "codegen/whole_struct_function_dictionary.h"
#include "codegen/whole_struct_function_info.h"
namespace gw {
   using namespace gcc_wrappers;
}

namespace codegen::instructions::utils {
   whole_struct_function_info generation_context::_make_whole_struct_functions_for(gw::type::record type) const {
      const auto& ty = gw::builtin_types::get_fast();
      auto& decl_dict = decl_dictionary::get();
      
      auto result = func_pair(
         gw::decl::function(
            lu::strings::zview(lu::stringf("__lu_bitpack_read_%s", type.name().data())),
            gw::type::function(
               ty.basic_void,
               // args:
               *basic_global_state::get().global_options.types.bitstream_state_ptr,
               type.add_pointer()
            )
         ),
         gw::decl::function(
            lu::strings::zview(lu::stringf("__lu_bitpack_save_%s", type.name().data())),
            gw::type::function(
               ty.basic_void,
               // args:
               *basic_global_state::get().global_options.types.bitstream_state_ptr,
               type.add_const().add_pointer()
            )
         )
      );
      result.read.as_modifiable().set_result_decl(gw::decl::result(ty.basic_void));
      result.save.as_modifiable().set_result_decl(gw::decl::result(ty.basic_void));
      
      result.read.nth_parameter(0).make_used();
      result.save.nth_parameter(0).make_used();
      result.read.nth_parameter(1).make_used();
      result.save.nth_parameter(1).make_used();
      
      std::unique_ptr<instructions::base> root;
      //
      // Generate the tree of nodes for reads based on the PARM_DECL for the 
      // "read" function's in-struct argument. Then, update the tree's "save" 
      // descriptor pointers to poit to the PARM_DECL for "save."
      //
      {
         std::vector<serialization_item> si;
         {
            serialization_item item;
            auto& segm = item.segments.emplace_back();
            auto& data = segm.data.emplace<serialization_items::basic_segment>();
            data.desc = &decl_dict.dereference_and_describe(result.read.nth_parameter(1));
            si = item.expanded();
            
            serialization_item_list_ops::fold_sequential_array_elements(si);
            serialization_item_list_ops::force_expand_unions_and_anonymous(si);
            serialization_item_list_ops::force_expand_omitted_and_defaulted(si);
         }
         std::vector<rechunked::item> ri;
         {
            size_t size = si.size();
            ri.resize(size);
            for(size_t i = 0; i < size; ++i)
               ri[i] = rechunked::item(si[i]);
         }
         root = rechunked::items_to_instruction_tree(ri);
      }
      //
      // Do the update.
      //
      {
         const auto& desc_read = decl_dict.dereference_and_describe(result.read.nth_parameter(1));
         const auto& desc_save = decl_dict.dereference_and_describe(result.save.nth_parameter(1));
         auto _fix_value = [&desc_read, &desc_save](value_path& path) {
            if (path.segments.empty())
               return;
            auto& segm = path.segments.front();
            assert(!segm.is_array_access());
            auto& pair = segm.member_descriptor();
            if (pair.save == &desc_read) {
               pair.save = &desc_save;
            }
         };
         
         walk(
            [&_fix_value](instructions::base& node) {
               if (auto* casted = node.as<instructions::array_slice>()) {
                  _fix_value(casted->array.value);
                  return;
               }
               if (auto* casted = node.as<instructions::single>()) {
                  _fix_value(casted->value);
                  return;
               }
               if (auto* casted = node.as<instructions::transform>()) {
                  _fix_value(casted->to_be_transformed_value);
                  return;
               }
               if (auto* casted = node.as<instructions::union_switch>()) {
                  _fix_value(casted->condition_operand);
                  return;
               }
            },
            *root.get()
         );
      }
      
      //
      // Now we can actually generate code from that node hierarchy.
      //
      generation_context context = *this;
      context.state_ptr = optional_value_pair(
         result.read.nth_parameter(0).as_value(),
         result.save.nth_parameter(0).as_value()
      );
      auto root_expr = root->generate(context);
      
      if (!root_expr.read.is<gw::expr::local_block>()) {
         gw::expr::local_block block;
         block.statements().append(root_expr.read);
         root_expr.read = block;
      }
      if (!root_expr.save.is<gw::expr::local_block>()) {
         gw::expr::local_block block;
         block.statements().append(root_expr.save);
         root_expr.save = block;
      }
      
      result.read.set_is_defined_elsewhere(false);
      result.save.set_is_defined_elsewhere(false);
      {
         auto block_read = root_expr.read.as<gw::expr::local_block>();
         auto block_save = root_expr.save.as<gw::expr::local_block>();
         result.read.as_modifiable().set_root_block(block_read);
         result.save.as_modifiable().set_root_block(block_save);
      }
      
      // expose these identifiers so we can inspect them with our debug-dump pragmas.
      // (in release builds we'll likely want to remove this, so user code can't call 
      // these functions or otherwise access them directly.)
      result.read.introduce_to_current_scope();
      result.save.introduce_to_current_scope();
      
      whole_struct_function_info info;
      info.functions         = result;
      info.instructions_root = std::move(root);
      return info;
   }
   
   func_pair generation_context::get_whole_struct_functions_for(gw::type::record type) const {
      auto pair = this->whole_struct_functions.get_functions_for(type);
      if (!pair.read) {
         assert(!pair.save);
         auto info = this->_make_whole_struct_functions_for(type);
         pair = info.functions;
         this->whole_struct_functions.add_functions_for(type, std::move(info));
      } else {
         assert(!pair.save.empty());
      }
      return pair;
   }
}