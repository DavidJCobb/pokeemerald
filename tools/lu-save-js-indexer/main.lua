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
include("../lu-lua-lib/c-ast.lua")
include("../lu-lua-lib/c-parser.lua")
include("../lu-lua-lib/c-exec-integer-constant-expression.lua")

include("enumeration.lua")

local enums = {}
do
   local NAMES = {
      "FLAG",    -- overworld script flags
      "GROWTH",  -- EXP growth rates
      "NATURE",  -- Pokemon natures
      "SPECIES", -- Pokemon species
   }
   for _, name in ipairs(NAMES) do
      enums[name] = enumeration(name)
   end
end
local desired_vars = {
   "NUM_NATURES",
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

function try_handle_enumeration_member(name, value)
   if name:startswith("SPECIES_UNOWN_") then
      return true
   end
   for k, v in pairs(enums) do
      if name:startswith(k .. "_") then
         v:set(name, value)
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
   enums[name]:print()
end
enums.GROWTH:print()
enums.NATURE:print()
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
   local data = enums.FLAG:get(name)
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

include("../lu-lua-lib/dataview.lua")
do -- TEST: natures.dat
   local dst_path = "../lu-save-js/formats/1/natures.dat"
   local view     = dataview()
   
   function _write_subrecord(signature, version, functor)
      signature = tostring(signature)
      assert(#signature == 8)
      view:append_eight_cc(signature)
      view:append_uint32(version)
      local size_offset = view.size
      view:append_uint32(0) -- dummy for size
      functor(view)
      assert(view.size >= size_offset)
      local subrecord_size = view.size - size_offset
      view:set_uint32(size_offset, subrecord_size)
   end
   
   _write_subrecord("ENUMDATA", 1, function(view)
      local enumeration = enums.NATURE
      enumeration:update_cache()
      local signed = enumeration.cache.lowest < 0
      local prefix = enumeration.name .. "_"
      local sorted = enumeration:to_sorted_pairs()
      
      do
         local flags = 0
         if signed then
            flags = flags | 1 -- FLAG: Enum values should be interpreted as signed.
         end
         if enumeration.cache.sparse then
            flags = flags | 2 -- FLAG: Enum is sparse.
         end
         if enumeration.cache.lowest ~= 0 then
            flags = flags | 3 -- FLAG: Enum's lowest value is non-zero.
         end
         view:append_uint8(flags)
      end
      view:append_length_prefixed_string(1, prefix)
      view:append_uint32(enumeration.cache.count)
      
      local names_start = nil
      if enumeration.cache.sparse then
         names_start = view.size
         view:append_uint32(0)
      else
         if enumeration.cache.lowest ~= 0 then
            view:append_uint32(enumeration.cache.lowest or 0)
         end
      end
      do -- Serialize enum member names as a block
         local first = true
         for _, pair in ipairs(sorted) do
            if first then
               first = false
            else
               view:append_uint8(0x00) -- separator
            end
            local name = pair[1]:sub(#prefix + 1)
            view:append_raw_string(name)
         end
      end
      if enumeration.cache.sparse then
         view:set_uint32(name_start, view.size - name_start)
         --
         -- Serialize enum values as a block
         --
         for _, pair in ipairs(sorted) do
            view:append_uint32(pair[2])
         end
      end
   end)
   _write_subrecord("VARIABLS", 1, function(view)
      function _write_variable(name, value)
         if not value then
            return
         end
         view:append_length_prefixed_string(2, name)
         view:append_uint8((value < 0) and 0x01 or 0x00) -- value < 0 ? 1 : 0 -- "is signed" byte
         view:append_uint32(value)
      end
      
      _write_variable("NUM_NATURES", all_found_macros["NUM_NATURES"])
   end)
   
   view:print()
end
