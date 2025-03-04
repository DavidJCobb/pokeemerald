#pragma once
#include <gcc-plugin.h>
#include <tree.h>

namespace attribute_handlers {
   extern tree bitpack_union_internal_tag(tree* node, tree name, tree args, int flags, bool* no_add_attrs);
}