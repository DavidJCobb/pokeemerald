function include(path) -- because require is janky
   local this_path = debug.getinfo(1, 'S').source
      :sub(2)
      :gsub("^([^/])", "./%1")
      :gsub("[^/]*$", "")
   
   local dst_folder = path
      :gsub("^([^/])", "./%1")
      :gsub("[^/]*$", "")
   local dst_file = path:gsub("([^/]*)$", "%1")
   
   local prior = package.path
   package.path = this_path .. dst_file
   
   local status, err = pcall(function()
      require(dst_file)
   end)
   package.path = prior
   if err then
      error(err)
   end
end
include("../lu-lua-lib/classes.lua")

do -- Test subclass relationships and proper call order for call-super
   local A = make_class({
      constructor = function(self, arg)
         print("A constructed (" ..tostring(arg) .. ")")
      end
   })
   local B = make_class({
      superclass  = A,
      constructor = function(self, arg)
         print("B constructed (" ..tostring(arg) .. ")")
      end
   })
   local C = make_class({
      superclass  = B,
      calls_super = true,
      constructor = function(super, arg)
         local self = super("c-arg")
         print("C constructed (" ..tostring(arg) .. ")")
      end
   })
   local D = make_class({
      superclass  = C,
      constructor = function(self, arg)
         print("D constructed (" ..tostring(arg) .. ")")
      end
   })
   local E = make_class({
      superclass  = D,
      calls_super = true,
      constructor = function(super, arg)
         local self = super("e-arg")
         print("E constructed (" ..tostring(arg) .. ")")
      end
   })
   local F = make_class({
      superclass  = E,
      constructor = function(self, arg)
         print("F constructed (" ..tostring(arg) .. ")")
      end
   })

   local instance = F("f-arg")
   print("F is an F? " .. tostring(F.is(instance)))
   print("F is an A? " .. tostring(A.is(instance)))
end