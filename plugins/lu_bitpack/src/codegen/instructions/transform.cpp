#include "codegen/instructions/transform.h"
#include "codegen/instructions/array_slice.h"
#include "codegen/instructions/single.h"
#include "codegen/decl_descriptor.h"
#include "codegen/decl_descriptor_pair.h"
#include "codegen/decl_dictionary.h"
#include "codegen/expr_pair.h"
#include "codegen/func_pair.h"
#include "lu/stringf.h"
#include "gcc_wrappers/expr/call.h"
#include "gcc_wrappers/expr/local_block.h"
#include "gcc_wrappers/builtin_types.h"
namespace gw {
   using namespace gcc_wrappers;
}
namespace typed_options {
   using namespace bitpacking::typed_data_options::computed;
}

namespace codegen::instructions {
   transform::transform(const std::vector<gw::type::base>& types)
   :
      types(types),
      transformed({
         .variables = {
            .read = gw::decl::variable(
               "__transformed_read",
               types.back()
            ),
            .save = gw::decl::variable(
               "__transformed_save",
               types.back()
            )
         },
      })
   {
      auto& dict = decl_dictionary::get();
      this->transformed.descriptors = decl_descriptor_pair{
         .read = &dict.describe(this->transformed.variables.read),
         .save = &dict.describe(this->transformed.variables.save),
      };
   }
   
   /*virtual*/ expr_pair transform::generate(const utils::generation_context& ctxt) const {
      auto& decl_dict = decl_dictionary::get();
      
      gw::expr::local_block block_read;
      gw::expr::local_block block_save;
      assert(!this->types.empty());
      
      auto statements_read = block_read.statements();
      auto statements_save = block_save.statements();
      
      optional_value_pair trans = this->to_be_transformed_value.as_value_pair();
      assert(!!trans.read);
      assert(!!trans.save);
      //
      struct transform_step {
         struct {
            optional_expr_pair   decl_exprs;
            decl_descriptor_pair desc_pair;
            optional_value_pair  values;
         } transformed_value;
         optional_expr_pair transform_call;
         optional_func_pair transform_func;
      };
      std::vector<transform_step> steps;
      //
      for(size_t i = 0; i < types.size(); ++i) {
         auto  type = this->types[i];
         auto  name = type.name();
         auto& step = steps.emplace_back();
         
         gw::decl::optional_variable var_read;
         gw::decl::optional_variable var_save;
         if (i == this->types.size() - 1) {
            var_read = this->transformed.variables.read;
            var_save = this->transformed.variables.save;
         } else {
            auto name_read = lu::stringf("__transformed_read_as_%s", name.data());
            auto name_save = lu::stringf("__transformed_save_as_%s", name.data());
            var_read = gw::decl::variable(name_read.c_str(), type);
            var_save = gw::decl::variable(name_save.c_str(), type);
         }
         step.transformed_value = {
            .decl_exprs = {
               .read = var_read->make_declare_expr(),
               .save = var_save->make_declare_expr(),
            },
            .desc_pair = {
               .read = &decl_dict.describe(*var_read),
               .save = &decl_dict.describe(*var_save),
            },
            .values = {
               var_read->as_value(),
               var_save->as_value(),
            },
         };
         //
         const auto& options = (
            i == 0 ?
               this->to_be_transformed_value.bitpacking_options()
            :
               steps[i - 1].transformed_value.desc_pair.read->options
         ).as<typed_options::transformed>();
         step.transform_func = func_pair(*options.post_unpack, *options.pre_pack);
         step.transform_call = optional_expr_pair{
            .read = gw::expr::call(
               *step.transform_func.read,
               // args:
               trans.read->address_of(), // in situ
               var_read->as_value().address_of() // transformed
            ),
            .save = gw::expr::call(
               *step.transform_func.save,
               // args:
               trans.save->address_of(), // in situ
               var_save->as_value().address_of() // transformed
            )
         };
         
         trans.read = var_read->as_value();
         trans.save = var_save->as_value();
      }
      
      //
      // Transformations on read need to be in reverse order. Consider:
      //
      //    {
      //       struct TransformedMiddle __transformed_1;
      //       struct TransformedFinal  __transformed_2;
      //       PackTransform(&original, &__transformed_1);
      //       PackTransform(&__transformed_1, &__transformed_2);
      //       
      //       __child_instructions(&__transformed_2);
      //    }
      //    
      //    {
      //       struct TransformedMiddle __transformed_1;
      //       struct TransformedFinal  __transformed_2;
      //       
      //       __child_instructions(&__transformed_2);
      //       
      //       UnpackTransform(&__transformed_1, &__transformed_2);
      //       UnpackTransform(&original, &__transformed_1);
      //    }
      //
      // It's a little more complicated though, if the transformation 
      // is split across multiple sectors. If we serialize only some 
      // fields in one sector, and then try to get the remaining fields 
      // in the next sctor, then the transformed object in the second 
      // sector would be missing the fields seen in the first sector -- 
      // and the call to UnpackTransform would clobber those fields 
      // with garbage.
      //
      // The solution to this is... scuffed. If we believe that we're 
      // working with a split value, then we can generate code like 
      // this:
      //
      //    {
      //       struct Transformed __transformed;
      //       PackTransform(&original, &__transformed);
      //       
      //       __serialize(&__transformed);
      //    }
      //    
      //    {
      //       struct Transformed __transformed;
      //       PackTransform(&original, &__transformed);
      //       
      //       __serialize(&__transformed);
      //       
      //       UnpackTransform(&original, &__transformed);
      //    }
      //
      // As long as the pre-pack and post-unpack functions are symmetric 
      // and the former is safe to invoke on a potentially incomplete 
      // object, the pack-read-unpack pattern will ensure that any fields 
      // we've already read are packed into the object, such that those 
      // fields are retained when we then unpack the object.
      //
      const bool read_needs_pre_pack = this->is_probably_split();
      
      // Append variable declarations.
      for(auto& step : steps) {
         statements_read.append(*step.transformed_value.decl_exprs.read);
         statements_save.append(*step.transformed_value.decl_exprs.save);
      }
      // Append pre-pack calls.
      for(size_t i = 0; i < steps.size(); ++i) {
         auto& step = steps[i];
         if (read_needs_pre_pack) {
            gw::value from = (i == 0) ?
               *this->to_be_transformed_value.as_value_pair().read
            :
               *steps[i - 1].transformed_value.values.read
            ;
            
            gw::value to = *step.transformed_value.values.read;
            
            statements_read.append(gw::expr::call(
               *step.transform_func.save,
               // args:
               from.address_of(), // in situ
               to.address_of() // transformed
            ));
         }
         statements_save.append(*step.transform_call.save);
      }
      // Append child instructions.
      for(auto& child_ptr : this->instructions) {
         auto pair = child_ptr->generate(ctxt);
         statements_read.append(pair.read);
         statements_save.append(pair.save);
      }
      // Append post-unpack calls.
      for(auto it = steps.rbegin(); it != steps.rend(); ++it) {
         auto& step = *it;
         statements_read.append(*step.transform_call.read);
      }
      
      return expr_pair(block_read, block_save);
   }
   
   bool transform::is_probably_split() const {
      if (this->instructions.empty())
         return false;
      if (this->instructions.size() != 1)
         return true;
      
      const auto* node = this->instructions[0].get();
      if (auto* casted = node->as<single>()) {
         return casted->value != this->to_be_transformed_value;
      }
      if (auto* casted = node->as<array_slice>()) {
         if (casted->array.value != this->to_be_transformed_value)
            return true;
      }
      
      return false;
   }
}