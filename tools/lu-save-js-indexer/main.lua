this_dir = (function()
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

-- `require` is janky and seems to choke on relative paths, 
-- so this shims it and allows lookups based on relative 
-- paths
function include(path)
   local dst_folder = path
      :gsub("^([^/])", "./%1")
      :gsub("[^/]*$", "")
   local dst_file = path:gsub("([^/]*)$", "%1")
   --
   -- Override `package.path` to tell it to look for an exact file.
   --
   local prior = package.path
   package.path = this_dir .. dst_file
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

do -- ensure expected CWD
   local cwd  = shell:resolve(".")
   local here = shell:resolve(this_dir)
   if cwd == here then
      shell:cd("../../") -- CWD is this script's folder; back out to repo folder
   end
end
local data = shell:run_and_read('$DEVKITARM/bin/arm-none-eabi-gcc -Iinclude -nostdinc -E -dD -DMODERN=1 -x c "src/lu/preprocessor-macros-of-interest.txt"')

include("../lu-lua-lib/c-lex.lua")
include("../lu-lua-lib/c-ast.lua")
include("../lu-lua-lib/c-parser.lua")
include("../lu-lua-lib/c-exec-integer-constant-expression.lua")

include("enumeration.lua")

-- Here, we specify what extra-data files should contain what 
-- information of interest.
local goals = {
   ["flags.dat"] = {
      enums = {
         "FLAG",
         "TRAINER",
      },
      vars = {
         "TEMP_FLAGS_START",
         "TEMP_FLAGS_END", -- actually "last", not "end"
         "TRAINER_FLAGS_START",
         "TRAINER_FLAGS_END", -- actually "last", not "end"
         "TRAINERS_COUNT",
         "DAILY_FLAGS_START",
         "DAILY_FLAGS_END", -- actually "last", not "end"
      }
   },
   ["game-stats.dat"] = {
      enums = { "GAME_STAT" },
      vars  = {
         "NUM_USED_GAME_STATS",
         "NUM_GAME_STATS",
      }
   },
   ["items.dat"] = {
      enums = { "ITEM" }
   },
   ["moves.dat"] = {
      enums = { "MOVE" }
   },
   ["misc.dat"] = {
      enums = {
         "CONTEST_CATEGORY",
         "CONTEST_RANK",
         "GROWTH",
      },
      vars = {
         "SHINY_ODDS",
         
         -- Pokemon gender ratio sentinel values
         "MON_MALE",
         "MON_FEMALE",
         "MON_GENDERLESS",
      }
   },
   ["natures.dat"] = {
      enums = { "NATURE" }
   },
   ["species.dat"] = {
      enums = { "SPECIES" }
   },
   ["vars.dat"] = {
      enums = { "VAR" },
      vars  = {
         "VARS_START",
         "TEMP_VARS_START",
         "TEMP_VARS_END", -- actually "last," not "end"
      }
   },
}

local desired_vars = {
   -- Though not emitted into any specific extra-data file, this 
   -- variable is mission-critical for generating the files. We 
   -- want to list it here so that we track it in a more compact 
   -- table (`found_vars_of_interest` versus `all_found_macros`).
   "SAVEDATA_SERIALIZATION_VERSION",
}
local enums = {}
do
   local en_count   = 0
   local dv_count   = #desired_vars
   local seen_enums = {}
   local seen_vars  = {}
   for _, v in pairs(goals) do
      local de = v.enums
      local dv = v.vars
      if de then
         for _, name in ipairs(de) do
            if not seen_enums[name] then
               seen_enums[name] =  true
               en_count = en_count + 1
               enums[name] = enumeration(name)
            end
         end
      end
      if dv then
         for _, name in ipairs(dv) do
            if not seen_vars[name] then
               seen_vars[name] = true
               dv_count = dv_count + 1
               desired_vars[dv_count] = name
            end
         end
      end
   end
end

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
   --
   -- Prevent mistaking certain vars for enum members.
   --
   if name:startswith("TRAINER_FLAGS_") then
      return false
   end
   --
   -- Exclude a few things from the enums.
   --
   if name:startswith("SPECIES_UNOWN_") then
      return true
   end
   if name == "MOVE_UNAVAILABLE" then
      return true
   end
   if name:startswith("FLAG_") then
      --
      -- Exclude "special flags," as these aren't actually present in 
      -- the savegame.
      --
      local special = all_found_macros["SPECIAL_FLAGS_START"]
      if special and value >= special then
         return true
      end
   end
   if name:startswith("VAR_") then
      --
      -- Exclude "special vars," as these aren't actually present in 
      -- the savegame.
      --
      local special = all_found_macros["SPECIAL_VARS_START"]
      if special and value >= special then
         return true
      end
   end
   if name:startswith("ITEM_") then
      if name == "ITEM_LIST_END"
      or name == "ITEM_TO_BERRY"
      or name == "ITEM_TO_MAIL"
      or name == "ITEM_HAS_EFFECT"
      then
         return true
      end
      if name:startswith("ITEM_USE_")
      or name:startswith("ITEM_B_USE_")
      then
         return true
      end
   end
   --
   -- Check if this name matches any enum and if so, remember the 
   -- name/value pair.
   --
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
         if value then
            all_found_macros[name] = value
            try_handle_macro(name, value)
         end
      end
   end
end

if false then
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
   --
   -- TODO: FLAG_REGISTERED_ROSE fails because values like REMATCH_ROSE aren't 
   -- preprocessor macros; they're enumeration members (in `constants/rematch.h`).
   -- Don't suppose GCC has a flag to print *those* for us too? :\ 
   --
   debug_print_flag("FLAG_HIDDEN_ITEM_LAVARIDGE_TOWN_ICE_HEAL")
   debug_print_flag("FLAG_UNUSED_0x91F")
   debug_print_flag("FLAG_UNUSED_0x920")
end

if not found_vars_of_interest["SAVEDATA_SERIALIZATION_VERSION"] then
   error("Unable to find the SAVEDATA_SERIALIZATION_VERSION macro, or its value could not be resolved to an integer constant.")
end

include("../lu-lua-lib/dataview.lua")
include("subrecords.lua")

local format_version = found_vars_of_interest["SAVEDATA_SERIALIZATION_VERSION"]
local base_dir       = "tools/lu-save-js/formats/" .. tostring(format_version) .. "/"

--
-- Output files:
--

-- Format XML
shell:exec("cp lu_bitpack_savedata_format.xml " .. base_dir .. "format.xml")

-- Extra-data files
--[[--
for filename, info in pairs(goals) do
   local dst_path = base_dir .. filename
   local view     = dataview()
   local wrote    = false
   if info.enums then
      for _, name in ipairs(info.enums) do
         local enumeration = enums[name]
         if enumeration then
            wrote = true
            write_ENUMDATA_subrecord(view, enumeration)
         end
      end
   end
   if info.vars then
      local pair_list = {}
      for _, name in ipairs(info.vars) do
         local value = found_vars_of_interest[name]
         if value then
            pair_list[#pair_list + 1] = { name, value }
         end
      end
      wrote = true
      write_VARIABLS_subrecord(view, pair_list)
   end
   if wrote then
      view:save_to_file(this_dir.."/../../"..dst_path)
   else
      os.remove(this_dir.."/../../"..dst_path)
   end
end
--]]--

io.write("Generating and indexing extra-data files for the save editor...\n")

--
-- Update the index file.
--
include("../lu-lua-lib/json.lua")
local function get_file_size(path)
   local file = io.open(this_dir..path, "r")
   if file then
      local size = file:seek("end")
      file:close()
      return size
   end
end
local function file_is_identical_to(path, view)
   local size = get_file_size(path)
   if not size then
      return false
   end
   if size ~= view.size then
      return false
   end
   local src = dataview()
   src:load_from_file(this_dir..path)
   if view:is_identical_to(src) then
      return true
   end
   return false
end
do
   local INDEX_VERSION <const> = 1

   local index_data = {}
   do
      local file = io.open(this_dir.."/../lu-save-js/formats/index.json", "r")
      if file then
         local data = file:read("*all")
         xpcall(
            function()
               index_data = json.from(data)
            end,
            function(err)
               print("Failed to load the index file.")
               print("Error: " .. tostring(err))
               print(debug.traceback())
               error("Halting execution of this post-build script.")
            end
         )
      end
      if not index_data then
         index_data = {}
      end
   end
   
   do -- versioning
      local prior_version = index_data.index_version
      if prior_version and prior_version ~= INDEX_VERSION then
         --
         -- TODO: If we ever change how this JSON data is laid out, here's 
         --       where we'd want to adapt old layouts to the new one.
         --
      end
      index_data.index_version = INDEX_VERSION
   end
   if not index_data.formats then
      index_data.formats = {}
   end
   index_data.formats[format_version] = nil
   
   -- Generate a map such that prior_files[filename] is a list of savedata 
   -- formats which contain (rather than share) the given file, with the 
   -- list in reverse order (newest formats first).
   local prior_files = {}
   for version, prior_format in pairs(index_data.formats) do
      version = tonumber(version)
      if version and prior_format.files then
         for _, filename in ipairs(prior_format.files) do
            local list = prior_files[filename]
            if not list then
               list = {}
               prior_files[filename] = list
            end
            list[#list + 1] = version
         end
      end
   end
   for filename, list in pairs(prior_files) do
      table.sort(list, function(a, b) return b < a end)
   end
   
   local new_format = {
      files  = {},
      shared = {},
   }
   index_data[format_version] = new_format
   
   for filename, info in pairs(goals) do
      local dst_path = base_dir .. filename
      --
      -- Generate an extra-data file in memory.
      --
      local view     = dataview()
      local wrote    = false
      if info.enums then
         for _, name in ipairs(info.enums) do
            local enumeration = enums[name]
            if enumeration then
               wrote = true
               write_ENUMDATA_subrecord(view, enumeration)
            end
         end
      end
      if info.vars then
         local pair_list = {}
         for _, name in ipairs(info.vars) do
            local value = found_vars_of_interest[name]
            if value then
               pair_list[#pair_list + 1] = { name, value }
            end
         end
         wrote = true
         write_VARIABLS_subrecord(view, pair_list)
      end
      if wrote then
         --
         -- The extra-data file has any content. Diff it against prior 
         -- formats, to figure out whether we should create a new file 
         -- (if this file is unique) or reuse an existing file.
         --
         local share = nil
         do
            local list  = prior_files[filename]
            if list then
               for _, prior in ipairs(list) do
                  if prior < format_version then
                     local prior_path = "/../lu-save-js/formats/"..prior.."/"..filename
                     if file_is_identical_to(prior_path, view) then
                        share = prior
                        break
                     end
                  end
               end
            end
         end
         if share then
            new_format.shared[filename] = share
            print(" - Sharing `"..filename.."` with previously-indexed savedata format `"..prior.."`.")
            os.remove(this_dir.."/../../"..dst_path)
         else
            view:save_to_file(this_dir.."/../../"..dst_path)
            print(" - Generated `"..filename.."` for the current savedata format.")
            new_format.files[#new_format.files + 1] = filename
         end
      else
         os.remove(this_dir.."/../../"..dst_path)
      end
   end
   print(" - All extra-data files generated or shared as appropriate.")
   
   local text = json.to(index_data, { pretty_print = true })
   do
      local file = io.open(this_dir.."/../lu-save-js/formats/index.json", "w")
      file:write(text)
      file:close()
   end
   print(" - Updated `index.json`.")
end
