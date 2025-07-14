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
package.path = package.path .. ";" .. this_dir .. "/?.lua;" .. this_dir .. "../lu-lua-lib/?.lua"

local build_options = {
   build_name = nil,
}
if arg then
   local i     = 1
   local count = #arg
   while i <= count do
      local chunk = arg[i]
      i = i + 1
      
      if chunk == "--build-name" then
         local value = arg[i]
         build_options.build_name = value
         i = i + 1
      end
   end
end

require "stringify-table"
require "classes"
require "filesystem"
require "shell"
require "stdext"

do -- ensure expected CWD
   local cwd  = shell:resolve(".")
   local here = shell:resolve(this_dir)
   if cwd == here then
      shell:cd("../../") -- CWD is this script's folder; back out to repo folder
      filesystem.current_path("../../")
   end
end

require "for-each-macro-in"
require "enumeration"

-- Here, we specify what extra-data files should contain what 
-- information of interest.
local goals = {
   ["easy-chat.dat"] = {
      enums = {
         "EC_GROUP",
         "EC_WORD"
      },
      vars  = {
         "EC_MASK_BITS",
      }
   },
   ["flags.dat"] = {
      enums = { "FLAG" },
      vars = {
         "TEMP_FLAGS_START",
         "TEMP_FLAGS_END", -- actually "last", not "end"
         "TRAINER_FLAGS_START",
         "TRAINER_FLAGS_END", -- actually "last", not "end"
         "DAILY_FLAGS_START",
         "DAILY_FLAGS_END", -- actually "last", not "end"
      },
      slice_params = {
         pattern         = "flags-#.dat",
         enum_to_slice   = "FLAG",
         count_per_slice = 300
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
   ["maps.dat"] = {
      enums = {
         "MAPSEC", -- used for met locations
         "METLOC", -- used for met locations (same ID space as MAPSEC, but different prefix)
      },
      vars  = {
         "KANTO_MAPSEC_START",
         "KANTO_MAPSEC_END", -- actually "last," not "end"
         "KANTO_MAPSEC_COUNT"
      },
      map_data = true
   },
   ["moves.dat"] = {
      enums = { "MOVE" }
   },
   ["misc.dat"] = {
      enums = {
         "CONTEST_CATEGORY",
         "CONTEST_RANK",
         "FRONTIER_LVL",
         "GROWTH",
         "LANGUAGE",
         "OPTIONS_BATTLE_STYLE",
         "OPTIONS_BUTTON_MODE",
         "OPTIONS_SOUND",
         "OPTIONS_TEXT_SPEED",
         "VERSION", -- game versions
      },
      vars = {
         "SHINY_ODDS",
         
         "FRONTIER_LVL_MODE_COUNT",
         
         -- gender
         "MALE",
         "FEMALE",
         "GENDER_COUNT",
         
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
   ["trainers.dat"] = {
      enums = { "TRAINER" },
      vars = {
         "TRAINERS_COUNT",
      }
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
   
   "FRONTIER_LVL_MODE_COUNT",
   
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
   
   -- We need these variables in order to know what enum values 
   -- to ignore for FLAG and VAR: we want to ignore anything 
   -- that wouldn't actually make it into savedata.
   "SPECIAL_FLAGS_START",
   "SPECIAL_VARS_START",
}
local enums = {}
do -- Initialize `enumeration` objects for enums of interest, and initialize `desired_vars`.
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
local map_data = nil

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
      local special = found_vars_of_interest["SPECIAL_FLAGS_START"]
      if special and value >= special then
         return false
      end
   elseif name:startswith("VAR_") then
      --
      -- Exclude "special vars," as these aren't actually present in 
      -- the savegame.
      --
      local special = found_vars_of_interest["SPECIAL_VARS_START"]
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
         do -- Handle unused-name patterns for enums.
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

for_each_macro_in(
   filesystem.absolute(filesystem.path("src/lu/preprocessor-macros-of-interest.txt")),
   function(name, value)
      try_handle_macro(name, value)
   end
)
do -- Try and get map data.
   require "json"
   
   local file = filesystem.open("data/maps/map_groups.json")
   if file then
      local data    = file:read("*all")
      local success = true
      local parsed
      file:close()
      xpcall(
         function()
            parsed = json.from(data)
         end,
         function(err)
            print("Failed to parse data/maps/map_groups.json.")
            print("Error: " .. tostring(err))
            print(debug.traceback())
            success = false
         end
      )
      if success and parsed then
         if parsed.group_order then
            map_data = {
               group_order = parsed.group_order,
               groups = {}
            }
            for _, name in ipairs(parsed.group_order) do
               if name ~= "group_order" and parsed[name] then
                  map_data.groups[name] = parsed[name]
               else
                  map_data.groups[name] = {}
               end
            end
         end
      end
   end
end

if not found_vars_of_interest["SAVEDATA_SERIALIZATION_VERSION"] then
   error("Unable to find the SAVEDATA_SERIALIZATION_VERSION macro, or its value could not be resolved to an integer constant.")
end

require "dataview"
require "subrecords"

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
filesystem.copy_file("lu_bitpack_savedata_format.xml", format_dir/"format.xml")

print("Generating and indexing extra-data files for the save editor...\n")

--
-- Update the index file.
--
require "json"

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

   local index_data
   do
      local file = filesystem.open(save_editor_dir/"formats/index.json", "r")
      if file then
         local data    = file:read("*all")
         local success = true
         file:close()
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
   if build_options.build_name then
      new_format.name = tostring(build_options.build_name)
   end
   index_data.formats[format_version] = new_format
   
   local function do_goal(filename, info)
      local wrote_any = false
      
      local function write_enumerations(view, except)
         if not info.enums then
            return
         end
         for _, name in ipairs(info.enums) do
            local enumeration = enums[name]
            if enumeration then
               wrote_any = true
               if enumeration ~= except then
                  --
                  -- Some especially large enums are sliced into multiple extra-data 
                  -- files. In these cases, we don't want to write ENUMDATA for those 
                  -- enums here, because we have a separate function which writes the 
                  -- ENUMDATA for each slice.
                  --
                  write_ENUMDATA_subrecord(view, enumeration)
                  --
                  -- ...but we DO want to write ENUNUSED and similar metadata to the 
                  -- first such slice, so we write those unconditionally, below.
                  --
               end
               write_ENUNUSED_subrecord(view, enumeration)
            end
         end
      end
      local function write_map_data(view)
         if not map_data then
            return
         end
         local size = view.size
         write_MAPSDATA_subrecord(view, map_data)
         if view.size > size then
            wrote_any = true
         end
      end
      local function write_variables(view)
         if not info.vars then
            return
         end
         local pair_list = {}
         for _, name in ipairs(info.vars) do
            local value = found_vars_of_interest[name]
            if value then
               pair_list[#pair_list + 1] = { name, value }
            end
         end
         if #pair_list > 0 then
            wrote_any = true
            write_VARIABLS_subrecord(view, pair_list)
         end
      end
      
      -- Check if the file that we'd generate for the current format version 
      -- is exactly identical to that of any previous format version. If so, 
      -- return the previous format version number.
      local function file_is_shared(filename, view)
         local list = prior_files[filename]
         if not list then
            return
         end
         for _, prior in ipairs(list) do
            if prior < format_version then
               local prior_path = save_editor_dir / ("formats/"..prior.."/"..filename)
               if file_is_identical_to(prior_path, view) then
                  return prior
               end
            end
         end
      end
      
      -- If we didn't generate any data, then delete any file that may already 
      -- exist for this format version. Otherwise, share or own the file as 
      -- appropriate.
      local function finalize_file(filename, view)
         local dst_path = format_dir/filename
         if not wrote_any then
            filesystem.remove(dst_path)
            return
         end
         local share = file_is_shared(filename, view)
         if share then
            new_format.shared[filename] = share
            print(" - Sharing `"..filename.."` with previously-indexed savedata format `"..share.."`.")
            filesystem.remove(dst_path)
         else
            view:save_to_file(tostring(filesystem.absolute(dst_path)))
            print(" - Generated `"..filename.."` for the current savedata format.")
            new_format.files[#new_format.files + 1] = filename
         end
      end
   
      if info.slice_params and info.enums then
         --
         -- Some enums are expected to be especially large (think 30+KB total 
         -- data), so we want to write them "sliced" in multiple extra-data 
         -- files. That way, if a savedata version changes, say, the name of 
         -- one single flag, it only needs to generate its own file for one 
         -- of those slices, rather than having 30KB of nearly identical data 
         -- with just one name changed.
         --
         local function should_slice_enum(enum, slice_size)
            enum:update_cache()
            if enum.cache.count < slice_size * 2 then
               return false
            end
            if (enum.cache.lowest or 0) < 0 then
              return false
            end
            return true
         end
      
         local enum_to_slice
         if table.findindex(info.enums, info.slice_params.enum_to_slice) then
            enum_to_slice = enums[info.slice_params.enum_to_slice]
         end
         if enum_to_slice and should_slice_enum(enum_to_slice, info.slice_params.count_per_slice) then
            local slices = write_ENUMDATA_slice_subrecords(enum_to_slice, info.slice_params.count_per_slice)
            
            local first_slice = slices[1]
            
            write_enumerations(first_slice, enum_to_slice)
            if info.map_data then
               write_map_data(first_slice)
            end
            write_variables(first_slice)
            
            local count = #slices
            for i = 1, count do
               local dst_name = info.slice_params.pattern:gsub("#", tostring(i - 1))
               finalize_file(dst_name, slices[i])
            end
            return
         end
      end
      --
      -- Write a non-sliced/whole file.
      --
      local view = dataview()
      write_enumerations(view, enum_to_slice)
      if info.map_data then
         write_map_data(view)
      end
      write_variables(view)
      finalize_file(filename, view)
   end
   
   for filename, info in pairs(goals) do
      do_goal(filename, info)
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
