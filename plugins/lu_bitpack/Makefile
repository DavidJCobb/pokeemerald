PLUGIN_NAME=lu_bitpack
PLUGIN=$(PLUGIN_NAME).so
INCDIR=include

include Makefile.gcc-wrappers
SOURCES=\
        $(GCC_WRAPPERS_SOURCES) \
        src/attribute_handlers/helpers/bp_attr_context.cpp \
        src/attribute_handlers/helpers/type_transitively_has_attribute.cpp \
        src/attribute_handlers/bitpack_as_opaque_buffer.cpp \
        src/attribute_handlers/bitpack_bitcount.cpp \
        src/attribute_handlers/bitpack_default_value.cpp \
        src/attribute_handlers/bitpack_misc_annotation.cpp \
        src/attribute_handlers/bitpack_range.cpp \
        src/attribute_handlers/bitpack_stat_category.cpp \
        src/attribute_handlers/bitpack_string.cpp \
        src/attribute_handlers/bitpack_tagged_id.cpp \
        src/attribute_handlers/bitpack_transforms.cpp \
        src/attribute_handlers/bitpack_union_external_tag.cpp \
        src/attribute_handlers/bitpack_union_internal_tag.cpp \
        src/attribute_handlers/generic_bitpacking_data_option.cpp \
        src/attribute_handlers/generic_type_or_decl.cpp \
        src/attribute_handlers/test.cpp \
        src/attribute_handlers/no_op.cpp \
        src/bitpacking/attribute_attempted_on.cpp \
        src/bitpacking/data_options/typed.cpp \
        src/bitpacking/data_options.cpp \
        src/bitpacking/get_union_bitpacking_info.cpp \
        src/bitpacking/global_options.cpp \
        src/bitpacking/mark_for_invalid_attributes.cpp \
        src/bitpacking/requested_global_options.cpp \
        src/bitpacking/transform_function_validation_helpers.cpp \
        src/bitpacking/verify_bitpack_attributes_on_type_finished.cpp \
        src/bitpacking/verify_union_external_tag.cpp \
        src/bitpacking/verify_union_internal_tag.cpp \
        src/bitpacking/verify_union_members.cpp \
        src/codegen/debugging/print_sectored_rechunked_items.cpp \
        src/codegen/debugging/print_sectored_serialization_items.cpp \
        src/codegen/instructions/utils/generation_context.cpp \
        src/codegen/instructions/utils/tree_stringifier.cpp \
        src/codegen/instructions/array_slice.cpp \
        src/codegen/instructions/base.cpp \
        src/codegen/instructions/padding.cpp \
        src/codegen/instructions/single.cpp \
        src/codegen/instructions/transform.cpp \
        src/codegen/instructions/union_switch.cpp \
        src/codegen/serialization_item_list_ops/divide_items_by_sectors.cpp \
        src/codegen/serialization_item_list_ops/fold_sequential_array_elements.cpp \
        src/codegen/serialization_item_list_ops/force_expand_but_values_only.cpp \
        src/codegen/serialization_item_list_ops/force_expand_omitted_and_defaulted.cpp \
        src/codegen/serialization_item_list_ops/force_expand_unions_and_anonymous.cpp \
        src/codegen/serialization_item_list_ops/get_offsets_and_sizes.cpp \
        src/codegen/serialization_item_list_ops/get_total_serialized_size.cpp \
        src/codegen/serialization_items/basic_segment.cpp \
        src/codegen/serialization_items/condition.cpp \
        src/codegen/rechunked/chunks/array_slice.cpp \
        src/codegen/rechunked/chunks/condition.cpp \
        src/codegen/rechunked/chunks/padding.cpp \
        src/codegen/rechunked/chunks/qualified_decl.cpp \
        src/codegen/rechunked/chunks/transform.cpp \
        src/codegen/rechunked/item.cpp \
        src/codegen/rechunked/item_to_string.cpp \
        src/codegen/rechunked/items_to_instruction_tree.cpp \
        src/codegen/stats/c_type.cpp \
        src/codegen/stats/category.cpp \
        src/codegen/stats/sector.cpp \
        src/codegen/stats/serializable.cpp \
        src/codegen/decl_descriptor.cpp \
        src/codegen/decl_dictionary.cpp \
        src/codegen/describe_and_check_decl_tree.cpp \
        src/codegen/expr_pair.cpp \
        src/codegen/func_pair.cpp \
        src/codegen/generation_request.cpp \
        src/codegen/generation_result.cpp \
        src/codegen/optional_value_pair.cpp \
        src/codegen/serialization_item.cpp \
        src/codegen/stats_gatherer.cpp \
        src/codegen/value_path.cpp \
        src/codegen/whole_struct_function_dictionary.cpp \
        src/gcc_helpers/c/at_file_scope.cpp \
        src/gcc_helpers/identifier_path.cpp \
        src/gcc_helpers/stringify_function_signature.cpp \
        src/lu/strings/builder.cpp \
        src/lu/strings/handle_kv_string.cpp \
        src/lu/strings/trim.cpp \
        src/lu/stringf.cpp \
        src/pragma_handlers/helpers/parse_c_serialization_item.cpp \
        src/pragma_handlers/debug_dump_bp_data_options.cpp \
        src/pragma_handlers/debug_dump_function.cpp \
        src/pragma_handlers/debug_dump_identifier.cpp \
        src/pragma_handlers/enable.cpp \
        src/pragma_handlers/generate_functions.cpp \
        src/pragma_handlers/serialized_offset_to_constant.cpp \
        src/pragma_handlers/serialized_sector_id_to_constant.cpp \
        src/pragma_handlers/set_options.cpp \
        src/xmlgen/bitpacking_default_value_to_xml.cpp \
        src/xmlgen/bitpacking_x_options_to_xml.cpp \
        src/xmlgen/integral_type_index.cpp \
        src/xmlgen/is_valid_name.cpp \
        src/xmlgen/referenceable_struct_members_to_xml.cpp \
        src/xmlgen/report_generator.cpp \
        src/xmlgen/string_as_cdata.cpp \
        src/xmlgen/write_string_as_attribute_value.cpp \
        src/xmlgen/write_string_as_text_content.cpp \
        src/xmlgen/xml_element.cpp \
        src/basic_global_state.cpp \
        src/debugprint.cpp \
        src/last_generation_result.cpp \
        src/pragma_parse_exception.cpp \
        src/$(PLUGIN_NAME).cpp \
		$(END)

TARGET_CC := $(DEVKITARM)/bin/arm-none-eabi-gcc
TARGET_PLUGINDIR=$(shell $(TARGET_CC) -print-file-name=plugin)

BUILD_CXX=g++

BUILD_CXXFLAGS=-std=gnu++20 -fPIC -Wall -g -fno-rtti -I$(TARGET_PLUGINDIR)/include -I$(INCDIR)
BUILD_CXXFLAGS+=-Wno-literal-suffix

BUILD_OBJECT_PATH=build

BUILD_LDFLAGS=
BUILD_LDADD=

OBJECTS=$(patsubst src/%.cpp,$(BUILD_OBJECT_PATH)/%.o,$(SOURCES))

all: $(PLUGIN)

$(PLUGIN): $(OBJECTS)
	$(BUILD_CXX) $(BUILD_LDFLAGS) -o $@ -shared $+ $(BUILD_LDADD)
	@echo Plug-in built: $(PLUGIN_NAME).

$(BUILD_OBJECT_PATH)/%.o: src/%.cpp
	$(shell mkdir -p $(dir $@))
	$(BUILD_CXX) -c -o $@ $(BUILD_CXXFLAGS) $<

.PHONY: all clean
clean:
	rm -f $(OBJECTS) $(PLUGIN)