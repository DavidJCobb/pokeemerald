#pragma once
#include <cstdint>
#include <functional>
#include <vector>
#include "lu/eight_cc.h"
#include "lu/singleton.h"
#include "gcc_wrappers/type/base.h"

namespace gcc_wrappers::events {
   class on_type_finished;
   class on_type_finished : public lu::singleton<on_type_finished> {
      public:
         using callback_type = std::function<void(type::base)>;
         using id_type       = lu::eight_cc;
         
      protected:
         struct callback_info {
            id_type       id = 0;
            callback_type func;
         };
      
      protected:
         std::vector<type::base>    _seen;
         std::vector<callback_info> _callbacks;
         
         static void _gcc_handler(void* event_data, void* userdata);
         
      public:
         void initialize(const char* plugin_info_base_name); // register with GCC
      
         void add(id_type, callback_type);
         void remove(id_type);
   };
}