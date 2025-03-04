#include "xmlgen/bitpacking_x_options_to_xml.h"
#include "bitpacking/data_options.h"
#include "gcc_wrappers/type/base.h"
namespace gw {
   using namespace gcc_wrappers;
}
namespace typed_options {
   using namespace bitpacking::typed_data_options::computed;
}

namespace xmlgen {
   extern void bitpacking_x_options_to_xml(
      xml_element& subject,
      const bitpacking::data_options& options,
      bool write_as_child_node
   ) {
      
      bool unknown = false;
      xml_element* x_opt = &subject;
      
      if (write_as_child_node) {
         if (options.is<typed_options::boolean>())
            return;
         if (options.is<typed_options::pointer>())
            return;
         if (options.is<typed_options::structure>())
            return;
         
         auto x_opt_ptr = std::make_unique<xml_element>();
         x_opt = x_opt_ptr.get();
         subject.append_child(std::move(x_opt_ptr));
         if (options.is<typed_options::buffer>()) {
            x_opt->node_name = "opaque-buffer-options";
         } else if (options.is<typed_options::integral>()) {
            x_opt->node_name = "integral-options";
         } else if (options.is<typed_options::string>()) {
            x_opt->node_name = "string-options";
         } else if (options.is<typed_options::tagged_union>()) {
            x_opt->node_name = "union-options";
            //
            // We'll need some additional attributes to preserve information 
            // that would've otherwise been in the node name.
            //
            x_opt->set_attribute_b("is-internal", options.as<typed_options::tagged_union>().is_internal);
         } else if (options.is<typed_options::transformed>()) {
            x_opt->node_name = "transform-options";
         } else {
            x_opt->node_name = "unknown-options";
            unknown = true;
         }
      } else {
         if (options.is_omitted) {
            subject.node_name = "omitted";
         } else {
            subject.node_name = "unknown";
            if (options.is<typed_options::boolean>()) {
               subject.node_name = "boolean";
            } else if (options.is<typed_options::buffer>()) {
               subject.node_name = "buffer";
            } else if (options.is<typed_options::integral>()) {
               subject.node_name = "integer";
            } else if (options.is<typed_options::pointer>()) {
               subject.node_name = "pointer";
            } else if (options.is<typed_options::string>()) {
               subject.node_name = "string";
            } else if (options.is<typed_options::structure>()) {
               subject.node_name = "struct";
            } else if (options.is<typed_options::tagged_union>()) {
               auto& casted = options.as<typed_options::tagged_union>();
               if (casted.is_internal)
                  subject.node_name = "union-internal-tag";
               else
                  subject.node_name = "union-external-tag";
            } else if (options.is<typed_options::transformed>()) {
               subject.node_name = "transformed";
            } else {
               subject.node_name = "unknown";
               unknown = true;
            }
         }
      }
      
      if (unknown)
         return;
      
      //
      // Write details about the specific x-options this entity has.
      //
      
      auto& node = *x_opt;
      if (options.is<typed_options::buffer>()) {
         const auto& casted = options.as<typed_options::buffer>();
         node.set_attribute_i("bytecount", casted.bytecount);
      } else if (options.is<typed_options::integral>()) {
         const auto& casted = options.as<typed_options::integral>();
         node.set_attribute_i("bitcount", casted.bitcount);
         if (casted.min != typed_options::integral::no_minimum) {
            node.set_attribute_i("min", casted.min);
         }
         if (casted.max != typed_options::integral::no_maximum) {
            node.set_attribute_i("max", casted.max);
         }
      } else if (options.is<typed_options::pointer>()) {
         ;
      } else if (options.is<typed_options::string>()) {
         const auto& casted = options.as<typed_options::string>();
         node.set_attribute_i("length",    casted.length);
         node.set_attribute_b("nonstring", casted.nonstring);
      } else if (options.is<typed_options::structure>()) {
         ;
      } else if (options.is<typed_options::tagged_union>()) {
         const auto& casted = options.as<typed_options::tagged_union>();
         node.set_attribute("tag", casted.tag_identifier);
      } else if (options.is<typed_options::transformed>()) {
         const auto& casted = options.as<typed_options::transformed>();
         if (auto opt = casted.transformed_type)
            node.set_attribute("transformed-type", opt->pretty_print());
         if (auto opt = casted.pre_pack)
            node.set_attribute("pack-function",    opt->name());
         if (auto opt = casted.post_unpack)
            node.set_attribute("unpack-function",  opt->name());
      }
   }
}