-- `require` is janky and seems to choke on relative paths, 
-- so this shims it and allows lookups based on relative 
-- paths
function include(path)
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

include("../lu-lua-lib/classes.lua")
include("../lu-lua-lib/stringify-table.lua")
include("../lu-lua-lib/xml.lua")

local root_node
do
   local file = io.open("../../lu_bitpack_savedata_format.xml", "r")
   if not file then
      error("XML file could not be opened.")
   end
   local raw    = file:read("*all")
   local parser = xml.parser()
   parser:parse(raw)
   root_node = parser.root
   if not root_node then
      error("Parser failed to produce a result?")
   end
end

local sector_count
local sector_size
do
   local config = root_node:children_by_node_name("config")[1]
   assert(config, "Missing node: data:root > config.")
   config:for_each_child_element(function(node)
      local name = node.attributes["name"]
      if name == "max-sector-bytecount" then
         sector_size = tonumber(node.attributes["value"])
      elseif name == "max-sector-count" then
         sector_count = tonumber(node.attributes["value"])
      end
   end)
   assert(sector_count, "Missing node: data:root > config > option[name='max-sector-count'].")
   assert(sector_size, "Missing node: data:root > config > option[name='max-sector-bytecount'].")
end

local out_file = io.open("../../reports/savedata.md.tmp", "w")
if not out_file then
   error("Unable to open the output file for writing.")
end

out_file:write([=[
# Savedata format

This project uses an automatically-generated savedata format.

]=])

local type_nodes = {}
do
   local base = root_node:children_by_node_name("c-types")[1]
   assert(base, "Missing node: data:root > c-types")
   base:for_each_child_element(function(node)
      local name = node.attributes["name"]
      if not name then
         name = node.attributes["tag"]
         if not name then
            return
         end
      end
      type_nodes[name] = node
   end)
end

function strip_c_type(name)
   name = name:gsub("^const ", "")
   name = name:gsub("^volatile ", "")
   return name
end
function to_percentage(n)
   n = n * 100
   return tostring(math.floor(n * 100) / 100) .. "%"
end

out_file:write("## Stats\n\n")

local top_level_values = {}
do
   local base = root_node:children_by_node_name("top-level-values")[1]
   assert(base, "Missing node: data:root > top-level-values")
   base:for_each_child_element(function(node)
      local entry = {}
      entry.name  = node.attributes["name"]
      entry.deref = tonumber(node.attributes["dereference-count"] or "0")
      entry.types = {
         original   = node.attributes["type"],
         serialized = node.attributes["serialized-type"]
      }
      if not entry.types.original then
         entry.types.original = entry.types.serialized
      end
      entry.force_to_next_sector = node.attributes["force-to-next-sector"] == "true"
      top_level_values[#top_level_values + 1] = entry
   end)
end

do -- Stats per sector
   out_file:write("### Stats per sector\n\n")
   out_file:write("| Sector | Unpacked bytes | Unpacked % | Packed bytes | Packed bits | Packed % |\n")
   out_file:write("| -: | -: | -: | -: | -: | -: |\n")
   
   local tlv_index  = 1
   local tlv_byte   = 0
   local tlv_forced = true
   
   local total_unpacked_bytes = 0
   local total_packed_bits    = 0
   local total_lost_bits      = 0

   local base = root_node:children_by_node_name("sectors")[1]
   assert(base, "Missing node: data:root > sectors")
   local list = base:children_by_node_name("sector")
   for i = 1, sector_count do
      out_file:write("|")
      out_file:write(i - 1)
      out_file:write("|")
      
      local sector_forced_to_end = false
      
      if tlv_index <= #top_level_values then
         local total_size = 0
         while tlv_index <= #top_level_values do
            local tlv  = top_level_values[tlv_index]
            local size
--print(i .. ": " .. tlv_index .. ": " .. tlv.name)
            do
               local tn = strip_c_type(tlv.types.original)
               if tlv.deref > 0 then
                  for j = 1, tlv.deref do
                     local size = #tn
                     if tn:sub(size, size) == "*" then
                        tn = tn:sub(1, size - 1)
                     end
                  end
               end
               size = type_nodes[tn].attributes["c-sizeof"]
            end
            
            if tlv.force_to_next_sector and not tlv_forced then
               --
               -- This object is forced to the next sector.
               --
--print(" - forced to next")
               tlv_forced = true
               tlv_byte   = 0
               sector_forced_to_end = true
               break
            else
               local remaining_size = size - tlv_byte
            
               size = size - tlv_byte
               if total_size + remaining_size <= sector_size then
                  --
                  -- This object fits wholly within the current sector.
                  --
   --print(" - fits (" .. remaining_size .. " bytes)")
                  tlv_index  = tlv_index + 1
                  tlv_byte   = 0
                  total_size = total_size + remaining_size
                  tlv_forced = false
               else
                  --
                  -- This object is split across sectors.
                  --
                  local fit_size = sector_size - total_size
                  tlv_byte   = tlv_byte + fit_size
   --print(" - splits (" .. size .. " bytes; " .. fit_size .. " bytes fit; we will continue from " .. tlv_byte .. ")")
                  total_size = sector_size
                  break
               end
            end
            
            -- For structs split across sectors
            tlv_byte = 0
         end
         out_file:write(total_size)
         out_file:write("|")
         out_file:write(to_percentage(total_size / sector_size))
         out_file:write("|")
         
         total_unpacked_bytes = total_unpacked_bytes + total_size
      else
         out_file:write("0|0%|")
      end
      
      local node = list[i]
      if node then
         local bits = tonumber(node:children_by_node_name("stats")[1]:children_by_node_name("bitcounts")[1].attributes["total-packed"])
         out_file:write(math.ceil(bits / 8))
         out_file:write("|")
         out_file:write(bits)
         out_file:write("|")
         out_file:write(to_percentage(bits / (sector_size * 8)))
         out_file:write("|")
         
         total_packed_bits = total_packed_bits + bits
         if not sector_forced_to_end then
            total_lost_bits = total_lost_bits + ((sector_size * 8) - bits)
         end
      else
         out_file:write("|||")
      end
      out_file:write("\n")
   end
   out_file:write("\n")
   
   out_file:write("Totals:\n\n")
   
   out_file:write("* **Total unpacked bytes:** ")
   out_file:write(total_unpacked_bytes)
   out_file:write(" (")
   out_file:write(to_percentage(total_unpacked_bytes / (sector_size * sector_count)))
   out_file:write(")\n")
   
   out_file:write("* **Total packed bits:** ")
   out_file:write(total_packed_bits)
   out_file:write(" (")
   out_file:write(to_percentage(total_packed_bits / (sector_size * 8 * sector_count)))
   out_file:write(")\n")
   out_file:write("* **Total lost bits:** ")
   out_file:write(total_lost_bits)
   out_file:write(" (")
   out_file:write(to_percentage(total_lost_bits / (sector_size * 8 * sector_count)))
   out_file:write(")\n")
   out_file:write("  * Some values can't be split across sectors. If these values can't fit at the end of a sector, then they must be pushed to the start of the next sector &mdash; leaving unused space at the end of the sector they were pushed past.\n")
   
   out_file:write("\n")
end

do -- Stats per value category
   out_file:write("### Bitpack value categories\n\n")
   
   local base = root_node:children_by_node_name("categories")[1]
   assert(base, "Missing node: data:root > categories")
   local list = base:children_by_node_name("category")
   for i = 1, #list do
      local node = list[i]
      local name = node.attributes["name"]
      out_file:write("#### ")
      out_file:write(name)
      out_file:write("\n\n")
      
      local bc = node:children_by_node_name("bitcounts")[1]
      local c  = node:children_by_node_name("counts")[1]
      
      local unpacked = bc.attributes["total-unpacked"]
      local packed   = bc.attributes["total-packed"]
      
      out_file:write("* **Total unpacked size:** ")
      out_file:write(unpacked)
      out_file:write(" bits\n")
      out_file:write("* **Total packed size:** ")
      out_file:write(packed)
      out_file:write(" bits\n")
      out_file:write("* **Total size reduction:** ")
      out_file:write(to_percentage(packed / unpacked))
      out_file:write("\n")
      do
         local any = false
         c:for_each_child_element(function(child)
            if child.node_name ~= "in-sector" then
               return
            end
            if not any then
               any = true
               out_file:write("* **Counts by sector:**\n")
            end
            out_file:write("  * **Sector ")
            out_file:write(child.attributes["index"])
            out_file:write(":** ")
            out_file:write(child.attributes["count"])
            out_file:write("\n")
         end)
      end
      do
         local any = false
         c:for_each_child_element(function(child)
            if child.node_name ~= "in-top-level-value" then
               return
            end
            if not any then
               any = true
               out_file:write("* **Counts by top-level value:**\n")
            end
            out_file:write("  * **Value `")
            out_file:write(child.attributes["name"])
            out_file:write("`:** ")
            out_file:write(child.attributes["count"])
            out_file:write("\n")
         end)
      end
      out_file:write("\n")
   end
   out_file:write("\n")
end