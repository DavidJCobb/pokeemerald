
# `classes`

A relatively basic library that defines a class system. To create a class, call `make_class`, passing an options table with the appropriate configuration parameters. The return value is the class: a callable object which, when called, returns a new instance.

## Per-class configuration options

* **`constructor`:** Optional callable. When an instance is constructed, the constructor will be invoked with the new instance as the first parameter (i.e. `self`), along with any other parameters that were passed in when calling the class-callable. Constructors are invoked in order from superclass to subclass.

* **`superclass`:** Optional: a class that the new class should be a subclass of.

* **`calls_super`:** Optional boolean. If this is a truthy value, then the new class's constructor will be invoked out of order. Instead of being passed `self`, it will be passed `super`: a special callable which, when invoked, calls superclass constructors as appropriate, forwarding passed-in arguments to them. When `super` is called, it will return `self`.
  
  You can use this whenever you want a subclass to be constructed with different arguments from its superclass. The subclass can indicate that it `calls_super`, and then its constructor can run code like `local self = super(arguments, that, the, superclass, expects)` before then configuring the new instance as appropriate.
  
  If you use this option, then your constructor must call `super` exactly once before returning. Leaking `super` and then calling it after construction will raise an error. Calling `super` multiple times will raise an error. This option cannot be used on a class that has no constructor or a class that has no superclass (attempting either will raise an error).

* **`instance_members`:** Optional. A table: any properties on this table will be accessible on any instances of this class. Methods can be put here.

* **`static_members`:** Optional. A table: any properties on this table will be accessible on the class itself.

* **`instance_metamethods`:** Optional. A table: any official metamethod names *except* `__index` and `__newindex` will be added to the metatable for class instances. This is where you'd put operator overloads.

* **`__tostring`:** Optional function. You can define `__tostring` here or in the `instance_metamethods` option. The latter takes priority.

* **`getters`:** Optional. A table of functions, which define instance-level getters (i.e. `instance.foo` would invoke `getters.foo(self)` if present).

## Per-class functions

* **`some_class.is(v)`:** Returns a boolean indicating whether `v` is an instance of `some_class` or any subclass thereof.

## Example

```lua
local animal = make_class({
   constructor = function(self, name, leg_count, noise)
      self.name  = name
      self.legs  = leg_count
      self.noise = noise
   end,
   instance_members = {
      speak = function(self)
         print(self.noise or "[silence]")
      end
   }
})

local biped = make_class({
   constructor = function(super, name, noise)
      local self = super(name, 2, noise)
   end,
   superclass  = animal,
   calls_super = true
})

local human
do
   local instance_members = {}
   human = make_class({
      superclass = biped,
      instance_members = instance_members
   })
   
   function instance_members:speak()
      print('"Howdy, pardner!"')
   end
end

local dog = animal("dog", 4, "bark!")
dog:speak() -- "bark!"

local joe = human("joe", "hey there!")
joe:speak() -- "\"Howdy, pardner!\""
print(joe.legs) -- 2
```