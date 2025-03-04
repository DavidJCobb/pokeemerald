#pragma once
#include <gcc-plugin.h>
#include <tree.h>

namespace attribute_handlers {
   extern tree bitpack_tagged_id(tree* node, tree name, tree args, int flags, bool* no_add_attrs);
}