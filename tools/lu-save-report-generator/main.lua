this_dir = (function()
   --
   -- Abuse debug info to find out where the current file is located.
   --
   return debug.getinfo(1, 'S').source
      :sub(2)
      :gsub("^([^/])", "./%1")
      :gsub("[^/]*$", "")
end)()
package.path = package.path .. ";" .. this_dir .. "/?.lua;" .. this_dir .. "../lu-lua-lib/?.lua"

require "classes"
require "stringify-table"
require "xml"

local root_node
do
   local file = io.open(this_dir.."../../lu_bitpack_savedata_format.xml", "r")
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

function strip_c_type(name)
   name = name:gsub("^const ", "")
   name = name:gsub("^volatile ", "")
   return name
end

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
      node:for_each_child_element(function(child)
         if child.node_name ~= "typedef" then
            return
         end
         type_nodes[child.attributes["name"]] = node
      end)
   end)
end

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

local sector_size_info = {
   by_sector = {},
   totals = {
      unpacked = 0, -- bytes
      packed   = 0, -- bits
      lost     = 0, -- bits
   }
}
do
   local dst = sector_size_info

   local tlv_index  = 1
   local tlv_byte   = 0
   local tlv_forced = true
   
   local base = root_node:children_by_node_name("sectors")[1]
   assert(base, "Missing node: data:root > sectors")
   local list = base:children_by_node_name("sector")
   for i = 1, sector_count do
      local sector = {
         forced_to_end = false,
         sizes = {
            unpacked = 0, -- bits
            packed   = 0, -- bits
            lost     = 0, -- bits
         }
      }
      dst.by_sector[#dst.by_sector + 1] = sector
   
      local sector_forced_to_end = false
      
      if tlv_index <= #top_level_values then
         local total_size = 0
         while tlv_index <= #top_level_values do
            local tlv  = top_level_values[tlv_index]
            local size
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
               tlv_forced = true
               tlv_byte   = 0
               sector.forced_to_end = true
               break
            else
               local remaining_size = size - tlv_byte
            
               size = size - tlv_byte
               if total_size + remaining_size <= sector_size then
                  --
                  -- This object fits wholly within the current sector.
                  --
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
                  total_size = sector_size
                  break
               end
            end
            
            -- For structs split across sectors
            tlv_byte = 0
         end
         sector.sizes.unpacked = total_size
         dst.totals.unpacked = dst.totals.unpacked + total_size
      end
      
      local node = list[i]
      if node then
         local bits = tonumber(node:children_by_node_name("stats")[1]:children_by_node_name("bitcounts")[1].attributes["total-packed"])
         sector.sizes.packed = bits
         dst.totals.packed = dst.totals.packed + bits
         if not sector.forced_to_end and i < #list then
            sector.sizes.lost = (sector_size * 8) - bits
            dst.totals.lost = dst.totals.lost + sector.sizes.lost
         end
      end
   end
end

local out_file = io.open(this_dir.."../../reports/savedata.md.tmp", "w")
if not out_file then
   error("Unable to open the output file for writing.")
end

function to_percentage(n)
   n = n * 100
   return string.format("%.2f%%", n)
end

out_file:write("# Savedata format\n\n")
out_file:write("<!-- This file is auto-generated by a post-build script. Changes made to it will be lost.. -->\n\n")
do
   local available_bytes = sector_size * sector_count
   local available_bits  = available_bytes * 8
   local total_bits_used = sector_size_info.totals.packed + sector_size_info.totals.lost

   out_file:write("This project uses an automatically-generated savedata format wherein savedata is \"bitpacked\" to optimize for storage space. The vanilla savedata format consumes **")
   out_file:write(to_percentage(sector_size_info.totals.unpacked / available_bytes))
   out_file:write("** of the available space, leaving ")
   out_file:write(available_bytes - sector_size_info.totals.unpacked)
   out_file:write(" bytes to spare. The bitpacked format consumes **")
   out_file:write(to_percentage(sector_size_info.totals.packed / available_bits))
   out_file:write("** of that space, while wasting **")
   out_file:write(to_percentage(sector_size_info.totals.lost / available_bits))
   out_file:write("** of the available space due to technical limitations, thereby leaving ")
   out_file:write(available_bytes - math.ceil(total_bits_used / 8))
   out_file:write(" bytes to spare.\n\n")
end

out_file:write("## Stats\n\n")

do -- Stats per sector
   local available_bytes = sector_size * sector_count
   local available_bits  = available_bytes * 8
   local total_bits_used = sector_size_info.totals.packed + sector_size_info.totals.lost
   
   out_file:write("### Stats per sector\n\n")
   out_file:write("| Sector | Unpacked bytes | Unpacked % | Packed bytes | Packed bits | Packed % |\n")
   out_file:write("| -: | -: | -: | -: | -: | -: |\n")
   for i = 1, sector_count do
      out_file:write("|")
      out_file:write(i - 1)
      out_file:write("|")
      local sector = sector_size_info.by_sector[i]
      if sector then
         out_file:write(sector.sizes.unpacked)
         out_file:write("|")
         out_file:write(to_percentage(sector.sizes.unpacked / sector_size))
         out_file:write("|")
         out_file:write(math.ceil(sector.sizes.packed / 8))
         out_file:write("|")
         out_file:write(sector.sizes.packed)
         out_file:write("|")
         out_file:write(to_percentage(sector.sizes.packed / (sector_size * 8)))
         out_file:write("|")
      end
      out_file:write("\n")
   end
   out_file:write("|**Used**|")
   out_file:write(sector_size_info.totals.unpacked)
   out_file:write("|")
   out_file:write(to_percentage(sector_size_info.totals.unpacked / available_bytes))
   out_file:write("|")
   out_file:write(math.ceil(sector_size_info.totals.packed / 8))
   out_file:write("|")
   out_file:write(sector_size_info.totals.packed)
   out_file:write("|")
   out_file:write(to_percentage(sector_size_info.totals.packed / available_bits))
   out_file:write("|\n")
   out_file:write("|**Lost**|")
   -- no bytes lost in vanilla, since that's just a memcpy in slices
   out_file:write("|")
   -- no % lost in vanilla, since that's just a memcpy in slices
   out_file:write("|")
   out_file:write(math.ceil(sector_size_info.totals.lost / 8))
   out_file:write("|")
   out_file:write(sector_size_info.totals.lost)
   out_file:write("|")
   out_file:write(to_percentage(sector_size_info.totals.lost / available_bits))
   out_file:write("|\n")
   out_file:write("|**Free**|")
   out_file:write(available_bytes - sector_size_info.totals.unpacked)
   out_file:write("|")
   out_file:write(to_percentage(1 - (sector_size_info.totals.unpacked / available_bytes)))
   out_file:write("|")
   out_file:write(available_bytes - math.ceil(total_bits_used / 8))
   out_file:write("|")
   out_file:write(available_bits - math.ceil(total_bits_used / 8))
   out_file:write("|")
   out_file:write(to_percentage(1 - (total_bits_used / available_bits)))
   out_file:write("|\n\n")
   out_file:write("Some values can't be split across sectors. If these values can't fit at the end of a sector, then they must be pushed to the start of the next sector &mdash; leaving unused space at the end of the sector they were pushed past. This space is listed as \"lost\" in the table above.[^vanilla-never-loses-space]\n\n")
   out_file:write("[^vanilla-never-loses-space]: The vanilla game never produces \"lost\" space because it just uses `memcpy` to copy data directly between RAM and flash memory, blindly slicing values at sector boundaries. By contrast, the generating bitpacking code can only slice aggregates (e.g. arrays, structs, unions); it doesn't support slicing primitives (i.e. integers), strings, or opaque buffers.\n\n")
   out_file:write("Some top-level values are also deliberately forced to align with the start of a sector, rather than sharing a sector with any preceding value. These values are not counted as creating \"lost\" space at the end of the previous sector.\n\n")
end

function print_size_and_count_stats(base)
   local is_categories   = base.node_name == "categories"
   local name_col_header = is_categories and "Category" or "Typename"

   local items = {}
   base:for_each_child_element(function(node)
      local name
      if is_categories then
         if node.node_name ~= "category" then
            return
         end
         name = node.attributes["name"]
      else
         if node.node_name ~= "struct" and node.node_name ~= "union" then
            return
         end
         name = node.attributes["tag"] or node.attributes["name"]
         node = node:children_by_node_name("stats")[1]
         if not node then
            return
         end
      end
      if not name then
         return
      end
      local counts    = node:children_by_node_name("counts")[1]
      local bitcounts = node:children_by_node_name("bitcounts")[1]
      if not counts or not bitcounts then
         return
      end
      
      local item = {
         name  = name,
         sizes = {
            total_packed   = tonumber(bitcounts.attributes["total-packed"]),
            total_unpacked = tonumber(bitcounts.attributes["total-unpacked"]),
         },
         counts = {
            total = tonumber(counts.attributes["total"]),
            by_sector = {},
            by_top_level_value = {},
         }
      }
      items[#items + 1] = item
      if not is_categories then
         item.sizes.single_packed   = item.sizes.total_packed / item.counts.total
         item.sizes.single_unpacked = item.sizes.total_unpacked / item.counts.total
      end
      
      counts:for_each_child_element(function(child)
         local count = child.attributes["count"]
         if not count then
            return
         end
         count = tonumber(count)
         if child.node_name == "in-sector" then
            item.counts.by_sector[tonumber(child.attributes["index"]) + 1] = count
         elseif child.node_name == "in-top-level-value" then
            item.counts.by_top_level_value[child.attributes["name"]] = count
         end
      end)
   end)

   -- Size information
   do
      out_file:write("#### Size info\n\n")
      if is_categories then
         out_file:write("All sizes listed are totals. The **% used** column indicates how much of the total save file space is consumed by values of a given category. The **% size reduction** column is relative to the total unpacked size consumed by a given category.\n\n")
         out_file:write("| Categories | Count | Unpacked bytes | Packed bits | % used | % size reduction |\n")
         out_file:write("| :- | -: | -: | -: | -: | -: |\n")
         for i = 1, #items do
            local item = items[i]
            out_file:write("|**")
            out_file:write(item.name)
            out_file:write("**|")
            out_file:write(item.counts.total)
            out_file:write("|")
            out_file:write(item.sizes.total_unpacked / 8)
            out_file:write("|")
            out_file:write(item.sizes.total_packed)
            out_file:write("|")
            out_file:write(to_percentage(item.sizes.total_packed / (sector_size * 8 * sector_count)))
            out_file:write("|")
            out_file:write(to_percentage(1 - (item.sizes.total_packed / item.sizes.total_unpacked)))
            out_file:write("|\n")
         end
         out_file:write("\n")
      else
         out_file:write("The **% used** column indicates how much of the total save file space is consumed by values of a given type.\n\n")
         out_file:write("<table>\n")
         out_file:write("<thead>\n")
         out_file:write("<tr>\n")
         out_file:write("<th></th>")
         out_file:write("<th colspan='3'>Sizes per instance</th>")
         out_file:write("<th></th>")
         out_file:write("<th colspan='3'>Total sizes</th>")
         out_file:write("</tr>\n")
         out_file:write("<tr>\n")
         out_file:write("<th style='text-align:left'>Typename</th>")
         out_file:write("<th>Unpacked bytes</th>")
         out_file:write("<th>Packed bits</th>")
         out_file:write("<th>Savings</th>")
         out_file:write("<th>Count</th>")
         out_file:write("<th>Unpacked bytes</th>")
         out_file:write("<th>Packed bits</th>")
         out_file:write("<th>% used</th>")
         out_file:write("</tr>\n")
         out_file:write("</thead>\n")
         out_file:write("<tbody style='text-align:right'>\n")
         for i = 1, #items do
            local item = items[i]
            out_file:write("<tr>")
            out_file:write("<th style='text-align:left'>")
            out_file:write(item.name)
            out_file:write("</th>")
            if not is_categories then
               out_file:write("<td>")
               out_file:write(item.sizes.single_unpacked / 8)
               out_file:write("</td>")
               out_file:write("<td>")
               out_file:write(item.sizes.single_packed)
               out_file:write("</td>")
               out_file:write("<td>")
               out_file:write(to_percentage(1 - (item.sizes.single_packed / item.sizes.single_unpacked)))
               out_file:write("</td>")
            end
            out_file:write("<td>")
            out_file:write(item.counts.total)
            out_file:write("</td>")
            out_file:write("<td>")
            out_file:write(item.sizes.total_unpacked / 8)
            out_file:write("</td>")
            out_file:write("<td>")
            out_file:write(item.sizes.total_packed)
            out_file:write("</td>")
            out_file:write("<td>")
            out_file:write(to_percentage(item.sizes.total_packed / (sector_size * 8 * sector_count)))
            out_file:write("</td>")
            if is_categories then
               out_file:write("<td>")
               out_file:write(to_percentage(1 - (item.sizes.total_packed / item.sizes.total_unpacked)))
               out_file:write("</td>")
            end
            out_file:write("</tr>\n")
         end
         out_file:write("</tbody>\n")
         out_file:write("</table>\n\n")
      end
   end
   
   -- Counts per sector
   do
      out_file:write("#### Counts by sector\n\n")
      out_file:write("| ")
      out_file:write(name_col_header)
      out_file:write(" ")
      for i = 1, sector_count do
         out_file:write("| ")
         out_file:write(i - 1)
         out_file:write(" ")
      end
      out_file:write("| Total |\n")
      out_file:write("| :- ")
      for i = 1, sector_count do
         out_file:write("| -: ")
      end
      out_file:write("| -: |\n")
      for i = 1, #items do
         local item = items[i]
         out_file:write("|**")
         out_file:write(item.name)
         out_file:write("**")
         for i = 1, sector_count do
            local c = item.counts.by_sector[i]
            if c and c > 0 then
               out_file:write("|")
               out_file:write(c)
            else
               out_file:write("|")
            end
         end
         out_file:write("|")
         out_file:write(item.counts.total)
         out_file:write("|\n")
      end
      out_file:write("\n")
   end
   
   -- Counts by top-level value
   do
      out_file:write("#### Counts by top-level value\n\n")
      out_file:write("| ")
      out_file:write(name_col_header)
      out_file:write(" ")
      for i = 1, #top_level_values do
         out_file:write("| ")
         out_file:write(top_level_values[i].name)
         out_file:write(" ")
      end
      out_file:write("| Total |\n")
      out_file:write("| :- ")
      for i = 1, #top_level_values do
         out_file:write("| -: ")
      end
      out_file:write("| -: |\n")
      for i = 1, #items do
         local item = items[i]
         out_file:write("|**")
         out_file:write(item.name)
         out_file:write("**")
         for i = 1, #top_level_values do
            local c = item.counts.by_top_level_value[top_level_values[i].name]
            if c and c > 0 then
               out_file:write("|")
               out_file:write(c)
            else
               out_file:write("|")
            end
         end
         out_file:write("|")
         out_file:write(item.counts.total)
         out_file:write("|\n")
      end
      out_file:write("\n")
   end
end

do -- Stats per value category
   out_file:write("### Bitpack value categories\n\n")
   out_file:write("Types or values can be annotated with category names. Category names have no effect on how values are packed; they are purely an informational tool for external tools which read the bitpack format XML (e.g. the tool which produced this report).\n\n")
   
   local base = root_node:children_by_node_name("categories")[1]
   assert(base, "Missing node: data:root > categories")
   print_size_and_count_stats(base)
end

do -- Stats per type
   out_file:write("### Stats per struct/union type\n\n")
   out_file:write("**Note:** These listings make no effort to distinguish cases where one struct is transformed into another during serialization (both types may be listed), nor to indicate when one struct commonly or only appears as a member of another struct.\n\n")
   
   local base = root_node:children_by_node_name("c-types")[1]
   assert(base, "Missing node: data:root > c-types")
   print_size_and_count_stats(base)
end

out_file:close()

--
-- Replace the last finished Markdown file with the temporary file we've generated.
--

require "shell"

local copy_exit_code = shell:exec("/bin/cp "..this_dir.."/../../reports/savedata.md.tmp "..this_dir.."/../../reports/savedata.md")
if copy_exit_code == 0 then
   os.remove(this_dir.."../../reports/savedata.md.tmp")
   print("Wrote a report on the savedata format to /reports/savedata.md.")
else
   print("Warning: Failed to write the savedata report to /reports/savedata.md. The generated report should be in /reports/savedata.md.tmp.")
end