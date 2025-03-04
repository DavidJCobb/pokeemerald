#include "gcc_wrappers/events/on_type_finished.h"

namespace gcc_wrappers::events {
   /*static*/ void on_type_finished::_gcc_handler(void* event_data, void* userdata) {
      auto& self = *(on_type_finished*)userdata;
      auto  node = (tree) event_data;
      
      //
      // We have to keep track of what types we've already seen, because every 
      // occurrence of `struct Typename` triggers `PLUGIN_TYPE_FINISHED` as of 
      // GCC 11.4. Do recall that in C, if `Typename` wasn't declared using a 
      // `typedef` declaration, then every time you use `Typename`, you have 
      // to prefix it with `struct`, thereby re-triggering the "type finished" 
      // handler.
      //
      for(const auto type : self._seen)
         if (type.unwrap() == node)
            return;
      auto type = type::base::wrap(node);
      self._seen.push_back(type);
      
      auto list = self._callbacks; // in case a callback adds/removes listeners
      for(const auto& info : list) {
         (info.func)(type);
      }
   }
   
   void on_type_finished::initialize(const char* plugin_info_base_name) {
      register_callback(
         plugin_info_base_name,
         PLUGIN_FINISH_TYPE,
         &_gcc_handler,
         this
      );
   }

   void on_type_finished::add(id_type id, callback_type func) {
      this->_callbacks.push_back({
         .id   = id,
         .func = func,
      });
   }
   void on_type_finished::remove(id_type id) {
      std::erase_if(this->_callbacks, [id](auto& info) { return info.id == id; });
   }
}