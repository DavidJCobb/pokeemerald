function include(path) -- because require is janky
   --
   -- Abuse debug info to find out where the current file is located.
   --
   local this_path = debug.getinfo(1, 'S').source
      :sub(2)
      :gsub("^([^/])", "./%1")
      :gsub("[^/]*$", "")
   
   local dst_folder = path
      :gsub("^([^/])", "./%1")
      :gsub("[^/]*$", "")
   local dst_file = path:gsub("([^/]*)$", "%1")
   --
   -- Override `package.path` to tell it to look for an exact file.
   --
   local prior = package.path
   package.path = this_path .. dst_file
   local status, err = pcall(function()
      require(dst_file)
   end)
   package.path = prior
   if not status then
      error(err)
   end
end

include("../lu-lua-lib/stringify-table.lua")
include("../lu-lua-lib/classes.lua")
include("../lu-lua-lib/shell.lua")
include("../lu-lua-lib/console.lua")
include("../lu-lua-lib/stdext.lua")

local data = shell:run_and_read('$DEVKITARM/bin/arm-none-eabi-gcc -Iinclude -nostdinc -E -dD -DMODERN=1 -x c "src/lu/preprocessor-macros-of-interest.txt"')

include("../lu-lua-lib/c-lex.lua")
include("../lu-lua-lib/c-parser.lua")

local macros = {}

local enums = {
   FLAG    = {}, -- overworld script flags
   GROWTH  = {}, -- EXP growth rates
   NATURE  = {}, -- Pokemon natures
   SPECIES = {}, -- Pokemon species
}
local desired_vars = {
   "SHINY_ODDS",
   
   -- Pokemon gender ratio sentinel values
   "MON_MALE",
   "MON_FEMALE",
   "MON_GENDERLESS",
}
local found_vars_of_interest = {}

-- Needed in order to resolve cases where a macro we care about references 
-- a macro we don't care about.
local all_found_macros = {}

-- The `name` and `line` arguments are just here to aid with debugging.
function parse_macro_value(value, name, line)
   local n = tonumber(value)
   if n then
      return n
   end
   
   local success, err = pcall(function()
      local lexed  = lex_c(value)
      local parser = c_parser:new()
      local parsed = parser:parse(lexed)
      
      local computed = 0
      local function _exec(ast_node)
         if ast_node.type == "primary-expression" then
            if ast_node.case == "constant" then
               return ast_node.data
            end
            if ast_node.case == "identifier" then
               local iv = all_found_macros[ast_node.data]
               if iv then
                  return iv
               end
               error("missing identifier")
            end
            if ast_node.case == "parenthetical" then
               return _exec(ast_node.data.expression)
            end
            if ast_node.case == "string-literal" then
               error("string literal (not an integral constant)")
            end
            error("unhandled "..ast_node.type.." case")
         end
         if ast_node.type == "postfix-expression" then
            if ast_node.case == "compound-literal" then
               error("not a valid constant expression (bare compound literal)")
            end
            if ast_node.case == "array-subscript" then
               error("not a valid constant expression (array subscript)")
            end
            if ast_node.case == "call" then
               error("not a valid constant expression (function call)")
            end
            if ast_node.case == "dereferencing-member-access" then
               error("not a valid constant expression (dereference and member access)")
            end
            if ast_node.case == "member-access" then
               error("not a valid constant expression (member access)")
            end
            if ast_node.case == "operator" then
               error("not a valid constant expression (increment/decrement operator)")
            end
            error("unhandled "..ast_node.type.." case")
         end
         if ast_node.type == "unary-expression" then
            if ast_node.case == "sizeof-type" or ast_node.case == "sizeof-expression" then
               error("not a computable constant expression (we can't process sizeof here)")
            end
            if ast_node.case == "operator" then
               local op = ast_node.data.operator
               if op == "&" or op == "*" or op == "++" or op == "--" then
                  error("not a valid constant expression (unary/prefix operator " .. op .. ")")
               end
               if op == "!" or op == "-" or op == "+" or op == "~" then
                  local subject = _exec(ast_node.data.subject)
                  if op == "!" then
                     return subject == 0
                  end
                  if op == "-" then
                     return -tonumber(subject)
                  end
                  if op == "+" then
                     return tonumber(subject)
                  end
                  if op == "~" then
                     return ~tonumber(subject)
                  end
               end
               error("unhandled unary operator")
            end
            error("unhandled "..ast_node.type.." case")
         end
         if ast_node.type == "cast-expression" then
            error("not a valid constant expression (static cast)")
         end
         if ast_node.type == "operator-tree" then
            local function _operate(node)
               function _term_to_number(term)
                  if term.item_type == "node" then
                     return _operate(term)
                  end
                  term = term.data
                  if type(term) == "table" then
                     term = _exec(term)
                  end
                  return term
               end
            
               local accum = _term_to_number(node.terms[1])
               for i = 2, #node.terms do
                  local term = _term_to_number(node.terms[i])
                  local comparison = nil
                  if node.operator == "+" then
                     accum = accum + term
                  elseif node.operator == "-" then
                     accum = accum - term
                  elseif node.operator == "*" then
                     accum = accum * term
                  elseif node.operator == "/" then
                     accum = accum / term
                  elseif node.operator == "%" then
                     accum = accum % term
                  elseif node.operator == "<<" then
                     accum = accum << term
                  elseif node.operator == ">>" then
                     accum = accum >> term
                  elseif node.operator == "|" then
                     accum = accum | term
                  elseif node.operator == "&" then
                     accum = accum & term
                  elseif node.operator == "^" then
                     accum = accum ~ term
                  elseif node.operator == "==" then
                     comparison = accum == term
                  elseif node.operator == "!=" then
                     comparison = accum ~= term
                  elseif node.operator == "<" then
                     comparison = accum < term
                  elseif node.operator == ">" then
                     comparison = accum > term
                  elseif node.operator == "<=" then
                     comparison = accum <= term
                  elseif node.operator == ">=" then
                     comparison = accum >= term
                  elseif node.operator == "&&" then
                     comparison = accum ~= 0 and term ~= 0
                  elseif node.operator == "||" then
                     comparison = accum ~= 0 or term ~= 0
                  end
                  if comparison ~= nil then
                     accum = 0
                     if comparison then
                        accum = 1
                     end
                  end
               end
               return accum
            end
            return _operate(ast_node.data)
         end
         if ast_node.type == "conditional-expresion" then
            local cond = _exec(ast_node.data.condition)
            if cond and cond ~= 0 then
               return _exec(ast_node.data.if_true)
            else
               return _exec(ast_node.data.if_false)
            end
         end
         if ast_node.type == "assignment-expression" then
            error("not a valid constant expression (assignment)")
         end
         if ast_node.type == "comma-expression" then
            return _exec(ast_node.data.retained)
         end
         error("unhandled AST node type")
      end
      value = _exec(parsed)
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

function try_handle_enumeration_member(name, value)
   if name:startswith("SPECIES_UNOWN_") then
      return true
   end
   for k, v in pairs(enums) do
      if name:startswith(k .. "_") then
         v[name] = value
         return true
      end
   end
   return false
end
function try_handle_variable(name, value)
   for _, v in ipairs(desired_vars) do
      if v == name then
         found_vars_of_interest[name] = value
         return true
      end
   end
   return false
end

function try_handle_macro(name, value)
   if try_handle_enumeration_member(name, value) then
      return
   end
   if try_handle_variable(name, value) then
      return
   end
end

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
         value = parse_macro_value(value, name, line)
         all_found_macros[name] = value
         try_handle_macro(name, value)
      end
   end
end

--
-- DEBUG: print all found vars
--
for k, v in pairs(found_vars_of_interest) do
   print(k .. " == " .. v)
end
--
-- DEBUG: print some of the smaller enums
--
function debug_print_enum(name)
   print(name .. ":")
   for k, v in pairs(enums[name]) do
      print("   " .. k .. " == " .. v)
   end
end
debug_print_enum("GROWTH")
debug_print_enum("NATURE")
--
-- DEBUG: print flags that we know require parsing C constant expressions
--
function debug_print_macro(name)
   local v = all_found_macros[name]
   if v then
      print(name .. " == " .. v)
   else
      print(name .. " == <not found or unparseable>")
   end
end
debug_print_macro("TEMP_FLAGS_START")
debug_print_macro("TRAINER_FLAGS_START") -- 0x500
debug_print_macro("MAX_TRAINERS_COUNT")
debug_print_macro("TRAINER_FLAGS_END")   -- 0x85F == (TRAINER_FLAGS_START + MAX_TRAINERS_COUNT - 1)
debug_print_macro("SYSTEM_FLAGS")        -- 0x860 == (TRAINER_FLAGS_END + 1)
debug_print_macro("DAILY_FLAGS_START")
function debug_print_flag(name)
   local data = enums.FLAG[name]
   if data then
      print(name .. " == " .. data)
   else
      print(name .. " == <unparsable>")
   end
end
debug_print_flag("FLAG_TEMP_1")
debug_print_flag("FLAG_TEMP_1F")
debug_print_flag("FLAG_UNUSED_0x020")
debug_print_flag("FLAG_REGISTERED_ROSE")
debug_print_flag("FLAG_HIDDEN_ITEM_LAVARIDGE_TOWN_ICE_HEAL")
debug_print_flag("FLAG_UNUSED_0x91F")
debug_print_flag("FLAG_UNUSED_0x920")

--
-- TODO: FLAG_REGISTERED_ROSE fails because values like REMATCH_ROSE aren't 
-- preprocessor macros; they're enumeration members (in `constants/rematch.h`).
-- Don't suppose GCC has a flag to print *those* for us too? :\ 
--
