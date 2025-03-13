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
include("../lu-lua-lib/filesystem.lua")

do -- ensure expected CWD
   local cwd  = shell:resolve(".")
   local here = shell:resolve(this_dir)
   if cwd == here then
      shell:cd("../../") -- CWD is this script's folder; back out to repo folder
      filesystem.current_path("../../")
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

-- Here, we specify "enum-confusables:" preprocessor macro names 
-- that look like they're part of various enums, but aren't. The 
-- entries in this array may end in '*', to denote prefixes that 
-- are confusable. Wildcards elsewhere are not permitted.
local enum_confusables = {
   "FLAG_HIDDEN_ITEMS_START",

   -- Temp flag aliases. Exclude these to ensure consistent naming 
   -- when temp flags may have multiple purposes.
   "FLAG_TEMP_SKIP_GABBY_INTERVIEW",
   "FLAG_TEMP_REGICE_PUZZLE_STARTED",
   "FLAG_TEMP_REGICE_PUZZLE_FAILED",
   "FLAG_TEMP_HIDE_MIRAGE_ISLAND_BERRY_TREE",
   
   "ITEM_HAS_EFFECT",
   "ITEM_LIST_END",
   "ITEM_NAME_LENGTH",
   "ITEM_TO_BERRY",
   "ITEM_TO_MAIL",
   "ITEM_USE_*",
   "ITEM_B_USE_*",
   
   "MOVE_NAME_LENGTH",
   "MOVE_UNAVAILABLE",
   
   "SPECIES_UNOWN_*",
   
   "TRAINER_FLAGS_*",
   "TRAINER_ID_LENGTH",
   "TRAINER_NAME_LENGTH",
   "TRAINER_REGISTERED_FLAGS_START",
   
   -- Temp var aliases. Exclude these to ensure consistent naming 
   -- when temp vars may have multiple purposes.
   "VAR_TEMP_CHALLENGE_STATUS",
   "VAR_TEMP_MIXED_RECORDS",
   "VAR_TEMP_RECORD_MIX_GIFT_ITEM",
   "VAR_TEMP_PLAYING_PYRAMID_MUSIC",
   "VAR_TEMP_FRONTIER_TUTOR_SELECTION",
   "VAR_TEMP_FRONTIER_TUTOR_ID",
   "VAR_TEMP_TRANSFERRED_SPECIES",
}

-- Indicate names or name formats for unused enum members.
local enum_unused_names = {
   ["FLAG"] = {
      "FLAG_UNUSED_0x*"
   }
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
   for _, confusable in ipairs(enum_confusables) do
      local s = confusable:sub(#confusable)
      if s == "*" then
         s = confusable:sub(1, -2)
         if name:startswith(s) then
            return false
         end
      elseif name == confusable then
         return false
      end
   end
   if name:startswith("FLAG_") then
      --
      -- Exclude "special flags," as these aren't actually present in 
      -- the savegame.
      --
      local special = all_found_macros["SPECIAL_FLAGS_START"]
      if special and value >= special then
         return false
      end
   end
   if name:startswith("VAR_") then
      --
      -- Exclude "special vars," as these aren't actually present in 
      -- the savegame.
      --
      local special = all_found_macros["SPECIAL_VARS_START"]
      if special and value >= special then
         return false
      end
   end
   --
   -- Check if this name matches any enum and if so, remember the 
   -- name/value pair.
   --
   for k, v in pairs(enums) do
      if name:startswith(k .. "_") then
         do
            local unused_names = enum_unused_names[k]
            if unused_names then
               local unused = false
               for _, v in ipairs(unused_names) do
                  if v:sub(#v) == "*" then
                     v = v:sub(1, -2)
                     if name:startswith(v) then
                        unused = true
                        break
                     end
                  elseif name == v then
                     unused = true
                     break
                  end
               end
               if unused then
                  v:mark_value_unused(value)
                  return true
               end
            end
         end
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

if not found_vars_of_interest["SAVEDATA_SERIALIZATION_VERSION"] then
   error("Unable to find the SAVEDATA_SERIALIZATION_VERSION macro, or its value could not be resolved to an integer constant.")
end

include("../lu-lua-lib/dataview.lua")
include("subrecords.lua")

local save_editor_dir = filesystem.path("usertools/lu-save-js/")

local format_version = found_vars_of_interest["SAVEDATA_SERIALIZATION_VERSION"]
local format_dir     = save_editor_dir/("formats/"..tostring(format_version).."/")
if not filesystem.create_directory(format_dir) then
   error("Failed to create directory `"..tostring(format_dir).."` for savedata format "..format_version..".")
end

--
-- Output files:
--

-- Format XML
print("Copying the current savedata format XML into the appropriate folder for the save editor...\n")
filesystem.copy_file(
   "lu_bitpack_savedata_format.xml",
   format_dir/"format.xml"
)

print("Generating and indexing extra-data files for the save editor...\n")

--
-- Update the index file.
--
include("../lu-lua-lib/json.lua")
local function file_is_identical_to(path, view)
   local size = filesystem.file_size(path)
   if not size then
      return false
   end
   if size ~= view.size then
      return false
   end
   local src = dataview()
   src:load_from_file(tostring(filesystem.absolute(path)))
   if view:is_identical_to(src) then
      return true
   end
   return false
end
do
   local INDEX_VERSION <const> = 1

   local index_data = {}
   do
      local file = filesystem.open(save_editor_dir/"formats/index.json", "r")
      if file then
         local data    = file:read("*all")
         local success = true
         xpcall(
            function()
               index_data = json.from(data)
            end,
            function(err)
               print("Failed to load the index file.")
               print("Error: " .. tostring(err))
               print(debug.traceback())
               success = false
            end
         )
         if not success then
            error("Halting execution of this post-build script.")
         end
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
   if index_data.formats then
      --
      -- Convert from an array to a map, as necessary.
      --
      local mt = getmetatable(index_data.formats)
      if mt then
         if mt.__json_type == "array" then
            local remapped = {}
            for k, v in pairs(index_data.formats) do
               remapped[k - 1] = v
            end
            setmetatable(remapped, mt)
            index_data.formats = remapped
         end
         mt.__json_is_zero_indexed = true
      else
         local mt = { __json_is_zero_indexed = true }
         setmetatable(index_data.formats, mt)
      end
   else
      index_data.formats = {}
      
      local mt = { __json_is_zero_indexed = true }
      setmetatable(index_data.formats, mt)
   end
   index_data.formats[format_version] = nil
   
   -- Generate a map such that prior_files[filename] is a list of savedata 
   -- formats which contain (rather than share) the given file, with the 
   -- list in reverse order (newest formats first).
   local prior_files = {}
   for version, prior_format in pairs(index_data.formats) do
      version = tonumber(version)
      if version and prior_format and prior_format.files then
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
   index_data.formats[format_version] = new_format
   
   for filename, info in pairs(goals) do
      local dst_path = format_dir/filename
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
               write_ENUNUSED_subrecord(view, enumeration)
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
                     local prior_path = save_editor_dir / ("formats/"..prior.."/"..filename)
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
            print(" - Sharing `"..filename.."` with previously-indexed savedata format `"..share.."`.")
            filesystem.remove(dst_path)
         else
            view:save_to_file(tostring(filesystem.absolute(dst_path)))
            print(" - Generated `"..filename.."` for the current savedata format.")
            new_format.files[#new_format.files + 1] = filename
         end
      else
         filesystem.remove(dst_path)
      end
   end
   do -- Normalize output.
      if #new_format.files == 0 then
         new_format.files = nil
      else
         table.sort(new_format.files) -- ensure consistent ordering
      end
      do
         local k, v = next(new_format.shared, nil)
         if not k then -- if table is empty
            new_format.shared = nil
         end
      end
   end
   print(" - All extra-data files generated or shared as appropriate.")
   
   local text = json.to(index_data, { pretty_print = true })
   do
      local file = filesystem.open(save_editor_dir/"formats/index.json", "w")
      file:write(text)
      file:close()
   end
   print(" - Updated `index.json`.")
end
