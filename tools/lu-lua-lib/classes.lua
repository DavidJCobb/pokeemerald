
make_class = nil

--[[--

   This is a relatively basic library for a class system. To create 
   a class, call `make_class`, passing the appropriate configuration 
   parameters for your class.
   
   Allowed parameters:
   
      constructor
      
         Optional. A callable responsible for initializing the newly 
         constructed class instance. This takes at least one argument; 
         by default, that argument is `self`, the instance being 
         constructed.
         
      superclass
      
         Optional. A class (previously returned from this library) 
         to use as a superclass.
         
      calls_super
      
         Optional. If truthy, then instead of `self`, the constructor 
         should expect a `super` argument. This argument is a function 
         that passes its arguments to the superclass constructor(s), 
         returning the instance being constructed.
         
         Using this option on a class with no superclass is an error. 
         Using this option on a class without its own constructor is 
         also an error.
         
         If you use this option, your constructor must call `super` 
         exactly once before returning. Leaking the `super` function 
         and calling it later is an error.
      
      instance_members
      
         Optional. If this is a table, then its contents will be 
         accessible on all instances. Put instance member functions 
         here.
         
      static_members
      
         Optional. If this is a table, then its contents will be 
         accessible on the class itself. Put static member functions 
         here.
         
      __tostring
      
         Optional. Becomes the `__tostring` metamethod for instances.
   
   This library's entry point creates and returns a class, which is a 
   callable that constructs a new instance when called. All classes 
   have an `is` static member function by default, which returns a 
   boolean indicating whether the argument is an instance of the class 
   (or any subclass).

--]]--

do

   --
   -- Common infrastrcture for classes.
   --

   -- The __index metamethod for instances of any class.
   local function instance_meta_index(inst, key)
      do
         local v = rawget(inst, key)
         if v ~= nil then
            return v
         end
      end
      local inst_meta = getmetatable(inst)
      local class     = inst_meta.__class
      local clas_meta = getmetatable(class)
      repeat
         for k, v in pairs(clas_meta.__instance_members) do
            if k == key then
               return v
            end
         end
         for k, v in pairs(clas_meta.__instance_getters) do
            if k == key then
               return v(inst)
            end
         end
         class     = clas_meta.__superclass
         clas_meta = getmetatable(class)
      until not class
   end
   
   -- Function used to invoke class constructors on an instance, once its 
   -- metatable has been set.
   local function instance_construct(inst, ...)
      local constructors   = {}
      local calls_super    = {}
      local any_call_super = false
      local count = 0
      do
         local inst_meta = getmetatable(inst)
         local class     = inst_meta.__class
         local clas_meta = getmetatable(class)
         repeat
            local constructor = clas_meta.__constructor
            if constructor then
               count = count + 1
               constructors[count] = constructor
               calls_super[count]  = clas_meta.__constructor_calls_super
               if clas_meta.__constructor_calls_super then
                  any_call_super = true
               end
            end
            class     = clas_meta.__superclass
            clas_meta = getmetatable(class)
         until not class
      end
      if count == 0 then
         return
      end
      local args = {...}
      if any_call_super then
         assert(not calls_super[count], "The outermost base class has no super to call! We should've rejected this when the class was created.")
         --
         -- Constructors that want to call super should receive a `super argument 
         -- instead of a `self` argument. Calling `super` will return the new 
         -- `self`.
         --
         -- Calling super is a bit tricky, because not every constructor has to 
         -- do it. Consider, for example, the inheritance chain A B C D E F, where 
         -- the constructors for C and E call super but the others do not. In that 
         -- instance, the call tree looks something like this:
         --
         --    E(f_arguments)
         --       super(e_arguments)
         --          C(e_arguments)
         --             A(c_arguments)
         --             B(c_arguments)
         --          D(e_arguments)
         --    F(f_arguments)
         --
         -- We can express this by splitting the constructor list at every super-
         -- calling constructor:
         --
         --    ab|Cd|Ef
         --
         -- Then, we'd invoke the groups in reverse order. Within each group, we'd 
         -- call the constructors in forward order:
         --
         --  - We first call E. In order to access its instance, E has to call 
         --    super.
         --
         --  - E's call to super triggers a call to C. In order to access its 
         --    instance, C must in turn call super as well.
         --
         --  - C's call to super triggers a call to A...
         --
         --  - ...Then to B.
         --
         --  - We return to C.
         --
         --  - We return to E's super, which next calls D.
         --
         --  - We return to E.
         --
         --  - We next call F.
         --
         local groups = {}
         for i = count, 1, -1 do
            local last = groups[#groups]
            if calls_super[i] or not last then
               groups[#groups + 1] = {
                  constructors   = { constructors[i] },
                  arguments      = args,
                  is_first_group = not last,
                  did_call_super = false, -- guard against multiple calls, and against failing to call
                  super_expired  = false, -- guard against leaking `super`
               }
            else
               last.constructors[#last.constructors + 1] = constructors[i]
            end
         end

         local baseline_super
         local function invoke_group(n)
            local group = groups[n]
            for i = 1, #group.constructors do
               local cons = group.constructors[i]
               if i == 1 and not group.is_first_group then
                  local function super(...)
                     return baseline_super(n, table.unpack({...}))
                  end
                  --
                  -- Use `pcall` to ensure that if a constructor leaks `super` and 
                  -- then throws an error, we can mark that `super` as expired and 
                  -- then re-throw the error.
                  --
                  local status, ex = pcall(function()
                     cons(super, table.unpack(group.arguments))
                  end)
                  group.super_expired = true
                  if status then
                     if not group.did_call_super then
                        error("If your constructor asks to call `super`, then it has to actually call `super`!")
                     end
                  else
                     error(ex)
                  end
               else
                  cons(inst, table.unpack(group.arguments))
               end
            end
         end
         baseline_super = function(n, ...)
            do
               local group = groups[n]
               if group.super_expired then
                  error("Do not call `super` from outside of the relevant constructor.")
               end
               if group.did_call_super then
                  error("Do not call `super` more than once within your constructor.")
               end
               group.did_call_super = true
            end
            local super_group = groups[n - 1]
            super_group.arguments = {...}
            invoke_group(n - 1)
            return inst
         end

         -- Invoke groups in reverse order, as discussed above. (This function 
         -- will recurse, indirectly, via the calls to `super`.)
         invoke_group(#groups)
      else
         --
         -- The `constructors` variable is now a list of constructors, ordered 
         -- subclasses first/superclasses last. We want to call the superclass 
         -- constructors first.
         --
         for i = count, 1, -1 do
            constructors[i](inst, table.unpack(args))
         end
      end
   end
   
   -- Powers the `class.is(inst)` method for checking whether `inst` is an 
   -- instance of `class`.
   local function instance_is(inst, class)
      local inst_meta = getmetatable(inst)
      if not inst_meta then
         return false
      end
      local inst_clas = inst_meta.__class
      while inst_clas do
         if inst_clas == class then
            return true
         end
         local clas_meta = getmetatable(inst_clas)
         if not clas_meta then
            return false
         end
         inst_clas = clas_meta.__superclass
      end
      return false
   end
   
   --
   -- Code to make a class.
   --
   
   make_class = function(options)
      local constructor
      local static_members   = {}
      local instance_members = {}
      local instance_getters = {}
      local superclass
      local calls_super = false
      if type(options) == "table" then
         constructor      = options.constructor      or nil
         instance_members = options.instance_members or {}
         instance_getters = options.getters          or {}
         static_members   = options.static_members   or {}
         superclass       = options.superclass       or nil
         if options.calls_super then
            if not constructor then
               error("It's illegal for a class to be constructed with `super` when it has no constructor!")
            end
            if not superclass then
               error("It's illegal for a class constructor to expect `super` when it has no superclass!")
            end
            calls_super = true
         end
      end
      
      local instance_metatable = {
         __index = instance_meta_index
      }
      if type(options) == "table" then
         local v = options.__tostring
         if v then
            instance_metatable.__tostring = v
         end
      end
      local class_metatable = {
         __species = "class",
         __index   = static_members,
         __constructor             = constructor,
         __constructor_calls_super = calls_super,
         __instance_members        = instance_members,
         __instance_getters        = instance_getters,
         __instance_metatable      = instance_metatable,
         __superclass              = superclass,
      }
      --
      -- Allow constructing class instances via `class(args)`.
      --
      function class_metatable:__call(...)
         local instance = {}
         setmetatable(instance, instance_metatable)
         instance_construct(instance, table.unpack({...}))
         return instance
      end
      
      local class_table = {}
      instance_metatable.__class = class_table
      setmetatable(class_table, class_metatable)
      function class_table.is(a, b)
         local subject = a
         if a == class_table then -- we were called as `class:is(instance)`
            subject = b
         end
         return instance_is(subject, class_table)
      end
      
      return class_table
   end
end

-- older, simpler implementation w/o call-super and where class.is didn't 
-- properly handle superclasses
--[[
function make_class(options)
   local constructor
   local static_members   = {}
   local instance_members = {}
   local superclass
   if type(options) == "table" then
      constructor      = options.constructor      or nil
      instance_members = options.instance_members or {}
      static_members   = options.static_members   or {}
      superclass       = options.superclass       or nil
   end
   
   local instance_metatable = {
      __index = instance_members
   }
   local class_metatable = {
      __species    = "class",
      __superclass = nil,
      __index      = static_members
   }
   
   --
   -- Allow constructing class instances via `class(args)`.
   --
   if constructor then
      function class_metatable:__call(...)
         local instance = {}
         local args     = {...}
         setmetatable(instance, instance_metatable)
         constructor(instance, table.unpack(args))
         return instance
      end
   else
      function class_metatable:__call()
         local instance = {}
         setmetatable(instance, instance_metatable)
         return instance
      end
   end
   
   local class_table = {}
   instance_metatable.__class = class_table
   setmetatable(class_table, class_metatable)
   function class_table.is(a, b)
      local subject = a
      if a == class_table then -- we were called as `class:is(instance)`
         subject = b
      end
      return getmetatable(b) == instance_metatable
   end
   
   return class_table
end
--]]--
