local this_dir = (function()
   --
   -- Abuse debug info to find out where the current file is located.
   -- Note: This may be relative to the CWD e.g. "./" rather than 
   -- an absolute path.
   --
   return debug.getinfo(1, 'S').source
      :sub(2)
      :gsub("^([^/])", "./%1")
      :gsub("[^/]*$", "")
end)()

require "classes"
require "filesystem"
require "shell"
require "stringify-table"
require "c-lex"
require "c-ast"
require "c-parser"
require "c-exec-integer-constant-expression"

-- The `name` and `line` arguments are just here to aid with debugging.
local function parse_macro_value(value, all_found_macros, name, line)
   local n = tonumber(value)
   if n then
      return n
   end
   
   local success, err = pcall(function()
      local lexed  = lex_c(value)
      local parser = c_parser()
      local parsed = parser:parse(lexed)
      value = try_execute_c_constant_integer_expression(parsed, all_found_macros)
   end)
   if not success then
      return nil
   end
   if type(value) == "table" then
      print("Improperly processed macro value:")
      print_table(value)
      print("containing line was:\n   " .. line)
      error("error!")
   end
   
   return value
end

function for_each_macro_in(path, functor)
   path = tostring(filesystem.absolute(path))
   local data = shell:run_and_read("$DEVKITARM/bin/arm-none-eabi-gcc -Iinclude -nostdinc -E -dD -DMODERN=1 -x c '"..path.."'")
   
   local all_found_macros = {}
   
   for line in data:gmatch("[^\r\n]+") do
      if line:startswith("#define ") then
         local token = line:gmatch("[^ ]+")
         local name  = nil
         local value = ""
         do
            local i = 1
            for t in token do
               if (t:startswith("//")) then
                  break
               end
               if i == 1 then
                  -- #define
               elseif i == 2 then
                  name = t
               elseif i == 3 then
                  value = t
               else
                  value = value .. " " .. t
               end
               i = i + 1
            end
         end
         if name and value then
            value = parse_macro_value(value, all_found_macros, name, line)
            if value then
               all_found_macros[name] = value
               functor(name, value)
            end
         end
      end
   end
end