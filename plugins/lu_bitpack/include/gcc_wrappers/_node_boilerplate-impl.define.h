#undef  GCC_NODE_WRAPPER_BOILERPLATE
#define GCC_NODE_WRAPPER_BOILERPLATE(type_name) \
   /*static*/ type_name type_name::wrap(tree t) { \
      impl::wrap_fail_on_null(t); \
      impl::wrap_fail_on_wrong_type< type_name >(t); \
      type_name out; \
      out._node = t; \
      return out; \
   }

