#pragma once
#include <optional>
#include <string_view>
#include <gcc-plugin.h>
#include <c-family/c-pragma.h> // cpp_reader
#include "gcc_wrappers/identifier.h"

namespace bitpacking {
   //
   // Helper for handling the act of parsing the options out of the pragma. 
   // You can then feed this into `global_options` to actually handle them.
   //
   class requested_global_options {
      public:
         struct location_set {
            location_t key  = UNKNOWN_LOCATION;
            location_t data = UNKNOWN_LOCATION;
         };
      
         struct identifier_option {
            identifier_option(gcc_wrappers::optional_identifier oi) : data(*oi) {}
            
            gcc_wrappers::identifier data;
            location_set loc;
         };
         struct size_option {
            size_option() {}
            size_option(size_t s) : data(s) {}
            
            size_t       data;
            location_set loc;
         };
      
         struct function_set {
            std::optional<identifier_option> boolean;
            std::optional<identifier_option> s8;
            std::optional<identifier_option> s16;
            std::optional<identifier_option> s32;
            std::optional<identifier_option> u8;
            std::optional<identifier_option> u16;
            std::optional<identifier_option> u32;
            std::optional<identifier_option> buffer;
            std::optional<identifier_option> string_nt; // string always w/ terminator
            std::optional<identifier_option> string_ut; // string w/ optional terminator
         };
         
      protected:
         std::optional<identifier_option>* _id_option_for_key(std::string_view);
         std::optional<size_option>* _size_option_for_key(std::string_view);
         
      public:
         // May throw `pragma_parse_exception`.
         requested_global_options(cpp_reader&);
      
         location_t pragma_location = UNKNOWN_LOCATION;
         struct {
            std::optional<identifier_option> stream_state_init;
            function_set read;
            function_set save;
         } functions;
         struct {
            std::optional<size_option> max_count;
            std::optional<size_option> bytes_per;
         } sectors;
         struct {
            std::optional<identifier_option> bitstream_state;
            std::optional<identifier_option> boolean;
            std::optional<identifier_option> buffer_byte;
         } types;
   };
}