#pragma once
#include <memory>
#include <vector>
#include "codegen/instructions/base.h"
#include "codegen/func_pair.h"
#include "codegen/whole_struct_function_dictionary.h"

namespace codegen {
   class generation_request;
}

namespace codegen {
   class generation_result {
      public:
         // void __lu_bitpack_read_sector_0(struct lu_BitstreamState*);
         // void __lu_bitpack_save_sector_0(struct lu_BitstreamState*);
         std::vector<func_pair> per_sector;
         
         // void __lu_bitpack_read(const buffer_byte_type* src, int sector_id);
         // void __lu_bitpack_save(buffer_byte_type* dst, int sector_id);
         optional_func_pair top_level;
         
         whole_struct_function_dictionary whole_struct;
         
      protected:
         void _generate_per_sector_functions(const generation_request&, const std::vector<std::unique_ptr<instructions::base>>& instructions_by_sector);
         bool _get_or_declare_top_level_functions(const generation_request&);
         void _generate_top_level_function(const generation_request&, size_t sector_count, bool is_read);
         
      public:
         bool generate(const generation_request&, const std::vector<std::unique_ptr<instructions::base>>&);
   };
}