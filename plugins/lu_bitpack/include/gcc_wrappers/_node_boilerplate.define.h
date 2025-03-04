#undef GCC_NODE_WRAPPER_BOILERPLATE
#undef GCC_NODE_WRAPPER_BOILERPLATE_NO_DEFAULT_CONSTRUCTOR
#undef _GCC_NODE_WRAPPER_BOILERPLATE_WRAP
#undef _GCC_NODE_WRAPPER_BOILERPLATE_IS
#undef _GCC_NODE_WRAPPER_BOILERPLATE_AS

#define _GCC_NODE_WRAPPER_BOILERPLATE_WRAP(type_name) \
   static type_name wrap(tree);

#define _GCC_NODE_WRAPPER_BOILERPLATE_IS(type_name) \
   template<typename Subclass> requires impl::can_is_as<type_name, Subclass> \
   bool is() const { \
      return Subclass::raw_node_is(this->_node); \
   }

#define _GCC_NODE_WRAPPER_BOILERPLATE_AS(type_name) \
   template<typename Subclass> requires impl::can_is_as<type_name, Subclass> \
   Subclass as() { \
      impl::wrap_fail_on_wrong_type<Subclass>(this->_node); \
      return Subclass::wrap(this->_node); \
   } \
   \
   template<typename Subclass> requires impl::can_is_as<type_name, Subclass> \
   const Subclass as() const { \
      impl::wrap_fail_on_wrong_type<Subclass>(this->_node); \
      return Subclass::wrap(this->_node); \
   }

// For `static T::wrap` to work, we need to be able to default-construct
// each `T`. Default constructors don't have to be public, but they do 
// have to exist. (The alternative would be to require these macros to 
// take the superclass as a parameter, and use that to define private 
// non-default constructors that take a "tag" argument of some sort. 
// That seems less maintainable, if a bit more safe.)
//
// Giving this a dummy template parameter means that it won't conflict 
// with any no-args constructor that the class itself may define. Any 
// such constructors will take precedence over this one.
#define _GCC_NODE_WRAPPER_BOILERPLATE_HIDDEN_DEFAULT_CONSTRUCTOR(type_name) \
   template<bool Dummy = false> type_name() {}

// Combined macro for all of our boilerplate.
#define GCC_NODE_WRAPPER_BOILERPLATE(type_name) \
   protected: \
      _GCC_NODE_WRAPPER_BOILERPLATE_HIDDEN_DEFAULT_CONSTRUCTOR(type_name) \
   public: \
      _GCC_NODE_WRAPPER_BOILERPLATE_WRAP(type_name) \
      _GCC_NODE_WRAPPER_BOILERPLATE_IS(type_name) \
      _GCC_NODE_WRAPPER_BOILERPLATE_AS(type_name) \

#undef  DECLARE_GCC_OPTIONAL_NODE_WRAPPER
#define DECLARE_GCC_OPTIONAL_NODE_WRAPPER(wrapper_name) \
   using optional_##wrapper_name = optional< wrapper_name >;
