#include <bit>
#include "bitpacking/data_options/typed.h"
#include "gcc_wrappers/decl/base_value.h"
#include "gcc_wrappers/decl/field.h"
#include "gcc_wrappers/type/array.h"
#include "gcc_wrappers/type/integral.h"
#include <diagnostic.h>
namespace gw {
   using namespace gcc_wrappers;
}

static gw::type::optional_base _get_innermost_value_type(gw::node node) {
   gw::type::optional_base value_type;
   if (node.is<gw::decl::base_value>()) {
      value_type = node.as<gw::decl::base_value>().value_type();
   } else if (node.is<gw::type::base>()) {
      value_type = node.as<gw::type::base>();
   }
   if (value_type) {
      while (value_type->is_array())
         value_type = value_type->as_array().value_type();
   }
   return value_type;
}

static location_t _loc(gw::node node) {
   if (node.is<gw::decl::base>()) {
      return node.as<gw::decl::base>().source_location();
   } else if (node.is<gw::type::base>()) {
      return node.as<gw::type::base>().source_location();
   }
   return UNKNOWN_LOCATION;
}

namespace bitpacking::typed_data_options::computed {
   
   //
   // buffer
   //
   
   bool buffer::load(gw::node target, const requested::buffer& src, bool complain) {
      auto type = _get_innermost_value_type(target);
      if (!type) {
         if (complain) {
            error_at(_loc(target), "failed to apply bitpacking options: we cannot figure out what value type we are applying them to, for some reason");
         }
         return false;
      }
      this->bytecount = type->size_in_bytes();
      return true;
   }
   
   //
   // integral
   //
   
   bool integral::load(gw::node target, const requested::integral& src, bool complain) {
      std::optional<size_t> bitwidth;
      if (target.is<gw::decl::field>()) {
         auto decl = target.as<gw::decl::field>();
         if (decl.is_bitfield())
            bitwidth = decl.size_in_bits();
      }
      
      if (src.bitcount.has_value()) {
         this->bitcount = *src.bitcount;
         if (src.min.has_value()) {
            this->min = *src.min;
         }
         if (src.max.has_value()) {
            this->max = *src.max;
         }
         return true;
      }
      if (src.min.has_value() && src.max.has_value()) {
         this->min = *src.min;
         this->max = *src.max;
         if (bitwidth.has_value()) {
            this->bitcount = *bitwidth;
         } else {
            this->bitcount = std::bit_width((uintmax_t)(this->max - this->min));
         }
         return true;
      }
      
      auto type = _get_innermost_value_type(target);
      if (!type) {
         if (complain) {
            error_at(_loc(target), "failed to apply bitpacking options: we cannot figure out what value type we are applying them to, for some reason, and we do not have enough information to know how many bits to pack this value into");
         }
         return false;
      }
      //
      // Compute the missing information from the type.
      //
      assert(type->is_integral()); // should've been verified by the attribute handlers
      if (bitwidth.has_value()) {
         this->bitcount = *bitwidth;
      }
      auto it = type->as_integral();
      if (src.min.has_value()) {
         this->min = *src.min;
         if (!bitwidth.has_value()) {
            this->bitcount = std::bit_width((uintmax_t)(it.maximum_value() - this->min));
         }
      } else if (src.max.has_value()) {
         this->max = *src.max;
         if (!bitwidth.has_value()) {
            this->bitcount = this->max - it.minimum_value();
         }
      } else {
         if (!bitwidth.has_value()) {
            this->bitcount = it.size_in_bits();
         }
      }
      return true;
   }
   
   //
   // string
   //
   
   bool string::load(gw::node target, const requested::string& src, bool complain) {
      if (src.nonstring.has_value())
         this->nonstring = *src.nonstring;
      
      gw::type::optional_array array_type;
      if (target.is<gw::decl::base_value>()) {
         auto type = target.as<gw::decl::base_value>().value_type();
         if (type.is_array())
            array_type = type.as_array();
      } else if (target.is<gw::type::array>()) {
         array_type = target.as<gw::type::array>();
      }
      if (!array_type) {
         if (complain) {
            error_at(_loc(target), "string bitpacking options cannot be applied to something that is not an array");
         }
         return false;
      }
      auto value_type = array_type->value_type();
      while (value_type.is_array()) {
         array_type = value_type.as_array();
         value_type = array_type->value_type();
      }
      
      auto ex_opt = array_type->extent();
      if (!ex_opt.has_value()) {
         if (complain) {
            error_at(_loc(target), "string bitpacking options cannot be applied to a variable-length array");
         }
         return false;
      }
      this->length = *ex_opt;
      if (!this->nonstring) {
         assert(this->length > 0); // should've been validated by the attribute handler
         --this->length; // we require a null terminator, which eats into the array length
      }
      return true;
   }
   
   //
   // tagged_union
   //
   
   bool tagged_union::load(gw::node target, const requested::tagged_union& src, bool complain) {
      if (src.is_external && src.is_internal) {
         if (complain) {
            error_at(_loc(target), "invalid bitpacking options for this tagged union: the tag cannot be both inside and outside of the union");
         }
         return false;
      }
      this->is_internal    = src.is_internal;
      this->tag_identifier = src.tag_identifier;
      return true;
   }
   
   //
   // transformed
   //
   
   bool transformed::load(gw::node target, const requested::transformed& src, bool complain) {
      this->transformed_type = src.transformed_type;
      this->pre_pack         = src.pre_pack;
      this->post_unpack      = src.post_unpack;
      if (src.never_split_across_sectors)
         this->never_split_across_sectors = true;
      
      auto type = _get_innermost_value_type(target);
      if (type && this->transformed_type && (*type).is_same(*this->transformed_type)) {
         if (complain) {
            error_at(_loc(target), "invalid bitpacking options for this to-be-transformed entity: transforming a value to the type it already has is nonsensical");
         }
         return false;
      }
      return true;
   }
}