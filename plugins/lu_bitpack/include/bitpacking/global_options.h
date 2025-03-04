#pragma once
#include <limits>
#include "gcc_wrappers/decl/function.h"
#include "gcc_wrappers/type/base.h"
#include "gcc_wrappers/type/function.h"
#include "gcc_wrappers/type/pointer.h"
#include "gcc_wrappers/type/record.h"
#include "gcc_wrappers/identifier.h"

namespace bitpacking {
   class requested_global_options;
}

namespace bitpacking {
   class global_options {
      public:
         struct function_set {
            gcc_wrappers::decl::optional_function boolean;
            gcc_wrappers::decl::optional_function s8;
            gcc_wrappers::decl::optional_function s16;
            gcc_wrappers::decl::optional_function s32;
            gcc_wrappers::decl::optional_function u8;
            gcc_wrappers::decl::optional_function u16;
            gcc_wrappers::decl::optional_function u32;
            gcc_wrappers::decl::optional_function buffer;
            gcc_wrappers::decl::optional_function string_nt; // string always w/ terminator
            gcc_wrappers::decl::optional_function string_ut; // string w/ optional terminator
         };
         
      protected:
         void _missing_option(const requested_global_options&, std::string_view name);
         
         gcc_wrappers::decl::optional_function _get_function_or_fail(location_t, gcc_wrappers::identifier);
         
         bool _check_and_report_unprototyped(location_t, gcc_wrappers::type::function);
         
      public:
         struct {
            gcc_wrappers::decl::optional_function stream_state_init;
            function_set read;
            function_set save;
         } functions;
         struct {
            size_t max_count = 1;
            size_t bytes_per = std::numeric_limits<size_t>::max(); // size in bytes per sector
         } sectors;
         struct {
            gcc_wrappers::type::optional_record  bitstream_state;
            gcc_wrappers::type::optional_pointer bitstream_state_ptr;
            gcc_wrappers::type::optional_base    boolean; // for old C codebases
            gcc_wrappers::type::optional_base    buffer_byte;
            gcc_wrappers::type::optional_pointer buffer_byte_ptr;
         } types;
         bool invalid = true;
         
         void grab(const requested_global_options&);
            
         //
         // Helpers:
         //
         
         bool type_is_boolean(const gcc_wrappers::type::base) const;
   };
}