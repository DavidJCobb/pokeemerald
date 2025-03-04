#include "xmlgen/bitpacking_default_value_to_xml.h"
#include "bitpacking/data_options.h"
#include "lu/stringf.h"
#include "gcc_wrappers/constant/floating_point.h"
#include "gcc_wrappers/constant/integer.h"
#include "gcc_wrappers/constant/string.h"
namespace gw {
   using namespace gcc_wrappers;
}

namespace xmlgen {
   extern void bitpacking_default_value_to_xml(
      xml_element& node,
      const bitpacking::data_options& options
   ) {
      if (!options.default_value)
         return;
      auto dv = *options.default_value;
      if (dv.is<gw::constant::string>()) {
         auto sub    = std::make_unique<xml_element>();
         sub->node_name = "default-value-string";
         sub->text_content = dv.as<gw::constant::string>().value();
         node.append_child(std::move(sub));
      } else if (dv.is<gw::constant::integer>()) {
         std::string text;
         {
            auto opt = dv.as<gw::constant::integer>().try_value_signed();
            if (opt.has_value()) {
               text = lu::stringf("%d", *opt);
            } else {
               text = "???";
            }
         }
         node.set_attribute("default-value", text);
      } else if (dv.is<gw::constant::floating_point>()) {
         auto text = dv.as<gw::constant::floating_point>().to_string();
         node.set_attribute("default-value", text);
      } else {
         node.set_attribute("default-value", "???");
      }
   }
}