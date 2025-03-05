require("tools/lu-lua-lib/stdext.lua")

local makelib = {}
do
   -- Given a single string, treat it as a path and extract the 
   -- directory part. Unlike $(dir ...) in make, this only acts 
   -- on a single string, rather than treating its argument as 
   -- a space-separated list of paths.
   makelib.dir = function(str)
      local i = string.findlast(str, "/")
      if not i then
         return "./"
      end
      return str:sub(1, i + 1)
   end
end