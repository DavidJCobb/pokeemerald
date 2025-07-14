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
require "c-ast-for-each-node"
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

-- Returns either the macro's computed value (a number), or a table explaining 
-- why we can't parse the macro.
local function process_macro(name, value, line, all_found_macros)
   local n = tonumber(value)
   if n then
      return n
   end
   
   local parsed
   do
      local lexed
      local success, err = pcall(function()
         lexed  = lex_c(value)
      end)
      if not success then
         return { cause = "lex failure" }
      end
      success, err = pcall(function()
         local parser = c_parser()
         parsed = parser:parse(lexed)
      end)
      if not success then
         return {
            cause = "parse failure",
            lexed = lexed,
            err   = err
         }
      end
   end
   
   local computed
   local success, err = pcall(function()
      computed = try_execute_c_constant_integer_expression(parsed, all_found_macros)
   end)
   if not success then
      if err == "missing identifier" then
         local missing = {}
         do
            local function _handle_identifier(name)
               if not all_found_macros[name] then
                  missing[#missing + 1] = name
               end
            end
            c_ast_for_each_node(
               parsed,
               function(node)
                  if c_ast.primary_expression.is(node) then
                     if node.case == "identifier" then
                        _handle_identifier(node.data)
                     end
                  end
               end
            )
         end
         return {
            cause   = "missing identifier",
            line    = line,
            parsed  = parsed,
            missing = missing
         }
      end
      return {
         cause = "uncomputable"
      }
   end
   if type(computed) == "table" then
      print("Improperly processed macro value:")
      print_table(computed)
      print("containing line was:\n   " .. line)
      error("error!")
   end
   return computed
end

function for_each_macro_in(path, functor)
   path = tostring(filesystem.absolute(path))
   local data = shell:run_and_read("$DEVKITARM/bin/arm-none-eabi-gcc -Iinclude -nostdinc -E -dD -DMODERN=1 -x c '"..path.."'")
   
   local all_found_macros = {}
   local deferred_macros  = {}
   
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
            local result = process_macro(name, value, line, all_found_macros)
            if type(result) == "table" then
               if result.cause == "missing identifier" then
                  deferred_macros[name] = result
               end
            elseif result then
               all_found_macros[name] = result
               functor(name, result)
            end
         end
      end
   end
   
   --
   -- Some macros are defined ahead of other macros they depend on. This doesn't break 
   -- because macros are evaluated by the preprocessor when used, not when defined.
   --
   repeat
      local any_resolved  = false
      local still_waiting = {}
      for name, info in pairs(deferred_macros) do
         local all_dependencies_present = true
         for _, dependency in ipairs(info.missing) do
            if not all_found_macros[dependency] then
               all_dependencies_present = false
               break
            end
         end
         if all_dependencies_present then
            local computed
            local success, err = pcall(function()
               computed = try_execute_c_constant_integer_expression(info.parsed, all_found_macros)
            end)
            if success and computed then
               if type(computed) == "table" then
                  print("Improperly processed macro value:")
                  print_table(computed)
                  print("containing line was:\n   " .. info.line)
                  error("error!")
               end
               all_found_macros[name] = computed
               functor(name, computed)
               any_resolved = true
            end
         else
            still_waiting[name] = info
         end
      end
      deferred_macros = still_waiting
   until not any_resolved
end