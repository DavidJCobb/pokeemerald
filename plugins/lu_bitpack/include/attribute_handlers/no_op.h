#pragma once
#include <gcc-plugin.h>
#include <tree.h>

namespace attribute_handlers {
   extern tree no_op(tree* node, tree name, tree args, int flags, bool* no_add_attrs);
}